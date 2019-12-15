#include <Arduino.h>

void setup() {
  pinMode(13, OUTPUT);
}

uint8_t b = false;

void loop() {
  b = !b;
  if (b > 3)
    b = 0;
    
  /* if */
  if (b == 0) {
    digitalWrite(13, 0);
  } else if (b == 1) {
    digitalWrite(13, 1);
  } else if (b == 2) {
    digitalWrite(13, 0);
  } else if (b == 3) {
    digitalWrite(13, 1);
  }
  /**/
  /* switch
  switch (b) {
    case 0: digitalWrite(13, 0); break;
    case 1: digitalWrite(13, 1); break;
    case 2: digitalWrite(13, 0); break;
    case 3: digitalWrite(13, 1); break;
  } 
  /**/
}