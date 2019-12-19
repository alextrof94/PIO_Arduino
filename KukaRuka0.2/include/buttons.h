#ifndef __MY_BUTTONS
#define __MY_BUTTONS

#define PIN_74HC165_DATA 23
#define PIN_74HC165_LOAD 29
#define PIN_74HC165_CLOCK 31
#define PIN_74HC165_INH 33
#define SETTINGS_74HC165_DELAY 5

struct Button {
  uint8_t pin = 2;
  bool inversed = true;
  bool pullup = true;
  bool isPressed = false;
  bool isPressedOld = false;
  uint32_t pressTime;
  uint32_t releaseTime;
  bool pressProcessed = true;
  bool releaseProcessed = true;
};

void buttonsInit();

#endif 