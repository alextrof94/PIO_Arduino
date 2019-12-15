#include <Arduino.h>
#include "FastLED.h"

const uint16_t LEDS_COUNT = 288;
CRGB leds[LEDS_COUNT];

void setup() {
  Serial.begin(57600);
  Serial.println("MEGA DigitalWheel!");
  FastLED.addLeds<WS2812B, 7, GRB>(leds, LEDS_COUNT);
  for (uint8_t i = 0; i < LEDS_COUNT; i++) {
    leds[i] = 0x222222;
    FastLED.show();
  }
  for (uint8_t i = 0; i < LEDS_COUNT; i++) {
    leds[i] = 0;
  }
  FastLED.show();
}

void loop() {
  // put your main code here, to run repeatedly:
}