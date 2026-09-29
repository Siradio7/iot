#include "sensor_service.h"

#include "config.h"
#include "temperature_sensor.h"
#include "light_sensor.h"

void sensor_service_init() {
    temperature_sensor_init();
    light_sensor_init();
}

SensorData sensor_service_read() {
    SensorData data;

    data.temperature = temperature_read();
    data.luminosite = light_sensor_read();

    return data;
}

EtatRegulation sensor_service_decide(float temperature, EtatRegulation previousState) {
    if (temperature > SEUIL_HAUT + HYSTERESIS) {
        return CLIMATISATION;
    }

    if (temperature < SEUIL_BAS - HYSTERESIS) {
        return CHAUFFAGE;
    }

    bool zoneNeutre = (temperature < SEUIL_HAUT - HYSTERESIS) && (temperature > SEUIL_BAS + HYSTERESIS);

    if (zoneNeutre) {
        return REPOS;
    }

    return previousState;
}

int sensor_service_fan_speed(float temperature) {
    if (temperature <= SEUIL_HAUT) {
        return 0;
    }

    float ecart = temperature - SEUIL_HAUT;

    if (ecart > ECART_VITESSE_MAX) {
        ecart = ECART_VITESSE_MAX;
    }

    float proportion = ecart / ECART_VITESSE_MAX;

    return VITESSE_MIN + (int)(proportion * (VITESSE_MAX - VITESSE_MIN));
}

bool sensor_service_fire_detected(int luminosite) {
    return luminosite < SEUIL_INCENDIE;
}

const char* sensor_service_state_name(EtatRegulation state) {
    switch (state) {
        case CLIMATISATION:
            return "CLIMATISATION";
        case CHAUFFAGE:
            return "CHAUFFAGE";
        default:
            return "REPOS";
    }
}