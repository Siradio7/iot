#ifndef CONFIG_H
#define CONFIG_H

// Serial
#define SERIAL_BAUDRATE 9600

// Temperature regulation
#define SEUIL_BAS       25.0
#define SEUIL_HAUT      26.0
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

// JSON model : info
#define INFO_IDENT "ESP32"
#define INFO_USER  "GR_F"
#define INFO_LOC   "A Biot"

// JSON model : location
#define LOCATION_ROOM    "312"
#define LOCATION_ADDRESS "Les lucioles"
#define LOCATION_LAT     43.62454
#define LOCATION_LON     7.050628

// JSON model : net (no WiFi yet)
#define NET_NOP "NOP"

// JSON model : reporthost
#define REPORT_TARGET_IP   "127.0.0.1"
#define REPORT_TARGET_PORT 1880
#define REPORT_SP          2

#endif