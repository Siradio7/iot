#include "littlefs_utils.h"

#include "config.h"

#include <Arduino.h>
#include "FS.h"
#include <LittleFS.h>

bool littlefs_init() {
    // Formate la partition seulement si elle est illisible (premier demarrage)
    if (!LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED)) {
        Serial.println("LittleFS Mount Failed");
        return false;
    }

    Serial.println("Little FS Mounted Successfully");

    return true;
}

void littlefs_list_dir(const char *dirname, uint8_t levels) {
    Serial.printf("Listing directory: %s\r\n", dirname);

    File root = LittleFS.open(dirname);
    if (!root) {
        Serial.println("- failed to open directory");
        return;
    }
    if (!root.isDirectory()) {
        Serial.println(" - not a directory");
        return;
    }

    // Parcours du dossier element par element
    File file = root.openNextFile();
    while (file) {
        if (file.isDirectory()) {
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if (levels) {
                littlefs_list_dir(file.path(), levels - 1);
            }
        } else {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("\tSIZE: ");
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }
}
