#include <Arduino.h>
#include "GyverEncoder.h"

const uint8_t PIN_ENC_A = 2;
const uint8_t PIN_ENC_B = 3;
const uint8_t PIN_ENC_SW = 4;

Encoder enc1(PIN_ENC_A, PIN_ENC_B, PIN_ENC_SW, TYPE1);

void isr() {
  enc1.tick();
}

void encoderInit() {
	attachInterrupt(digitalPinToInterrupt(2), isr, CHANGE);
}