#include "reporting.h"

#include "config.h"

#include <WiFi.h>
#include <HTTPClient.h>

// Ecrits par le handler HTTP (tache reseau), lus par loop() : de simples copies
// de petits types, au pire une requete part avec l'ancienne valeur.
static char targetIp[16] = REPORT_TARGET_IP;
static volatile int targetPort = REPORT_TARGET_PORT;
static volatile int periodSeconds = REPORT_SP;
static volatile bool sendNow = false;   // premiere emission immediate apres un changement de cible
static unsigned long lastSend = 0;

void reporting_init() {
    lastSend = millis();
}

bool reporting_set_target(const String& ip, long port, long sp) {
    IPAddress address;

    // fromString accepte aussi "1.2" ou "1": on exige quatre champs
    if (!address.fromString(ip) || ip.length() > 15 || ip.indexOf('.') < 0) {
        return false;
    }

    if (port < 1 || port > 65535 || sp < 0) {
        return false;
    }

    strlcpy(targetIp, ip.c_str(), sizeof(targetIp));
    targetPort = (int)port;
    periodSeconds = (int)sp;
    sendNow = (sp > 0);

    return true;
}

const char* reporting_ip() {
    return targetIp;
}

int reporting_port() {
    return targetPort;
}

int reporting_period() {
    return periodSeconds;
}

bool reporting_due() {
    if (periodSeconds <= 0 || WiFi.status() != WL_CONNECTED) {
        return false;
    }

    return sendNow || (millis() - lastSend >= (unsigned long)periodSeconds * 1000UL);
}

void reporting_send(const String& body) {
    sendNow = false;
    lastSend = millis();

    // Le serveur de reporting peut etre la cible de plusieurs objets : l'adresse MAC les identifie
    String url = String("http://") + targetIp + ":" + targetPort + "/esp?mac=" + WiFi.macAddress();

    WiFiClient client;
    HTTPClient http;

    http.setConnectTimeout(REPORT_TIMEOUT);
    http.setTimeout(REPORT_TIMEOUT);

    if (!http.begin(client, url)) {
        Serial.println("{\"erreur\":\"reporting : URL invalide\"}");
        return;
    }

    http.addHeader("Content-Type", "application/json");
    int code = http.POST(body);

    if (code > 0) {
        Serial.printf("Reporting POST %s -> %d\n", url.c_str(), code);
    } else {
        Serial.printf("Reporting POST %s impossible : %s\n", url.c_str(), HTTPClient::errorToString(code).c_str());
    }

    http.end();
}
