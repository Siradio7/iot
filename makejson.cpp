#include "makejson.h"

#include "config.h"

#include <Arduino.h>

JsonDocument makeJSON_fromstatus(SensorData data, EtatRegulation etat, int fanSpeed, bool fireDetected) {
    JsonDocument doc;

    JsonObject status = doc["status"].to<JsonObject>();
    status["temperature"] = data.temperature;
    status["light"] = data.luminosite;
    status["regul"] = (etat == REPOS) ? "HALT" : "RUNNING";
    status["fire"] = fireDetected;
    status["heat"] = (etat == CHAUFFAGE) ? "ON" : "OFF";
    status["cold"] = (etat == CLIMATISATION) ? "ON" : "OFF";
    status["fanspeed"] = fanSpeed;

    JsonObject location = doc["location"].to<JsonObject>();
    location["room"] = LOCATION_ROOM;
    JsonObject gps = location["gps"].to<JsonObject>();
    gps["lat"] = LOCATION_LAT;
    gps["lon"] = LOCATION_LON;
    location["address"] = LOCATION_ADDRESS;

    JsonObject regul = doc["regul"].to<JsonObject>();
    regul["lt"] = SEUIL_BAS;
    regul["ht"] = SEUIL_HAUT;

    JsonObject info = doc["info"].to<JsonObject>();
    info["ident"] = INFO_IDENT;
    info["user"] = INFO_USER;
    info["loc"] = INFO_LOC;

    JsonObject net = doc["net"].to<JsonObject>();
    net["uptime"] = NET_NOP;
    net["ssid"] = NET_NOP;
    net["mac"] = NET_NOP;
    net["ip"] = NET_NOP;

    JsonObject reporthost = doc["reporthost"].to<JsonObject>();
    reporthost["target_ip"] = REPORT_TARGET_IP;
    reporthost["target_port"] = REPORT_TARGET_PORT;
    reporthost["sp"] = REPORT_SP;

    return doc;
}
