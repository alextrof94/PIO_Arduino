#include <Arduino.h>
#include "Keyboard.h"

const uint8_t PIN_LED_POWER = A3;
const uint8_t PIN_LED_LAYOUT1 = 5;
const uint8_t PIN_LED_LAYOUT2 = 6;
const uint8_t PIN_LED_LAYOUT3 = 3;
const uint8_t PIN_LED_MIC1 = 10;
const uint8_t PIN_LED_MIC2 = 9;

const uint8_t PIN_SW_POWER = A2;
const uint8_t PIN_BTN_COL[6] = {16,A1,8,7,4,2};
const uint8_t PIN_BTN_ROW[3] = {14,15,A0};

bool enabled = false;

uint8_t selectedLayout = 0;
uint8_t selectedLayoutOld = 0;
const char charLayouts[3][2][6] = {
  {
    {
      '1','2','3','4','5','6'
    },
    {
      '7','8','9','0','a','b'
    }
  },
  {
    {
      'c','d','e','f','g','h'
    },
    {
      'i','j','k','l','m','n'
    }
  },
  {
    {
      'o','p','q','r','s','t'
    },
    {
      'u','v','w','x','y','z'
    }
  }
};

bool discordMicro = false;
bool obsMicro = false;

uint32_t unpressDelay = 100;

struct MyBtn
{
  bool pressed = false;
  bool pressedOld = false;
  uint32_t pressTime = 0;

  uint8_t row = 0;
  uint8_t col = 0;

  uint8_t mod = 0;
  uint8_t mod2 = 0;
  uint8_t key = 0;

  void pressHotKey() {
    if (mod != 0) {
      Keyboard.press(mod);
      delay(10);
    }
    if (mod2 != 0) {
      Keyboard.press(mod2);
      delay(10);
    }
    if (key != 0) {
      Keyboard.press(key);
      delay(10);
    }
  }

  void releaseHotKey() {
    if (mod != 0) {
      Keyboard.release(mod);
      delay(10);
    }
    if (mod2 != 0) {
      Keyboard.release(mod2);
      delay(10);
    }
    if (key != 0) {
      Keyboard.release(key); 
      delay(10);
    }
  }

  void specialRelease(uint32_t pressedTime) {
    if (row == 0 && col == 0) {
      obsMicro = 1;
      digitalWrite(PIN_LED_MIC1, obsMicro);
    }
    if (row == 0 && col == 1) {
      obsMicro = 0;
      digitalWrite(PIN_LED_MIC1, obsMicro);
    }
    if (row == 0 && col == 2) {
      if (pressedTime < 1000)
        discordMicro = !discordMicro;
      digitalWrite(PIN_LED_MIC2, discordMicro);
    }
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
        specialRelease(millis() - pressTime);
        pressedOld = pressed;
      }
    }
  }
};

MyBtn btns[3][6];

bool micEnabled1 = false;
bool micEnabled2 = false;
//
void showLayout(uint8_t defLightness = 20) {
  switch (selectedLayout)
  {
    case 0: digitalWrite(PIN_LED_LAYOUT2, 0); digitalWrite(PIN_LED_LAYOUT3, 0); analogWrite(PIN_LED_LAYOUT1, defLightness); break;
    case 1: digitalWrite(PIN_LED_LAYOUT1, 0); digitalWrite(PIN_LED_LAYOUT3, 0); analogWrite(PIN_LED_LAYOUT2, defLightness); break;
    case 2: digitalWrite(PIN_LED_LAYOUT1, 0); digitalWrite(PIN_LED_LAYOUT2, 0); analogWrite(PIN_LED_LAYOUT3, defLightness); break;
    default: break;
  }
}

void startup() {
  digitalWrite(PIN_LED_LAYOUT1, 1);
  digitalWrite(PIN_LED_LAYOUT2, 1);
  digitalWrite(PIN_LED_LAYOUT3, 1);
  digitalWrite(PIN_LED_MIC1, 1);
  digitalWrite(PIN_LED_MIC2, 1);
  delay(300);
  digitalWrite(PIN_LED_LAYOUT1, 0);
  digitalWrite(PIN_LED_LAYOUT2, 0);
  digitalWrite(PIN_LED_LAYOUT3, 0);
  digitalWrite(PIN_LED_MIC1, 0);
  digitalWrite(PIN_LED_MIC2, 0);

  showLayout();
}

