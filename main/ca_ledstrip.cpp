#include <esp_log.h>
#include <FastLED.h>

#include "ca_ledstrip.h"

#define DATA_PIN 26
#define NUM_LEDS 14

// Define the array of leds
static CRGB leds[NUM_LEDS];

void ca_ledstrip_setup(void) {
    FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);

    for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = (i % 2 == 0) ? CRGB::Red : (i % 3 == 0) ? CRGB::Green
                                                          : CRGB::Blue;
    }

    FastLED.show();
}