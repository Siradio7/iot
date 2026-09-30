#include "config.h"
#include "sensor_data.h"
#include "sensor_service.h"
#include "led.h"
#include "fan.h"
#include "led_strip.h"
#include "makejson.h"

EtatRegulation etatCourant = REPOS;
unsigned long dernierePrise = 0;

void setup() {
    Serial.begin(SERIAL_BAUDRATE);

    sensor_service_init();
    led_init();
    fan_init();
    led_strip_init();

    Serial.println("*** Regulation de temperature : demarrage ***");
    Serial.printf("Seuil bas = %.1f C, seuil haut = %.1f C\n", SEUIL_BAS, SEUIL_HAUT);
}

void loop() {
    if (millis() - dernierePrise < PERIODE_MESURE) {
        return;
    }

    dernierePrise = millis();
    SensorData data = sensor_service_read();

    if (data.temperature == TEMPERATURE_ERROR) {
        Serial.println("{\"erreur\":\"capteur de temperature non detecte\"}");

        return;
    }

    etatCourant = sensor_service_decide(data.temperature, etatCourant);

    int vitesse = 0;

    if (etatCourant == CLIMATISATION) {
        vitesse = sensor_service_fan_speed(data.temperature);
    }

    bool incendie = sensor_service_fire_detected(data.luminosite);

    // Actionneurs
    led_climatisation(etatCourant == CLIMATISATION);
    led_radiateur(etatCourant == CHAUFFAGE);
    led_alerte_feu(incendie);
    fan_set_speed(vitesse);

    // Bande LED
    switch (etatCourant) {
        case CLIMATISATION:
            led_strip_set_color(STRIP_RED);
            break;
        case CHAUFFAGE:
            led_strip_set_color(STRIP_BLUE);
            break;
        case REPOS:
        default:
            led_strip_set_color(STRIP_GREEN);
            break;
    }

    // Journalisation (une ligne JSON par mesure)
    JsonDocument status = makeJSON_fromstatus(data, etatCourant, vitesse, incendie);
    serializeJson(status, Serial);
    Serial.println();
}