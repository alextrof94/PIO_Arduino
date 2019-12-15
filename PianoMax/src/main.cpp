#include <Arduino.h>
#include "lcd.h"
#include "midi.h"
#include "encoder.h"
#include "metronome.h"
#include "liveleds.h"

const uint8_t SIMPLE_LEDS_PINS[] = {46, 44, 42, 40, 38, 36, 34, 32};

void setup() {
  lcdInit();
  midiInit();
}

void loop() {
  // put your main code here, to run repeatedly:
}