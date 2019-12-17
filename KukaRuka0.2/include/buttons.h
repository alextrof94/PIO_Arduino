#ifndef __MY_BUTTONS
#define __MY_BUTTONS

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