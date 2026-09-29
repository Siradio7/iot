#include "temperature_sensor.h"

#include "../config/pins.h"

#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>

static OneWire oneWire(PIN_TEMPERATURE);
static DallasTemperature sensor(&oneWire);

void temperature_sensor_init() {
    sensor.begin();
}

float temperature_read() {
    sensor.requestTemperaturesByIndex(0);

    return sensor.getTempCByIndex(0);
}