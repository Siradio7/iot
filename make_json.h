#ifndef MAKE_JSON_H
#define MAKE_JSON_H

#include "sensor_data.h"
#include "sensor_service.h"

#include <ArduinoJson.h>

// Construit le statut de l'ESP au format JSON du cours
// (sections status, location, regul, info, net, reporthost)
JsonDocument makeJSON_fromstatus(SensorData data, EtatRegulation etat, int fanSpeed, bool fireDetected);

#endif
