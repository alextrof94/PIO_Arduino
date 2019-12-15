#include <FastLED.h>
#include <Arduino.h>

#define NUM_LEDS 256
#define DATA_PIN 2
CRGB leds[NUM_LEDS];

#define PIN_LEFT_STRIP_SELECT 3
#define PIN_RIGHT_STRIP_SELECT 4

void ledsInit() {
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
  digitalWrite(PIN_LEFT_STRIP_SELECT, 0);
  digitalWrite(PIN_RIGHT_STRIP_SELECT, 0);
}

void selectLedstrip(uint8_t which) {
	if (which == 0) {
		digitalWrite(PIN_LEFT_STRIP_SELECT, 1);
		digitalWrite(PIN_RIGHT_STRIP_SELECT, 0);
	} else {
		digitalWrite(PIN_LEFT_STRIP_SELECT, 0);
		digitalWrite(PIN_RIGHT_STRIP_SELECT, 1);
	}
}

uint8_t l = 0;

void blink() {
	selectLedstrip(0);
	leds[l] = 0xFF0000;
	FastLED.show();
	selectLedstrip(1);
	leds[l] = 0x00FF00;
	FastLED.show();
	delay(100);
	leds[l] = 0;
	selectLedstrip(0);
	FastLED.show();
	selectLedstrip(1);
	FastLED.show();
	l++;
}