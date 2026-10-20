// Modele : copier ce fichier en wifi_secrets.h (meme dossier) et y mettre son
// point d'acces personnel. wifi_secrets.h est ignore par Git : le mot de passe
// ne part pas sur GitHub. L'ESP32 ne voit que le WiFi 2.4 GHz, et le SSID ne doit
// pas contenir d'espace.

#ifndef WIFI_SECRETS_H
#define WIFI_SECRETS_H

#define WIFI_MY_SSID     "NomDuReseau"
#define WIFI_MY_PASSWORD "MotDePasse"

#endif
