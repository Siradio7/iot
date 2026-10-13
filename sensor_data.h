#ifndef SENSOR_DATA_H
#define SENSOR_DATA_H

// Mesures regroupees a chaque cycle
struct SensorData {
    float temperature;  // degres C
    int luminosite;     // moyenne glissante, valeur ADC de 0 a 4095
};

#endif