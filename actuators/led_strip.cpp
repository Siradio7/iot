#include "led_strip.h"

#include "../config/pins.h"
#include "../config/config.h"

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

static Adafruit_NeoPixel strip(
    NB_LEDS,
    PIN_BANDE_LEDS,
    NEO_GRB + NEO_KHZ800
);

void led_strip_init() {
    strip.begin();
    strip.setBrightness(LUMINOSITE_LEDS);
    strip.show();
}

void led_strip_set_color(StripColor color) {

    uint32_t rgb;

    switch (color) {
        case STRIP_RED:
            rgb = strip.Color(255, 0, 0);
            break;

        case STRIP_BLUE:
            rgb = strip.Color(0, 0, 255);
            break;

        case STRIP_GREEN:
        default:
            rgb = strip.Color(0, 255, 0);
            break;
    }

    for (int i = 0; i < NB_LEDS; i++) {
        strip.setPixelColor(i, rgb);
    }

    strip.show();
}