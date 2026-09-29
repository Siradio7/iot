#include "fan.h"
#include "../config/pins.h"
#include "../config/config.h"
#include <Arduino.h>

void fan_init() {
    ledcAttach(PIN_VENTILATEUR, PWM_FREQUENCE, PWM_RESOLUTION);
    ledcWrite(PIN_VENTILATEUR, 0);
}

void fan_set_speed(int speed) {
    if (speed < 0) {
        speed = 0;
    }

    if (speed > VITESSE_MAX) {
        speed = VITESSE_MAX;
    }

    ledcWrite(PIN_VENTILATEUR, speed);
}