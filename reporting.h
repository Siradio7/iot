// Reporting periodique : l'ESP envoie son statut JSON par une requete POST
// vers un serveur de donnees (Node-RED), dont l'adresse est choisie depuis
// la page d'administration (formulaire POST /target).

#ifndef REPORTING_H
#define REPORTING_H

#include <Arduino.h>

// Valeurs de config.h (reporting inactif tant que sp vaut 0)
void reporting_init();

// Nouvelle cible. Retourne false sans rien changer si ip, port ou sp sont invalides
// (ip : IPv4, port : 1 a 65535, sp : secondes, 0 = arret). Peut etre appelee depuis un handler HTTP.
bool reporting_set_target(const String& ip, long port, long sp);

const char* reporting_ip();
int reporting_port();
int reporting_period();   // secondes, 0 = reporting arrete

// Vrai si le WiFi est connecte, que sp > 0 et que la periode est ecoulee
bool reporting_due();

// Envoie body en POST /esp?mac=<adresse MAC> avec Content-Type: application/json.
// Bloque au plus REPORT_TIMEOUT ms : a appeler depuis loop(), jamais depuis un handler HTTP.
void reporting_send(const String& body);

#endif
