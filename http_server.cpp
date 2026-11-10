#include "http_server.h"

#include "config.h"
#include "manual_control.h"
#include "reporting.h"

#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>

static AsyncWebServer server(HTTP_PORT);

// Derniere mesure : ecrite par loop(), lue par les handlers
static volatile bool measured = false;
static volatile float lastTemperature = 0;
static volatile int lastLight = 0;
static volatile EtatRegulation lastState = REPOS;
static volatile int lastFanSpeed = 0;
static volatile bool lastFire = false;

void http_server_update(SensorData data, EtatRegulation etat, int fanSpeed, bool fireDetected) {
    lastTemperature = data.temperature;
    lastLight = data.luminosite;
    lastState = etat;
    lastFanSpeed = fanSpeed;
    lastFire = fireDetected;
    measured = true;
}

// Texte affiche pour une valeur pas encore mesuree
static String measuredText(const String& value) {
    return measured ? value : String("-");
}

// Remplace les placeholders %NOM% de data/index.html (pas de '%' litteral dans la page).
// Une valeur inconnue renvoie une chaine vide, comme le demande la bibliotheque.
static String processor(const String& var) {
    bool wifi = (WiFi.status() == WL_CONNECTED);

    if (var == "UPTIME") return String(millis() / 1000);
    if (var == "WHERE") return String(LOCATION_ROOM) + ", " + LOCATION_ADDRESS + " (" + INFO_LOC + ")";
    if (var == "IDENT") return String(INFO_IDENT) + " / " + INFO_USER;
    if (var == "HOSTNAME") return WIFI_HOSTNAME;
    if (var == "GPS") return String(LOCATION_LAT, 6) + ", " + String(LOCATION_LON, 6);

    if (var == "SSID") return wifi ? WiFi.SSID() : String(NET_NOP);
    if (var == "MAC") return wifi ? WiFi.macAddress() : String(NET_NOP);
    if (var == "IP") return wifi ? WiFi.localIP().toString() : String(NET_NOP);
    if (var == "RSSI") return wifi ? String(WiFi.RSSI()) : String(NET_NOP);

    if (var == "TEMPERATURE") return measuredText(String(lastTemperature, 2));
    if (var == "LIGHT") return measuredText(String(lastLight));
    if (var == "FIRE") return measuredText(lastFire ? "OUI" : "non");
    if (var == "COOLER") return measuredText(lastState == CLIMATISATION ? "ON" : "OFF");
    if (var == "HEATER") return measuredText(lastState == CHAUFFAGE ? "ON" : "OFF");
    if (var == "FAN") return measuredText(String(lastFanSpeed));
    if (var == "COOL_MODE") return manual_name(manual_cool());
    if (var == "HEAT_MODE") return manual_name(manual_heat());

    if (var == "LT") return String(sensor_service_low_threshold(), 1);
    if (var == "HT") return String(sensor_service_high_threshold(), 1);

    if (var == "PRT_IP") return reporting_ip();
    if (var == "PRT_PORT") return String(reporting_port());
    if (var == "PRT_T") return String(reporting_period());

    return String();
}

static void send_page(AsyncWebServerRequest* request) {
    request->send(LittleFS, "/index.html", String(), false, processor);
}

// Un parametre de /value est-il connu ? Remplit le JSON s'il l'est.
static bool add_value(JsonDocument& doc, const String& name) {
    if (name == "temperature") {
        doc[name] = lastTemperature;
    } else if (name == "light") {
        doc[name] = lastLight;
    } else if (name == "fire") {
        doc[name] = lastFire;
    } else if (name == "ht") {
        doc[name] = sensor_service_high_threshold();
    } else if (name == "lt") {
        doc[name] = sensor_service_low_threshold();
    } else {
        return false;
    }

    return true;
}

// GET /value?temperature&light -> {"temperature":23.4,"light":843}
static void handle_value(AsyncWebServerRequest* request) {
    if (request->params() == 0) {
        request->send(400, "text/plain", "Parametres attendus : temperature, light, fire, ht, lt");
        return;
    }

    // Verifier tous les noms avant de repondre : un nom inconnu donne 404
    JsonDocument doc;

    for (size_t i = 0; i < request->params(); i++) {
        String name = request->getParam(i)->name();

        if (!add_value(doc, name)) {
            request->send(404, "text/plain", "Valeur inconnue : " + name);
            return;
        }
    }

    // Les mesures n'existent qu'apres la premiere periode de mesure
    if (!measured && (doc["temperature"].is<float>() || doc["light"].is<int>() || doc["fire"].is<bool>())) {
        request->send(503, "text/plain", "Pas encore de mesure");
        return;
    }

    String body;
    serializeJson(doc, body);
    request->send(200, "application/json", body);
}

