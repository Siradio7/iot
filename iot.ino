// Point d'entree du firmware : initialisation des modules puis boucle de regulation.
// Chaque PERIODE_MESURE : lecture des capteurs, decision, commande des actionneurs
// et envoi d'une ligne JSON sur le port serie (lue par esp32-bridge puis Node-RED).

#include "config.h"
#include "sensor_data.h"
#include "sensor_service.h"
#include "led.h"
#include "fan.h"
#include "led_strip.h"
#include "make_json.h"
#include "command.h"
#include "littlefs_utils.h"
#include "wifi_utils.h"
#include "http_server.h"
#include "reporting.h"
#include "manual_control.h"

EtatRegulation etatCourant = REPOS;  // conserve d'une mesure a l'autre pour l'hysteresis
unsigned long dernierePrise = 0;     // instant (ms) de la derniere mesure

void setup() {
    Serial.begin(SERIAL_BAUDRATE);

    // Fichiers en memoire flash (dossier data/ envoye avec "Upload LittleFS") :
    // la liste est affichee une seule fois, au demarrage
    if (littlefs_init()) {
        littlefs_list_dir("/", 1);
    }

    // WiFi : connexion au meilleur point d'acces connu (wificonnect_multi du cours).
    // Sans WiFi, la regulation fonctionne quand meme (voir WIFI_RESTART_IF_FAILED).
    wificonnect_multi(WIFI_HOSTNAME);
    if (WiFi.status() == WL_CONNECTED) {
        Serial.print("\nWiFi connected : yes ! \n");
    } else {
        Serial.print("\nWiFi connected : no ! \n");
    }

    sensor_service_init();
    led_init();
    fan_init();
    led_strip_init();
    reporting_init();

    // Serveur HTTP : page d'administration et API (necessite LittleFS et le WiFi)
    http_server_init();
}

void loop() {
    // Commandes du dashboard : lues a chaque tour, avant l'attente de la periode,
    // pour ne perdre aucun caractere recu
    command_process();

    // Cadence de mesure non bloquante (pas de delay)
    if (millis() - dernierePrise < PERIODE_MESURE) {
        return;
    }

    dernierePrise = millis();
    SensorData data = sensor_service_read();

    // Capteur absent ou debranche : on signale l'erreur sans piloter les actionneurs
    if (data.temperature == TEMPERATURE_ERROR) {
        Serial.println("{\"erreur\":\"capteur de temperature non detecte\"}");

        return;
    }

    // Decision de regulation (chauffage / repos / climatisation).
    // etatCourant garde la decision automatique pour l'hysteresis ; etat tient compte
    // d'un eventuel forcage demande par HTTP (/set?cool=on).
    etatCourant = sensor_service_decide(data.temperature, etatCourant);
    EtatRegulation etat = manual_apply(etatCourant);

    // Le ventilateur ne tourne qu'en climatisation, avec une vitesse progressive
    int vitesse = 0;

    if (etat == CLIMATISATION) {
        vitesse = sensor_service_fan_speed(data.temperature);
    }

    bool incendie = sensor_service_fire_detected(data);

    // Incendie : arret du ventilateur pour ne pas attiser le feu
    if (incendie) {
        vitesse = 0;
    }

    // Actionneurs
    led_climatisation(etat == CLIMATISATION);
    led_radiateur(etat == CHAUFFAGE);
    led_alerte_feu(incendie);
    fan_set_speed(vitesse);

    // Bande LED : couleur selon la temperature, avec l'hysteresis de la regulation
    // (rouge > seuil haut, orange entre les seuils, vert < seuil bas)
    switch (etat) {
        case CLIMATISATION:
            led_strip_set_color(STRIP_RED);
            break;
        case CHAUFFAGE:
            led_strip_set_color(STRIP_GREEN);
            break;
        case REPOS:
        default:
            led_strip_set_color(STRIP_ORANGE);
            break;
    }

    // Derniere mesure pour les handlers HTTP (qui ne lisent jamais les capteurs)
    http_server_update(data, etat, vitesse, incendie);

    // Journalisation (une ligne JSON par mesure)
    JsonDocument status = makeJSON_fromstatus(data, etat, vitesse, incendie);
    serializeJson(status, Serial);
    Serial.println();

    // Reporting periodique vers le serveur choisi dans la page d'administration
    if (reporting_due()) {
        String body;
        serializeJson(status, body);
        reporting_send(body);
    }
}