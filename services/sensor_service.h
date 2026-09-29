#ifndef SENSOR_SERVICE_H
#define SENSOR_SERVICE_H

#include "../models/sensor_data.h"

enum EtatRegulation {
    CHAUFFAGE,
    REPOS,
    CLIMATISATION
};

void sensor_service_init();

SensorData sensor_service_read();
EtatRegulation sensor_service_decide(float temperature, EtatRegulation previousState);

int sensor_service_fan_speed(float temperature);
bool sensor_service_fire_detected(int luminosite);
const char* sensor_service_state_name(EtatRegulation state);

#endif