// GET /set?cool=on&heat=auto&lt=22&ht=25
static void handle_set(AsyncWebServerRequest* request) {
    if (request->params() == 0) {
        request->send(400, "text/plain", "Parametres attendus : cool, heat (on/off/auto), lt, ht");
        return;
    }

    // Tout verifier avant d'appliquer quoi que ce soit : une requete invalide ne change rien
    for (size_t i = 0; i < request->params(); i++) {
        String name = request->getParam(i)->name();

        if (name != "cool" && name != "heat" && name != "lt" && name != "ht") {
            request->send(404, "text/plain", "Parametre inconnu : " + name);
            return;
        }
    }

    ManualMode cool = manual_cool();
    ManualMode heat = manual_heat();
    float low = sensor_service_low_threshold();
    float high = sensor_service_high_threshold();
    bool changeThresholds = false;

    for (size_t i = 0; i < request->params(); i++) {
        String name = request->getParam(i)->name();
        String value = request->getParam(i)->value();

        if (name == "cool" || name == "heat") {
            ManualMode mode;

            if (!manual_parse(value, &mode)) {
                request->send(400, "text/plain", name + " : on, off ou auto attendu");
                return;
            }

            if (name == "cool") {
                cool = mode;
            } else {
                heat = mode;
            }
        } else {
            char* end = nullptr;
            float number = strtof(value.c_str(), &end);

            if (value.length() == 0 || *end != '\0') {
                request->send(400, "text/plain", name + " : nombre attendu");
                return;
            }

            if (name == "lt") {
                low = number;
            } else {
                high = number;
            }

            changeThresholds = true;
        }
    }

    if (changeThresholds && !sensor_service_set_thresholds(low, high)) {
        request->send(400, "text/plain", "Seuils hors limites ou lt >= ht");
        return;
    }

    manual_set_cool(cool);
    manual_set_heat(heat);

    JsonDocument doc;
    doc["cool"] = manual_name(cool);
    doc["heat"] = manual_name(heat);
    doc["lt"] = sensor_service_low_threshold();
    doc["ht"] = sensor_service_high_threshold();

    String body;
    serializeJson(doc, body);
    request->send(200, "application/json", body);
}

// POST /target (formulaire de la page) : ip, port, sp
static void handle_target(AsyncWebServerRequest* request) {
    if (!(request->hasArg("ip") && request->hasArg("port") && request->hasArg("sp"))) {
        request->send(400, "text/plain", "Parametres attendus : ip, port, sp");
        return;
    }

    String ip = request->arg("ip");
    String port = request->arg("port");
    String sp = request->arg("sp");

    ip.trim();
    port.trim();
    sp.trim();

    // atol renvoie 0 pour un texte non numerique : on verifie les chiffres nous-memes
    bool numeric = port.length() > 0 && sp.length() > 0;

    for (size_t i = 0; i < port.length(); i++) numeric = numeric && isDigit(port[i]);
    for (size_t i = 0; i < sp.length(); i++) numeric = numeric && isDigit(sp[i]);

    if (!numeric || !reporting_set_target(ip, port.toInt(), sp.toInt())) {
        request->send(400, "text/plain", "ip (IPv4), port (1 a 65535) et sp (secondes, 0 = arret) invalides");
        return;
    }

    Serial.printf("Reporting : %s:%d toutes les %d s\n", reporting_ip(), reporting_port(), reporting_period());
    send_page(request);
}

void http_server_init() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Serveur HTTP non demarre : pas de WiFi");
        return;
    }

    server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
        send_page(request);
    });

    server.on("/esp.css", HTTP_GET, [](AsyncWebServerRequest* request) {
        request->send(LittleFS, "/esp.css", "text/css");
    });

    server.on("/value", HTTP_GET, handle_value);
    server.on("/set", HTTP_GET, handle_set);
    server.on("/target", HTTP_POST, handle_target);

    // Toute autre route : 404
    server.onNotFound([](AsyncWebServerRequest* request) {
        request->send(404, "text/plain", "Not found");
    });

    server.begin();
    Serial.printf("Serveur HTTP : http://%s/\n", WiFi.localIP().toString().c_str());
}
