#include "led.h"
#include "pins.h"
#include <Arduino.h>

void led_init() {
    pinMode(PIN_CLIMATISATION, OUTPUT);
    pinMode(PIN_RADIATEUR, OUTPUT);
    pinMode(PIN_ALERTE_FEU, OUTPUT);

    digitalWrite(PIN_CLIMATISATION, LOW);
    digitalWrite(PIN_RADIATEUR, LOW);
    digitalWrite(PIN_ALERTE_FEU, LOW);
}

void led_climatisation(bool active) {
    digitalWrite(PIN_CLIMATISATION, active ? HIGH : LOW);
}

void led_radiateur(bool active) {
    digitalWrite(PIN_RADIATEUR, active ? HIGH : LOW);
}

void led_alerte_feu(bool active) {
    digitalWrite(PIN_ALERTE_FEU, active ? HIGH : LOW);
}