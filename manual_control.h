// Forcage manuel du chauffage et de la climatisation (route HTTP /set?cool=on).
// Par defaut tout est automatique : la regulation decide.

#ifndef MANUAL_CONTROL_H
#define MANUAL_CONTROL_H

#include "sensor_service.h"

#include <Arduino.h>

enum ManualMode {
    MANUAL_AUTO,   // la regulation decide
    MANUAL_ON,     // force allume
    MANUAL_OFF     // force eteint
};

void manual_set_cool(ManualMode mode);
void manual_set_heat(ManualMode mode);
ManualMode manual_cool();
ManualMode manual_heat();

// Etat effectif a partir de l'etat decide par la regulation.
// Climatisation et chauffage ne peuvent pas etre actifs ensemble : la climatisation l'emporte.
EtatRegulation manual_apply(EtatRegulation automatic);

// "on", "off" ou "auto" -> mode. Retourne false si le texte est inconnu.
bool manual_parse(const String& text, ManualMode* mode);

const char* manual_name(ManualMode mode);

#endif
