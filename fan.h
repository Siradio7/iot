// Ventilateur pilote en PWM.

#ifndef FAN_H
#define FAN_H

void fan_init();
// Vitesse de 0 (arret) a VITESSE_MAX, bornee si hors limites
void fan_set_speed(int speed);

#endif