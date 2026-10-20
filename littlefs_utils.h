// Systeme de fichiers LittleFS dans la memoire flash de l'ESP32.
// Les fichiers du dossier data/ du croquis y sont envoyes avec le plugin
// "Upload LittleFS to Pico/ESP8266/ESP32" d'Arduino IDE.

#ifndef LITTLEFS_UTILS_H
#define LITTLEFS_UTILS_H

#include <stdint.h>

// Monte LittleFS. Retourne false si le systeme de fichiers est inutilisable.
bool littlefs_init();

// Affiche sur le port serie les fichiers d'un dossier (et ses sous-dossiers
// jusqu'a "levels" niveaux). Fonction listDir de l'exemple LITTLEFS_test d'Espressif.
void littlefs_list_dir(const char *dirname, uint8_t levels);

#endif
