// Brochage de l'ESP32 : tous les GPIO du projet sont definis ici.

#ifndef PINS_H
#define PINS_H

// Capteurs
#define PIN_TEMPERATURE 23   // bus OneWire du DS18B20
#define PIN_LUMINOSITE  33   // entree analogique de la photoresistance

// Actionneurs
#define PIN_CLIMATISATION 19
#define PIN_RADIATEUR     21
#define PIN_VENTILATEUR   27   // sortie PWM
#define PIN_BANDE_LEDS    13
#define PIN_ALERTE_FEU     2   // LED 2 : alerte incendie

// Bande NeoPixel
#define NB_LEDS 5            // nombre de LED de la bande

#endif