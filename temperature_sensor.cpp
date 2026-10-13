#include "temperature_sensor.h"

#include "pins.h"

#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>

static OneWire oneWire(PIN_TEMPERATURE);
static DallasTemperature sensor(&oneWire);

void temperature_sensor_init() {
    sensor.begin();
}

// Lecture du premier capteur du bus (index 0). L'appel est bloquant
// le temps de la conversion (environ 750 ms en resolution 12 bits).
float temperature_read() {
    sensor.requestTemperaturesByIndex(0);

    return sensor.getTempCByIndex(0);
}