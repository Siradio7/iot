// Parametres du projet : seuils, PWM, detection incendie, cadence et champs du JSON.

#ifndef CONFIG_H
#define CONFIG_H

// Liaison serie
#define SERIAL_BAUDRATE 9600         // bauds, doit correspondre a BAUD_RATE du bridge

// Regulation de temperature (en degres C)
#define SEUIL_BAS       25.0         // en dessous : chauffage
#define SEUIL_HAUT      26.0         // au-dessus : climatisation
#define HYSTERESIS       0.3         // marge autour des seuils pour eviter les basculements repetes
#define TEMPERATURE_ERROR -127.0     // valeur renvoyee par DallasTemperature si le capteur est absent

// Ventilateur (rapport cyclique PWM de 0 a 255)
#define VITESSE_MIN       80         // vitesse minimale pour que le ventilateur demarre
#define VITESSE_MAX       255        // pleine vitesse
#define ECART_VITESSE_MAX 5.0        // degres C au-dessus de SEUIL_HAUT pour atteindre VITESSE_MAX

#define PWM_FREQUENCE  25000         // Hz, frequence standard des ventilateurs PWM
#define PWM_RESOLUTION 8             // bits, soit des valeurs de 0 a 255

// Detection incendie (valeurs ADC de 0 a 4095)
#define SEUIL_INCENDIE 500           // incendie si la luminosite moyenne passe sous ce seuil
#define NB_ECHANTILLONS_LUMIERE 5   // taille de la moyenne glissante de luminosite

// Bande NeoPixel
#define LUMINOSITE_LEDS 50           // luminosite globale de la bande (0 a 255)

// Mesure
#define PERIODE_MESURE 1000          // ms entre deux mesures

// Modele JSON du cours : section info
#define INFO_IDENT "ESP32"
#define INFO_USER  "GR_F"
#define INFO_LOC   "A Biot"

// Modele JSON du cours : section location
#define LOCATION_ROOM    "312"
#define LOCATION_ADDRESS "Les lucioles"
#define LOCATION_LAT     43.62454
#define LOCATION_LON     7.050628

// Modele JSON du cours : section net (pas de WiFi, champs a "NOP")
#define NET_NOP "NOP"

// Modele JSON du cours : section reporthost (descriptif, non utilise par le bridge)
#define REPORT_TARGET_IP   "127.0.0.1"
#define REPORT_TARGET_PORT 1880
#define REPORT_SP          2

#endif