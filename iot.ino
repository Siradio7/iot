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

EtatRegulation etatCourant = REPOS;  // conserve d'une mesure a l'autre pour l'hysteresis
unsigned long dernierePrise = 0;     // instant (ms) de la derniere mesure

void setup() {
    Serial.begin(SERIAL_BAUDRATE);

    sensor_service_init();
    led_init();
    fan_init();
    led_strip_init();
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

    // Decision de regulation (chauffage / repos / climatisation)
    etatCourant = sensor_service_decide(data.temperature, etatCourant);

    // Le ventilateur ne tourne qu'en climatisation, avec une vitesse progressive
    int vitesse = 0;

    if (etatCourant == CLIMATISATION) {
        vitesse = sensor_service_fan_speed(data.temperature);
    }

    bool incendie = sensor_service_fire_detected(data);

    // Incendie : arret du ventilateur pour ne pas attiser le feu
    if (incendie) {
        vitesse = 0;
    }

    // Actionneurs
    led_climatisation(etatCourant == CLIMATISATION);
    led_radiateur(etatCourant == CHAUFFAGE);
    led_alerte_feu(incendie);
    fan_set_speed(vitesse);

    // Bande LED : couleur selon la temperature, avec l'hysteresis de la regulation
    // (rouge > seuil haut, orange entre les seuils, vert < seuil bas)
    switch (etatCourant) {
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

    // Journalisation (une ligne JSON par mesure)
    JsonDocument status = makeJSON_fromstatus(data, etatCourant, vitesse, incendie);
    serializeJson(status, Serial);
    Serial.println();
}