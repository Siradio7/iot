// Bande NeoPixel : toute la bande affiche une meme couleur selon la temperature.

#ifndef LED_STRIP_H
#define LED_STRIP_H

enum StripColor {
    STRIP_GREEN,   // temperature sous le seuil bas
    STRIP_ORANGE,  // temperature entre les deux seuils
    STRIP_RED      // temperature au-dessus du seuil haut
};

void led_strip_init();
void led_strip_set_color(StripColor color);

#endif