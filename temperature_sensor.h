// Capteur de temperature DS18B20 (bus OneWire).

#ifndef TEMPERATURE_SENSOR_H
#define TEMPERATURE_SENSOR_H

void temperature_sensor_init();
// Temperature en degres C, ou TEMPERATURE_ERROR (-127) si le capteur est absent
float temperature_read();

#endif