#ifndef LED_H
#define LED_H

void led_init();

void led_climatisation(bool active);
void led_radiateur(bool active);
void led_alerte_feu(bool active);

#endif