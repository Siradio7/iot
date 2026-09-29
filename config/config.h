#ifndef CONFIG_H
#define CONFIG_H

// Serial
#define SERIAL_BAUDRATE 9600

// Temperature regulation
#define SEUIL_BAS       26.0
#define SEUIL_HAUT      28.0
#define HYSTERESIS       0.3
#define TEMPERATURE_ERROR -127.0

// Fan
#define VITESSE_MIN       80
#define VITESSE_MAX       255
#define ECART_VITESSE_MAX 5.0

#define PWM_FREQUENCE  25000
#define PWM_RESOLUTION 8

// Fire detection
#define SEUIL_INCENDIE 500

// NeoPixel
#define LUMINOSITE_LEDS 50

// Measurement
#define PERIODE_MESURE 1000

#endif