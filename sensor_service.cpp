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
    data.luminosite = light_sensor_read_average();

    return data;
}

// Regulation avec hysteresis (valeurs par defaut : seuils 25 / 26, hysteresis 0.3) :
//
//   T < 24.7          -> CHAUFFAGE
//   24.7 <= T <= 25.3 -> etat precedent conserve
//   25.3 <  T <  25.7 -> REPOS
//   25.7 <= T <= 26.3 -> etat precedent conserve
//   T > 26.3          -> CLIMATISATION
//
// Conserver l'etat precedent pres des seuils evite que le chauffage ou la clim
// s'allument et s'eteignent a chaque petite variation de la mesure.
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

// Vitesse proportionnelle a l'ecart au seuil haut :
//   T <= SEUIL_HAUT            -> 0 (arret)
//   ecart juste au-dessus de 0 -> VITESSE_MIN (minimum pour que le ventilateur demarre)
//   ecart >= ECART_VITESSE_MAX -> VITESSE_MAX
// Entre les deux, la vitesse augmente lineairement.
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

// La luminosite recue est deja moyennee (voir light_sensor_read_average),
// ce qui evite qu'une seule lecture parasite declenche l'alerte
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