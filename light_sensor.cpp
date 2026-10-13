#include "light_sensor.h"
#include "pins.h"
#include "config.h"
#include <Arduino.h>

// Buffer circulaire des dernieres lectures
static int echantillons[NB_ECHANTILLONS_LUMIERE];
static int indexCourant = 0;
static int nbEchantillons = 0;

void light_sensor_init() {
    pinMode(PIN_LUMINOSITE, INPUT);
}

int light_sensor_read() {
    return analogRead(PIN_LUMINOSITE);
}

int light_sensor_read_average() {
    echantillons[indexCourant] = light_sensor_read();
    indexCourant = (indexCourant + 1) % NB_ECHANTILLONS_LUMIERE;

    // Au demarrage, on ne moyenne que les valeurs deja lues pour eviter
    // qu'un buffer encore a 0 declenche une fausse alerte incendie
    if (nbEchantillons < NB_ECHANTILLONS_LUMIERE) {
        nbEchantillons++;
    }

    long somme = 0;

    for (int i = 0; i < nbEchantillons; i++) {
        somme += echantillons[i];
    }

    return somme / nbEchantillons;
}
