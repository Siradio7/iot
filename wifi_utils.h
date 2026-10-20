// Connexion WiFi en mode station (code wificonnect_multi du cours tpwifi.pdf).
// L'ESP essaie les points d'acces connus et garde celui qui a le meilleur signal.

#ifndef WIFI_UTILS_H
#define WIFI_UTILS_H

#include <Arduino.h>
#include <WiFi.h> // https://www.arduino.cc/en/Reference/WiFi

#define WiFiMaxTry 10           // nombre de tentatives de connexion
#define SaveDisconnectTime 1000 // ms entre deux tentatives
                                // cf https://github.com/espressif/arduino-esp32/issues/2501

// Connexion a l'un des points d'acces de la liste (wifi_utils.cpp)
void wificonnect_multi(String hostname);

// Affiche l'etat de la connexion (IP, MAC, SSID, signal...) : C = 1 en C pur, 0 avec String
void wifi_printstatus(int C);

#endif
