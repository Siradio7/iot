#ifndef LIGHT_SENSOR_H
#define LIGHT_SENSOR_H

void light_sensor_init();

// Lecture brute de l'ADC (0 a 4095)
int light_sensor_read();

// Nouvelle lecture puis moyenne glissante des NB_ECHANTILLONS_LUMIERE dernieres valeurs
int light_sensor_read_average();

#endif
