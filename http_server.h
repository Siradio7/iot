// Serveur HTTP asynchrone de l'ESP (bibliotheque ESPAsyncWebServer).
//
//   GET  /                         page d'administration (data/index.html, placeholders %...%)
//   GET  /esp.css                  feuille de style de la page
//   POST /target  ip, port, sp     choisit le serveur de reporting et la periode (0 = arret)
//   GET  /value?temperature&light  valeurs demandees en JSON (temperature, light, fire, ht, lt)
//   GET  /set?cool=on&lt=22         positionne la regulation (cool, heat : on/off/auto ; lt, ht : seuils)
//
// Les handlers tournent dans la tache reseau : ils ne lisent jamais un capteur
// (delay/yield interdits), ils renvoient la derniere mesure memorisee par loop().

#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include "sensor_data.h"
#include "sensor_service.h"

// Demarre le serveur (LittleFS doit etre monte). Sans WiFi, il n'est pas demarre.
void http_server_init();

// Memorise la derniere mesure et les etats des actionneurs, a appeler a chaque mesure
void http_server_update(SensorData data, EtatRegulation etat, int fanSpeed, bool fireDetected);

#endif
