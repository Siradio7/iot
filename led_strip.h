#ifndef LED_STRIP_H
#define LED_STRIP_H

enum StripColor {
    STRIP_GREEN,
    STRIP_BLUE,
    STRIP_RED
};

void led_strip_init();
void led_strip_set_color(StripColor color);

#endif