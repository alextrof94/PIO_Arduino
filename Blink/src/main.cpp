#include <Arduino.h>
#include <myleds.h>


void setup() {
  Serial.begin(115200);
  ledsInit();
}

void loop() {
  blink();
}