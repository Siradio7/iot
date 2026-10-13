#include "fan.h"
#include "pins.h"
#include "config.h"
#include <Arduino.h>

// PWM materiel de l'ESP32 (API ledc du core 3.x), ventilateur arrete au demarrage
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