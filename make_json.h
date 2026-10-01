#ifndef MAKE_JSON_H
#define MAKE_JSON_H

#include "sensor_data.h"
#include "sensor_service.h"

#include <ArduinoJson.h>

// Builds the ESP status following the JSON model given in the course
JsonDocument makeJSON_fromstatus(SensorData data, EtatRegulation etat, int fanSpeed, bool fireDetected);

#endif
