#include "sensor_service.h"

#include "config.h"
#include "temperature_sensor.h"
#include "light_sensor.h"

// Seuils courants : initialises avec config.h, modifiables depuis le dashboard
static float seuilBas = SEUIL_BAS;
static float seuilHaut = SEUIL_HAUT;

// Lumiere ambiante de reference (detection incendie), -1 = pas encore initialisee
static float referenceLumiere = -1;
static int mesuresEnLumiereForte = 0;

// Buffer circulaire des dernieres temperatures (detection incendie)
static float temperatures[NB_ECHANTILLONS_TEMPERATURE];
static int indexTemperature = 0;
static int nbTemperatures = 0;

// Ajoute une temperature et retourne la moyenne des dernieres valeurs
static float temperature_average(float temperature) {
    temperatures[indexTemperature] = temperature;
    indexTemperature = (indexTemperature + 1) % NB_ECHANTILLONS_TEMPERATURE;

    // Au demarrage, on ne moyenne que les valeurs deja recues
    if (nbTemperatures < NB_ECHANTILLONS_TEMPERATURE) {
        nbTemperatures++;
    }

    float somme = 0;

    for (int i = 0; i < nbTemperatures; i++) {
        somme += temperatures[i];
    }

    return somme / nbTemperatures;
}

void sensor_service_init() {
    temperature_sensor_init();
    light_sensor_init();
}

SensorData sensor_service_read() {
    SensorData data;

    data.temperature = temperature_read();
    data.luminosite = light_sensor_read_average();

    return data;
}

float sensor_service_low_threshold() {
    return seuilBas;
}

float sensor_service_high_threshold() {
    return seuilHaut;
}

// Les bornes evitent des valeurs absurdes, et low < high garde la logique de regulation coherente
bool sensor_service_set_thresholds(float low, float high) {
    if (low < SEUIL_MIN || high > SEUIL_MAX || low >= high) {
        return false;
    }

    seuilBas = low;
    seuilHaut = high;

    return true;
}

// Regulation avec hysteresis (valeurs par defaut : seuils 25 / 26, hysteresis 0.3) :
//
//   T < 24.7          -> CHAUFFAGE
//   24.7 <= T <= 25.3 -> etat precedent conserve
//   25.3 <  T <  25.7 -> REPOS
//   25.7 <= T <= 26.3 -> etat precedent conserve
//   T > 26.3          -> CLIMATISATION
//
// Conserver l'etat precedent pres des seuils evite que le chauffage ou la clim
// s'allument et s'eteignent a chaque petite variation de la mesure.
EtatRegulation sensor_service_decide(float temperature, EtatRegulation previousState) {
    if (temperature > seuilHaut + HYSTERESIS) {
        return CLIMATISATION;
    }

    if (temperature < seuilBas - HYSTERESIS) {
        return CHAUFFAGE;
    }

    bool zoneNeutre = (temperature < seuilHaut - HYSTERESIS) && (temperature > seuilBas + HYSTERESIS);

    if (zoneNeutre) {
        return REPOS;
    }

    return previousState;
}

// Appelee uniquement en CLIMATISATION. Vitesse proportionnelle a l'ecart au seuil haut :
//   T <= seuil haut            -> VITESSE_MIN (minimum pour que le ventilateur demarre)
//   ecart >= ECART_VITESSE_MAX -> VITESSE_MAX
// Entre les deux, la vitesse augmente lineairement. Le cas T <= seuil haut arrive
// pendant l'hysteresis (clim encore active entre 25.7 et 26 avec les seuils par defaut) : le
// ventilateur reste alors au minimum au lieu de s'arreter.
int sensor_service_fan_speed(float temperature) {
    float ecart = temperature - seuilHaut;

    if (ecart < 0) {
        ecart = 0;
    }

    if (ecart > ECART_VITESSE_MAX) {
        ecart = ECART_VITESSE_MAX;
    }

    float proportion = ecart / ECART_VITESSE_MAX;

    return VITESSE_MIN + (int)(proportion * (VITESSE_MAX - VITESSE_MIN));
}

// Un feu produit de la lumiere ET de la chaleur : exiger les deux evite les
// fausses alertes (lampe seule, piece qui chauffe au soleil sans flamme).
// Les deux grandeurs sont moyennees sur plusieurs mesures pour qu'un pic
// isole (flash, reflet, lecture parasite) ne declenche pas l'alerte.
// La luminosite recue est deja moyennee (voir light_sensor_read_average).
//
// Pas de seuil fixe pour la lumiere : elle varie enormement selon le jour et
// le capteur. On la compare a une reference qui suit lentement la lumiere
// ambiante. Le capteur est inverse (plus de lumiere = valeur plus basse), donc
// "lumiere forte" = valeur nettement plus basse que la reference.
bool sensor_service_fire_detected(SensorData data) {
    float temperatureMoyenne = temperature_average(data.temperature);

    // Premiere mesure : la reference part de la lumiere actuelle
    if (referenceLumiere < 0) {
        referenceLumiere = data.luminosite;
    }

    bool lumiereForte = data.luminosite < referenceLumiere * (1 - VARIATION_LUMIERE_FEU);
    bool chaleur = temperatureMoyenne > SEUIL_FEU_TEMPERATURE;
    bool incendie = lumiereForte && chaleur;

    // La reference reste figee tant que la lumiere est forte : une lampe ou une
    // flamme qui dure ne doit pas devenir la nouvelle "lumiere normale" (le feu
    // reste detectable si la chaleur arrive apres la lumiere). Le gel est limite
    // a GEL_REFERENCE_MAX mesures pour qu'un changement durable de la lumiere du
    // jour (nuage qui s'ecarte) finisse par etre accepte comme normal.
    if (lumiereForte) {
        mesuresEnLumiereForte++;
    } else {
        mesuresEnLumiereForte = 0;
    }

    bool referenceFigee = lumiereForte && mesuresEnLumiereForte <= GEL_REFERENCE_MAX;

    if (!referenceFigee) {
        referenceLumiere += VITESSE_REFERENCE * (data.luminosite - referenceLumiere);
    }

    return incendie;
}

const char* sensor_service_state_name(EtatRegulation state) {
    switch (state) {
        case CLIMATISATION:
            return "CLIMATISATION";
        case CHAUFFAGE:
            return "CHAUFFAGE";
        default:
            return "REPOS";
    }
}