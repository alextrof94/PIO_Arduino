#include <Arduino.h>
#include "FastLED.h"

const uint8_t DATA_PIN = 7;
const uint8_t LEDS_COUNT = 61;
CRGB leds[LEDS_COUNT];

const uint32_t LEDS_UPDATE_DELAY = 10;
uint32_t ledsUpdateTimer = 0;

void liveledsInit() {
	FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, LEDS_COUNT);
	for (uint8_t i = 0; i < LEDS_COUNT; i++) {
		leds[i] = 0;
	}
	FastLED.show();
}