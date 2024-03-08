#include <Arduino.h>
#define FASTLED_MY_BLUEPILL 1 // FOR FASTLED
//#define F_CPU 72000000L
#include <FastLED.h>


#define NUM_LEDS 3
#define PIN_LEDS PB4
CRGB leds[NUM_LEDS];

const int MODE_IDLE = 0;
const int MODE_REACTION = 1;
const int MODE_ANSWER = 2;
const int MODE_JUDJE = 3;

int mode = MODE_IDLE;

/*
const CRGB COLOR_REACTION = CRGB::White; 
const CRGB COLOR_ANSWER = CRGB::Cyan; 
const CRGB COLOR_JUDJE = CRGB::Yellow; 

CRGB color = CRGB::Black;
*/
void setup() {
  Serial.begin(51400);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(PB4, OUTPUT);
  FastLED.addLeds<WS2812B, PIN_LEDS, GRB>(leds, NUM_LEDS);  // GRB ordering is typical
}

void loop() {

  delay(500);
  digitalWrite(LED_BUILTIN, 1);
  //digitalWrite(PB4, 1);
  
  leds[0] = CRGB::Red;
  leds[1] = CRGB::Green;
  leds[2] = CRGB::Blue;
  FastLED.show();
  
  
  delay(500);
  digitalWrite(LED_BUILTIN, 0);
  //digitalWrite(PB4, 0);
  
  leds[0] = CRGB::Black;
  leds[1] = CRGB::Black;
  leds[2] = CRGB::Black;
  FastLED.show();
  
  
}