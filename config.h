// Parametres du projet : seuils, PWM, detection incendie, cadence et champs du JSON.

#ifndef CONFIG_H
#define CONFIG_H

// Liaison serie
#define SERIAL_BAUDRATE 9600         // bauds, doit correspondre a BAUD_RATE du bridge

// Regulation de temperature (en degres C)
// SEUIL_BAS et SEUIL_HAUT sont les seuils au demarrage : le dashboard peut les modifier ensuite
#define SEUIL_BAS       25.0         // en dessous : chauffage
#define SEUIL_HAUT      26.0         // au-dessus : climatisation
#define HYSTERESIS       0.3         // marge autour des seuils pour eviter les basculements repetes
#define TEMPERATURE_ERROR -127.0     // valeur renvoyee par DallasTemperature si le capteur est absent

// Bornes acceptees pour les seuils recus du dashboard (en degres C)
#define SEUIL_MIN       15.0
#define SEUIL_MAX       35.0

// Ventilateur (rapport cyclique PWM de 0 a 255)
#define VITESSE_MIN       80         // vitesse minimale pour que le ventilateur demarre
#define VITESSE_MAX       255        // pleine vitesse
#define ECART_VITESSE_MAX 5.0        // degres C au-dessus du seuil haut pour atteindre VITESSE_MAX

#define PWM_FREQUENCE  25000         // Hz, frequence standard des ventilateurs PWM
#define PWM_RESOLUTION 8             // bits, soit des valeurs de 0 a 255

// Detection incendie : lumiere forte ET chaleur, sur des moyennes glissantes
// (le capteur de lumiere est inverse : plus de lumiere = valeur ADC plus basse)
// La lumiere est comparee a une reference qui suit lentement la lumiere ambiante
// (jour/nuit) : seule une augmentation brusque (lampe, flamme) compte comme lumiere forte.
#define VARIATION_LUMIERE_FEU 0.3    // lumiere forte si la valeur est 30 % sous la reference
#define VITESSE_REFERENCE     0.02   // part de l'ecart rattrapee par la reference a chaque mesure
#define GEL_REFERENCE_MAX     120    // mesures (~2 min) pendant lesquelles la reference reste figee en lumiere forte
#define NB_ECHANTILLONS_LUMIERE 5    // taille de la moyenne glissante de luminosite
#define SEUIL_FEU_TEMPERATURE 28.0   // degres C : temperature moyenne au-dessus = chaleur (bas pour les tests)
#define NB_ECHANTILLONS_TEMPERATURE 5 // taille de la moyenne glissante de temperature

// Bande NeoPixel
#define LUMINOSITE_LEDS 50           // luminosite globale de la bande (0 a 255)

// Mesure
#define PERIODE_MESURE 1000          // ms entre deux mesures

// LittleFS : formater la partition seulement si elle ne peut pas etre montee
#define FORMAT_LITTLEFS_IF_FAILED true

// Commandes recues sur le port serie (une ligne JSON)
#define COMMAND_BUFFER_SIZE 64       // caracteres maximum par commande

// Modele JSON du cours : section info
#define INFO_IDENT "ESP32"
#define INFO_USER  "GR_F"
#define INFO_LOC   "A Biot"

// Modele JSON du cours : section location
#define LOCATION_ROOM    "312"
#define LOCATION_ADDRESS "Les lucioles"
#define LOCATION_LAT     43.62454
#define LOCATION_LON     7.050628

// Modele JSON du cours : section net ("NOP" si le WiFi n'est pas connecte)
#define NET_NOP "NOP"

// WiFi (points d'acces dans wifi_utils.cpp et wifi_secrets.h)
#define WIFI_HOSTNAME "ESP32-GRF"        // nom de l'ESP sur le reseau (sans espace)
#define WIFI_RESTART_IF_FAILED false     // true : redemarrer si aucun point d'acces (comme le cours)

// Modele JSON du cours : section reporthost (descriptif, non utilise par le bridge)
#define REPORT_TARGET_IP   "127.0.0.1"
#define REPORT_TARGET_PORT 1880
#define REPORT_SP          2

#endif