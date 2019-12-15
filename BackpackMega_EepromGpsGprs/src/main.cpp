#include "main.h"

void setup() {
  randomSeed(analogRead(A0));
  PC.begin(115200);
  GPS.begin(9600);
  GPRS.begin(9600);
  if (!SD.begin(PIN_SD_CHIPSELECT)) {
    Serial.println("Card failed, or not present");
  }
}

void loop() {
  mypcWork();
  if (gpsEnabled)
    mygpsWork();
  if (gprsEnabled)
    mygprsWork();
  if (sdEnabled)
    mysdWork();
  if (sensorEnabled) {
    uint16_t b = analogRead(PIN_LIGHTSENSOR);
    b /= 4; // to 0..255
    b *= brightnessSensorMultiplexer;
    if (b > 255)
      b = 255;
    brightness = map(b, 0, 255, 0, brightnessMax);
  }
  myledsWork();
}