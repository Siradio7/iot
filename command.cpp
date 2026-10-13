#include "command.h"

#include "config.h"
#include "sensor_service.h"

#include <Arduino.h>
#include <ArduinoJson.h>

// Caracteres recus en attendant la fin de ligne
static char buffer[COMMAND_BUFFER_SIZE];
static size_t length = 0;
static bool overflow = false;

static void report_error(const char* message) {
    Serial.printf("{\"erreur\":\"commande refusee : %s\"}\n", message);
}

// Decode une ligne JSON et applique les seuils qu'elle contient
static void apply_command(const char* line) {
    JsonDocument doc;

    if (deserializeJson(doc, line)) {
        report_error("JSON invalide");
        return;
    }

    if (!doc.is<JsonObject>()) {
        report_error("un objet JSON est attendu");
        return;
    }

    // On part des seuils actuels : une commande peut n'en changer qu'un seul
    float low = sensor_service_low_threshold();
    float high = sensor_service_high_threshold();

    for (JsonPair pair : doc.as<JsonObject>()) {
        const char* key = pair.key().c_str();

        if (strcmp(key, "lt") != 0 && strcmp(key, "ht") != 0) {
            report_error("cle inconnue (lt ou ht attendu)");
            return;
        }

        if (!pair.value().is<float>()) {
            report_error("la valeur doit etre un nombre");
            return;
        }

        if (strcmp(key, "lt") == 0) {
            low = pair.value().as<float>();
        } else {
            high = pair.value().as<float>();
        }
    }

    if (doc.size() == 0) {
        report_error("aucun seuil (lt ou ht)");
        return;
    }

    if (!sensor_service_set_thresholds(low, high)) {
        report_error("seuils hors limites ou lt >= ht");
    }
}

void command_process() {
    while (Serial.available() > 0) {
        char c = Serial.read();

        if (c == '\r') {
            continue;
        }

        // Fin de ligne : la commande est complete
        if (c == '\n') {
            if (overflow) {
                report_error("ligne trop longue");
            } else if (length > 0) {
                buffer[length] = '\0';
                apply_command(buffer);
            }

            length = 0;
            overflow = false;
            continue;
        }

        if (length < COMMAND_BUFFER_SIZE - 1) {
            buffer[length++] = c;
        } else {
            overflow = true;
        }
    }
}
