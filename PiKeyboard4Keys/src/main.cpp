#include <Arduino.h>
#include "Keyboard.h"


const uint8_t PIN_BTN_COL[4] = {11,10,9,A0};

uint32_t unpressDelay = 100;

struct MyBtn
{
  bool pressed = false;
  bool pressedOld = false;
  uint32_t pressTime = 0;

  uint8_t col = 0;

  uint8_t mod = 0;
  uint8_t mod2 = 0;
  uint8_t key = 0;

  void pressHotKey() {
    if (mod != 0)
      Keyboard.press(mod);
    delay(10);
    if (mod2 != 0)
      Keyboard.press(mod2);
    delay(10);
    if (key != 0)
      Keyboard.press(key);
    delay(10);
  }

  void releaseHotKey() {
    if (mod != 0)
      Keyboard.release(mod);
    delay(10);
    if (mod2 != 0)
      Keyboard.release(mod2);
    delay(10);
    if (key != 0)
      Keyboard.release(key); 
    delay(10);   
  }
  void change(bool val) {
    if (val && !pressedOld){
      pressed = true;
      pressTime = millis();
      pressHotKey();
      pressedOld = pressed;
    } 
    else if (!val && pressedOld) {
      if (millis() > pressTime + unpressDelay) {
        pressed = false;
        releaseHotKey();
        pressedOld = pressed;
      }
    }
  }
};
MyBtn btns[4];

void readBtns() {
  for (uint8_t c = 0; c < 4; c++) {
    btns[c].change(!digitalRead(PIN_BTN_COL[c]));
  }
}

void debugBtns() {
  Serial.println();
  for (uint8_t c = 0; c < 4; c++) {
    Serial.print(btns[c].pressed);
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, 1);

  for (uint8_t c = 0; c < 4; c++) {
    pinMode(PIN_BTN_COL[c], INPUT_PULLUP);
  }
  for (uint8_t c = 0; c < 4; c++) {
    btns[c].col = c;
  }

  btns[0].key = '1';
  btns[0].mod = KEY_LEFT_CTRL;
  btns[0].mod2 = KEY_LEFT_SHIFT;
  btns[1].key = '2';
  btns[1].mod = KEY_LEFT_CTRL;
  btns[1].mod2 = KEY_LEFT_SHIFT;
  btns[2].key = '3';
  btns[2].mod = KEY_LEFT_CTRL;
  btns[2].mod2 = KEY_LEFT_SHIFT;
  btns[3].key = '4';
  btns[3].mod = KEY_LEFT_CTRL;
  btns[3].mod2 = KEY_LEFT_SHIFT;
}

void loop() {
  readBtns();
  debugBtns();
}