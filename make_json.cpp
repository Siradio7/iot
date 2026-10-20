#include "make_json.h"

#include "config.h"

#include <Arduino.h>
#include <WiFi.h>

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
    regul["lt"] = sensor_service_low_threshold();
    regul["ht"] = sensor_service_high_threshold();

    JsonObject info = doc["info"].to<JsonObject>();
    info["ident"] = INFO_IDENT;
    info["user"] = INFO_USER;
    info["loc"] = INFO_LOC;

    JsonObject net = doc["net"].to<JsonObject>();
    // Section net : valeurs reelles si le WiFi est connecte, "NOP" sinon
    bool wifiConnecte = (WiFi.status() == WL_CONNECTED);
    net["uptime"] = String(millis() / 1000);          // secondes depuis le demarrage
    net["ssid"] = wifiConnecte ? WiFi.SSID() : String(NET_NOP);
    net["mac"] = wifiConnecte ? WiFi.macAddress() : String(NET_NOP);
    net["ip"] = wifiConnecte ? WiFi.localIP().toString() : String(NET_NOP);

    JsonObject reporthost = doc["reporthost"].to<JsonObject>();
    reporthost["target_ip"] = REPORT_TARGET_IP;
    reporthost["target_port"] = REPORT_TARGET_PORT;
    reporthost["sp"] = REPORT_SP;

    return doc;
}
