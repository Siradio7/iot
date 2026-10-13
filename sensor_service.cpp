#include "sensor_service.h"

#include "config.h"
#include "temperature_sensor.h"
#include "light_sensor.h"

// Seuils courants : initialises avec config.h, modifiables depuis le dashboard
static float seuilBas = SEUIL_BAS;
static float seuilHaut = SEUIL_HAUT;

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

float sensor_service_low_threshold() {
    return seuilBas;
}

float sensor_service_high_threshold() {
    return seuilHaut;
}

// Les bornes evitent des valeurs absurdes, et low < high garde la logique de regulation coherente
bool sensor_service_set_thresholds(float low, float high) {
    if (low < SEUIL_MIN || high > SEUIL_MAX || low >= high) {
        return false;
    }

    seuilBas = low;
    seuilHaut = high;

    return true;
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
    if (temperature > seuilHaut + HYSTERESIS) {
        return CLIMATISATION;
    }

    if (temperature < seuilBas - HYSTERESIS) {
        return CHAUFFAGE;
    }

    bool zoneNeutre = (temperature < seuilHaut - HYSTERESIS) && (temperature > seuilBas + HYSTERESIS);

    if (zoneNeutre) {
        return REPOS;
    }

    return previousState;
}

// Appelee uniquement en CLIMATISATION. Vitesse proportionnelle a l'ecart au seuil haut :
//   T <= seuil haut            -> VITESSE_MIN (minimum pour que le ventilateur demarre)
//   ecart >= ECART_VITESSE_MAX -> VITESSE_MAX
// Entre les deux, la vitesse augmente lineairement. Le cas T <= seuil haut arrive
// pendant l'hysteresis (clim encore active entre 25.7 et 26 avec les seuils par defaut) : le
// ventilateur reste alors au minimum au lieu de s'arreter.
int sensor_service_fan_speed(float temperature) {
    float ecart = temperature - seuilHaut;

    if (ecart < 0) {
        ecart = 0;
    }

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