void readBtns() {
  for (uint8_t r = 0; r < 3; r++) {
    digitalWrite(PIN_BTN_ROW[r], 1);
    for (uint8_t c = 0; c < 6; c++) {
      btns[r][c].change(digitalRead(PIN_BTN_COL[c]));
    }
    digitalWrite(PIN_BTN_ROW[r], 0);
  }
}

void debugBtns() {
  Serial.println();
  for (uint8_t r = 0; r < 3; r++) {
    for (uint8_t c = 0; c < 6; c++) {
      Serial.print(btns[r][c].pressed);
    }
    Serial.println();
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_LED_POWER, OUTPUT);
  pinMode(PIN_LED_LAYOUT1, OUTPUT);
  pinMode(PIN_LED_LAYOUT2, OUTPUT);
  pinMode(PIN_LED_LAYOUT3, OUTPUT);
  pinMode(PIN_LED_MIC1, OUTPUT);
  pinMode(PIN_LED_MIC2, OUTPUT);

  pinMode(PIN_SW_POWER, INPUT_PULLUP);

  for (uint8_t r = 0; r < 3; r++) {
    pinMode(PIN_BTN_ROW[r], OUTPUT);
    digitalWrite(PIN_BTN_ROW[r], 0);
  }
  for (uint8_t c = 0; c < 6; c++) {
    pinMode(PIN_BTN_COL[c], INPUT);
  }
  for (uint8_t r = 0; r < 3; r++) {
    for (uint8_t c = 0; c < 6; c++) {
      btns[r][c].row = r;
      btns[r][c].col = c;
    }
  }

  btns[0][0].mod = KEY_LEFT_CTRL;
  btns[0][0].mod2 = KEY_LEFT_ALT;
  btns[0][0].key = '5';
  btns[0][1].mod = KEY_LEFT_CTRL;
  btns[0][1].mod2 = KEY_LEFT_ALT;
  btns[0][1].key = '4';
  btns[0][2].mod = KEY_LEFT_CTRL;
  btns[0][2].mod2 = KEY_LEFT_ALT;
  btns[0][2].key = '6';
  for (uint8_t r = 1; r < 3; r++) {
    for (uint8_t c = 0; c < 6; c++) {
      btns[r][c].mod = KEY_LEFT_CTRL;
      btns[r][c].mod2 = KEY_LEFT_SHIFT;
      btns[r][c].key = charLayouts[selectedLayout][r-1][c];
    }
  }
}


bool keyPressed = false;
void loop() {
  if (!digitalRead(PIN_SW_POWER)) {
    if (!enabled) {
      enabled = true;
      digitalWrite(PIN_LED_POWER, enabled);
      startup();
      digitalWrite(PIN_LED_MIC1, obsMicro);
      digitalWrite(PIN_LED_MIC2, discordMicro);
    }
  } else {
    if (enabled) {
      enabled = false;
      digitalWrite(PIN_LED_POWER, enabled);
      digitalWrite(PIN_LED_LAYOUT1, 0);
      digitalWrite(PIN_LED_LAYOUT2, 0);
      digitalWrite(PIN_LED_LAYOUT3, 0);
      digitalWrite(PIN_LED_MIC1, 0);
      digitalWrite(PIN_LED_MIC2, 0);
    }
    delay(10);
  }
  if (!enabled)
    return;

  readBtns();
  //debugBtns();
  if (btns[0][3].pressed)
    selectedLayout = 0;
  if (btns[0][4].pressed)
    selectedLayout = 1;
  if (btns[0][5].pressed)
    selectedLayout = 2;
  if (selectedLayoutOld != selectedLayout) {
    selectedLayoutOld = selectedLayout;
    for (uint8_t r = 1; r < 3; r++) {
      for (uint8_t c = 0; c < 6; c++) {
        btns[r][c].key = charLayouts[selectedLayout][r-1][c];
      }
    }
  }

  keyPressed = false;
  for (uint8_t r = 0; r < 3; r++) {
    for (uint8_t c = 0; c < 6; c++) {
      if (btns[r][c].pressed)
        keyPressed = true;
    }
  }
  if (keyPressed) {
    showLayout(255);
  }
  else {
    showLayout();
  }
}