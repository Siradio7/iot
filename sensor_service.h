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

// Vitesse PWM du ventilateur (0 a 255), progressive au-dessus du seuil haut
int sensor_service_fan_speed(float temperature);

// Vrai si la luminosite passe sous SEUIL_INCENDIE
bool sensor_service_fire_detected(int luminosite);

// Nom lisible d'un etat (pour l'affichage)
const char* sensor_service_state_name(EtatRegulation state);

#endif