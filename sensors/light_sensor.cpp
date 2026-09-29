#include "light_sensor.h"
#include "../config/pins.h"
#include <Arduino.h>

void light_sensor_init() {
    pinMode(PIN_LUMINOSITE, INPUT);
}

int light_sensor_read() {
    return analogRead(PIN_LUMINOSITE);
}