// Logique metier : lecture des capteurs, decision de regulation,
// vitesse du ventilateur et detection incendie.

#ifndef SENSOR_SERVICE_H
#define SENSOR_SERVICE_H

#include "sensor_data.h"

enum EtatRegulation {
    CHAUFFAGE,      // temperature trop basse : radiateur actif
    REPOS,          // temperature dans la plage voulue
    CLIMATISATION   // temperature trop haute : clim et ventilateur actifs
};

void sensor_service_init();

// Lit la temperature et la luminosite (moyennee)
SensorData sensor_service_read();

// Nouvel etat de regulation, avec hysteresis par rapport a l'etat precedent
EtatRegulation sensor_service_decide(float temperature, EtatRegulation previousState);

// Seuils courants (modifiables depuis le dashboard)
float sensor_service_low_threshold();
float sensor_service_high_threshold();

// Applique de nouveaux seuils s'ils sont entre SEUIL_MIN et SEUIL_MAX et que low < high.
// Retourne false sans rien changer sinon.
bool sensor_service_set_thresholds(float low, float high);

// Vitesse PWM du ventilateur en climatisation (VITESSE_MIN a VITESSE_MAX), progressive au-dessus du seuil haut
int sensor_service_fan_speed(float temperature);

// Vrai si la lumiere est nettement plus forte que la lumiere ambiante habituelle
// (VARIATION_LUMIERE_FEU) ET la temperature moyenne au-dessus de SEUIL_FEU_TEMPERATURE.
// A appeler une fois par mesure valide.
bool sensor_service_fire_detected(SensorData data);

// Nom lisible d'un etat (pour l'affichage)
const char* sensor_service_state_name(EtatRegulation state);

#endif