#include "manual_control.h"

// Ecrits par le handler HTTP, lus par loop() : un octet, pas de verrou necessaire
static volatile ManualMode coolMode = MANUAL_AUTO;
static volatile ManualMode heatMode = MANUAL_AUTO;

void manual_set_cool(ManualMode mode) {
    coolMode = mode;
}

void manual_set_heat(ManualMode mode) {
    heatMode = mode;
}

ManualMode manual_cool() {
    return coolMode;
}

ManualMode manual_heat() {
    return heatMode;
}

EtatRegulation manual_apply(EtatRegulation automatic) {
    ManualMode cool = coolMode;
    ManualMode heat = heatMode;

    if (cool == MANUAL_ON) {
        return CLIMATISATION;
    }

    if (heat == MANUAL_ON) {
        return CHAUFFAGE;
    }

    // Un forcage "off" eteint l'actionneur meme si la regulation voulait l'allumer
    if (automatic == CLIMATISATION && cool == MANUAL_OFF) {
        return REPOS;
    }

    if (automatic == CHAUFFAGE && heat == MANUAL_OFF) {
        return REPOS;
    }

    return automatic;
}

bool manual_parse(const String& text, ManualMode* mode) {
    if (text.equalsIgnoreCase("on")) {
        *mode = MANUAL_ON;
    } else if (text.equalsIgnoreCase("off")) {
        *mode = MANUAL_OFF;
    } else if (text.equalsIgnoreCase("auto")) {
        *mode = MANUAL_AUTO;
    } else {
        return false;
    }

    return true;
}

const char* manual_name(ManualMode mode) {
    switch (mode) {
        case MANUAL_ON:
            return "on";
        case MANUAL_OFF:
            return "off";
        default:
            return "auto";
    }
}
