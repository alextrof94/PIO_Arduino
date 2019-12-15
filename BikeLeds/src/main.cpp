#include <Arduino.h>
#include "FastLED.h"
#include "EEPROM.h"

const uint16_t LEDS_COUNT = 41; // +26
CRGB ledsD[52];
CRGB ledsF[6];
CRGB ledsB[9];
#define PIN_LEDSTRIP_DOWN 5
#define PIN_LEDSTRIP_BACK 6
#define PIN_LEDSTRIP_FRONT 7
#define PIN_BTN_0 9
#define PIN_BTN_1 11
#define PIN_BTN_2 10
#define PIN_BTN_GND 12
#define PIN_BTN_P_GND A3
#define PIN_BTN_P_0 A5
#define PIN_POT_GND A0
#define PIN_POT_5V A2
#define PIN_POT A1

struct Button
{
  uint8_t pin;
  bool pullup = false;
  uint32_t pressedTime;
  uint32_t releaseTime;
  bool pressed;
  bool pressedOld;
  bool pressingWasProcessed = false;
};

#define BTN_COUNT 4
Button btns[BTN_COUNT];

void setup() {
  Serial.begin(57600);
  Serial.println("Bikeleds!");

  pinMode(PIN_BTN_P_GND, OUTPUT);
  digitalWrite(PIN_BTN_P_GND, 0);
  pinMode(PIN_BTN_GND, OUTPUT);
  digitalWrite(PIN_BTN_GND, 0);

  btns[0].pin = PIN_BTN_0;
  btns[1].pin = PIN_BTN_1;
  btns[2].pin = PIN_BTN_2;
  btns[3].pin = PIN_BTN_P_0;
  for (uint8_t i = 0; i < BTN_COUNT; i++) {
    btns[i].pullup = true;
    pinMode(btns[i].pin, INPUT_PULLUP);
  }


  FastLED.addLeds<WS2812B, PIN_LEDSTRIP_DOWN, GRB>(ledsD, 52);
  FastLED.addLeds<WS2812B, PIN_LEDSTRIP_BACK, GRB>(ledsB, 9);
  FastLED.addLeds<WS2812B, PIN_LEDSTRIP_FRONT, GRB>(ledsF, 6);
  for (uint8_t i = 0; i < 52; i++) {
    ledsD[i] = 0x220000;
    FastLED.show();
    delay(20);
  }
  for (uint8_t i = 0; i < 6; i++) {
    ledsF[i] = 0x002200;
    FastLED.show();
    delay(20);
  }
  for (uint8_t i = 0; i < 9; i++) {
    ledsB[i] = 0x000022;
    FastLED.show();
    delay(20);
  }
  for (uint8_t i = 0; i < 52; i++) {
    ledsD[i] = 0;
  }
  for (uint8_t i = 0; i < 6; i++) {
    ledsF[i] = 0;
  }
  for (uint8_t i = 0; i < 9; i++) {
    ledsB[i] = 0;
  }
  FastLED.show();
}



uint32_t modePoliceTimer = 0;
int modePoliceStage = 0;
int modePoliceStageCount = 12;
uint32_t modePoliceDelays[12] = {300, 30, 30, 30, 30, 200, 300, 30, 30, 30, 30, 200};
uint32_t modePoliceColors[12] = {0xFF0000, 0, 0xFF0000, 0, 0xFF0000, 0, 0x0000FF, 0, 0x0000FF, 0, 0x0000FF, 0};
void policeTimeCheck() {
  if (millis() > modePoliceTimer) {
    modePoliceStage++;
    if (modePoliceStage > modePoliceStageCount) 
      modePoliceStage = 0;
    modePoliceTimer = millis() + modePoliceDelays[modePoliceStage];
  }
}

uint32_t modeRainbowTimer = 0;
uint32_t modeRainbowDelay = 10;
uint8_t modeRainbowHue = 0;
CRGB modeRainbowColor;
void rainbowTimeCheck() {
  if (millis() > modeRainbowTimer) {
    modeRainbowTimer = millis() + modeRainbowDelay;
    modeRainbowHue++;
    modeRainbowColor = CHSV(modeRainbowHue, 255, 255);
  }
}

int modeDown = 3;
const int modeDownCount = 7;
/*
0 - off
1 - light
2 - blink
3 - police half-half
4 - police full
5 - running rainbow
6 - full rainbow
*/

void modeDown0(){
  for (uint8_t i = 0; i < 52; i++) {
    ledsD[i] = 0;
  }
}

void modeDown1(){
  for (uint8_t i = 0; i < 52; i++) {
    ledsD[i] = 0xFFFFFF;
  }
}

uint32_t modeDown2Timer = 0;
uint32_t modeDown2Delay = 200;
bool modeDown2Stage = false;
void modeDown2(){
  if (millis() > modeDown2Timer) {
    modeDown2Timer = millis() + modeDown2Delay;
    modeDown2Stage = !modeDown2Stage;
    for (uint8_t i = 0; i < 52; i++) {
      if (modeDown2Stage)
        ledsD[i] = 0x9900FF;
      else
        ledsD[i] = 0x050011;
    }
  }
}

void modeDown3(){
  policeTimeCheck();
  if (modePoliceStage < 6) {
    for (uint8_t i = 0; i < 13; i++)
      ledsD[i] = modePoliceColors[modePoliceStage];
    for (uint8_t i = 39; i < 52; i++)
      ledsD[i] = modePoliceColors[modePoliceStage];
    for (uint8_t i = 13; i < 39; i++)
      ledsD[i] = 0;
  } else {
    for (uint8_t i = 0; i < 13; i++)
      ledsD[i] = 0;
    for (uint8_t i = 39; i < 52; i++)
      ledsD[i] = 0;
    for (uint8_t i = 13; i < 39; i++)
      ledsD[i] = modePoliceColors[modePoliceStage];
  }
}

void modeDown4(){
  policeTimeCheck();
  for (uint8_t i = 0; i < 52; i++)
    ledsD[i] = modePoliceColors[modePoliceStage];
}

int pos = 0;
int dir = 1;
uint32_t modeDown5Timer = 0;
uint32_t modeDown5Delay = 20;
void modeDown5() {
  rainbowTimeCheck();
  if (millis() > modeDown5Timer) {
    modeDown5Timer = millis() + modeDown5Delay;
    pos += dir;
    if (pos == 25 || pos == 0)
      dir = -dir;
  }
  for (uint8_t i = 0; i < 52; i++)
    ledsD[i] = 0;
  ledsD[pos] = CHSV(modeRainbowHue, 255, 255);
  ledsD[51-pos] = CHSV(modeRainbowHue, 255, 255);
}

void modeDown6(){
  rainbowTimeCheck();
  for (uint8_t i = 0; i < 52; i++)
    ledsD[i] = modeRainbowColor;
}

int modeFrontBack = 0;
int modeFrontBackCount = 6;
int modeFrontBackLast = 0;
/*
0 - blink
1 - light
2 - back police
3 - all police
-1 - turn left
-2 - turn right
-3 - orange blink
*/

uint32_t modeFrontBack0Timer = 0;
uint32_t modeFrontBack0Delay = 500;
bool modeFrontBack0Stage = false;
void modeFrontBack0() {
  if (millis() > modeFrontBack0Timer) {
    modeFrontBack0Timer = millis() + modeFrontBack0Delay;
    modeFrontBack0Stage = !modeFrontBack0Stage;
    if (modeFrontBack0Stage) {
      for (uint8_t i = 0; i < 6; i++)
          ledsF[i] = 0xFFFFFF;
      for (uint8_t i = 0; i < 9; i++)
          ledsB[i] = 0xFF0000;
    } else {
      for (uint8_t i = 0; i < 6; i++)
          ledsF[i] = 0x111111;
      for (uint8_t i = 0; i < 9; i++)
          ledsB[i] = 0x110000;
    }
  }
}

void modeFrontBack1() {
  for (uint8_t i = 0; i < 6; i++)
      ledsF[i] = 0xFFFFFF;
  for (uint8_t i = 0; i < 9; i++)
      ledsB[i] = 0xFF0000;
}

void modeFrontBack2() {
  policeTimeCheck();
  for (uint8_t i = 0; i < 9; i++)
    ledsB[i] = modePoliceColors[modePoliceStage];
  for (uint8_t i = 0; i < 6; i++)
    ledsF[i] = 0xFFFFFF;
}

void modeFrontBack3() {
  policeTimeCheck();
  for (uint8_t i = 0; i < 9; i++)
    ledsB[i] = modePoliceColors[modePoliceStage];
  for (uint8_t i = 0; i < 6; i++)
    ledsF[i] = modePoliceColors[modePoliceStage];
}

void modeFrontBack4() {
  rainbowTimeCheck();
  for (uint8_t i = 0; i < 9; i++)
    ledsB[i] = modeRainbowColor;
  for (uint8_t i = 0; i < 6; i++)
    ledsF[i] = modeRainbowColor;
}

void modeFrontBack5() {
  for (uint8_t i = 0; i < 9; i++)
    ledsB[i] = 0x010000;
  for (uint8_t i = 0; i < 6; i++)
    ledsF[i] = 0x010101;
}

uint32_t modeFrontBack_1Timer = 0;
uint32_t modeFrontBack_1Delay = 100;
bool modeFrontBack_1Stage = false;
void modeFrontBack_1() {
  if (millis() > modeFrontBack_1Timer) {
    modeFrontBack_1Timer = millis() + modeFrontBack_1Delay;
    modeFrontBack_1Stage = !modeFrontBack_1Stage;
  }
  for (uint8_t i = 0; i < 6; i++)
    ledsF[i] = 0;
  for (uint8_t i = 0; i < 9; i++)
    ledsB[i] = 0;
  for (uint8_t i = 2; i < 4; i++)
    ledsF[i] = 0xFFFFFF;
  for (uint8_t i = 3; i < 6; i++)
    ledsB[i] = 0xFF0000;
  if (modeFrontBack_1Stage) {
    for (uint8_t i = 0; i < 2; i++)
      ledsF[i] = 0xFF5500;
    for (uint8_t i = 0; i < 3; i++)
      ledsB[i] = 0xFF5500;
  }
}

uint32_t modeFrontBack_2Timer = 0;
uint32_t modeFrontBack_2Delay = 100;
bool modeFrontBack_2Stage = false;
void modeFrontBack_2() {
  if (millis() > modeFrontBack_2Timer) {
    modeFrontBack_2Timer = millis() + modeFrontBack_2Delay;
    modeFrontBack_2Stage = !modeFrontBack_2Stage;
  }
  for (uint8_t i = 0; i < 6; i++)
    ledsF[i] = 0;
  for (uint8_t i = 0; i < 9; i++)
    ledsB[i] = 0;
  for (uint8_t i = 2; i < 4; i++)
    ledsF[i] = 0xFFFFFF;
  for (uint8_t i = 3; i < 6; i++)
    ledsB[i] = 0xFF0000;
  if (modeFrontBack_2Stage) {
    for (uint8_t i = 4; i < 6; i++)
      ledsF[i] = 0xFF5500;
    for (uint8_t i = 6; i < 9; i++)
      ledsB[i] = 0xFF5500;
  }
}

uint32_t modeFrontBack_3Timer = 0;
uint32_t modeFrontBack_3Delay = 100;
bool modeFrontBack_3Stage = false;
void modeFrontBack_3() {
  if (millis() > modeFrontBack_3Timer) {
    modeFrontBack_3Timer = millis() + modeFrontBack_3Delay;
    modeFrontBack_3Stage = !modeFrontBack_3Stage;
  }
  for (uint8_t i = 0; i < 6; i++)
    ledsF[i] = 0;
  for (uint8_t i = 0; i < 9; i++)
    ledsB[i] = 0;
  for (uint8_t i = 2; i < 4; i++)
    ledsF[i] = 0xFFFFFF;
  for (uint8_t i = 3; i < 6; i++)
    ledsB[i] = 0xFF0000;
  if (modeFrontBack_3Stage) {
    for (uint8_t i = 0; i < 2; i++)
      ledsF[i] = 0xFF5500;
    for (uint8_t i = 0; i < 3; i++)
      ledsB[i] = 0xFF5500;
    for (uint8_t i = 4; i < 6; i++)
      ledsF[i] = 0xFF5500;
    for (uint8_t i = 6; i < 9; i++)
      ledsB[i] = 0xFF5500;
  }
}

void bl() {
    for(uint8_t i = 0; i < 52; i++)
      ledsD[i] = 0xFFFFFF;
    FastLED.show();
    delay(100);
    for(uint8_t i = 0; i < 52; i++)
      ledsD[i] = 0;
    FastLED.show();
}

void loop() {
  for (uint8_t i = 0; i < BTN_COUNT; i++) {
    btns[i].pressed = digitalRead(btns[i].pin);
    if (btns[i].pullup)
      btns[i].pressed = !btns[i].pressed;

    if (btns[i].pressed && !btns[i].pressedOld){
      btns[i].pressedTime = millis();
      btns[i].pressedOld = true;
    } else
    if (!btns[i].pressed && btns[i].pressedOld) {
      btns[i].releaseTime = millis();
      btns[i].pressedOld = false;
      btns[i].pressingWasProcessed = false;
    }
  }
  if (!btns[1].pressingWasProcessed) {
    btns[1].pressingWasProcessed = true;
    if (btns[1].releaseTime - btns[1].pressedTime > 200) {
      bl();
      modeDown++;
      if (modeDown >= modeDownCount)
        modeDown = 0;
    }
    else {
      if (modeFrontBack < 0)
        modeFrontBack = modeFrontBackLast;
      else
        modeDown = 0;
    }
  }
  if (!btns[0].pressingWasProcessed) {
    btns[0].pressingWasProcessed = true;
    if (btns[0].releaseTime - btns[0].pressedTime > 1000) {
      modeFrontBack = -3;
    }
    else {
      if (modeFrontBack < 0)
        modeFrontBack = modeFrontBackLast;
      else {
        modeFrontBackLast = modeFrontBack;
        modeFrontBack = -1;
      }
    }
  }
  if (!btns[2].pressingWasProcessed) {
    btns[2].pressingWasProcessed = true;
    if (btns[2].releaseTime - btns[2].pressedTime > 1000) {
      if (modeFrontBack < 0)
        modeFrontBack = modeFrontBackLast;
      modeFrontBack++;
      if (modeFrontBack >= modeFrontBackCount)
        modeFrontBack = 0;
    }
    else {
      if (modeFrontBack < 0)
        modeFrontBack = modeFrontBackLast;
      else {
        modeFrontBackLast = modeFrontBack;
        modeFrontBack = -2;
      }
    }
  }

  switch (modeDown) {
    case 0: modeDown0(); break;
    case 1: modeDown1(); break;
    case 2: modeDown2(); break;
    case 3: modeDown3(); break;
    case 4: modeDown4(); break;
    case 5: modeDown5(); break;
    case 6: modeDown6(); break;
    default: modeDown0(); break;
  }
  switch (modeFrontBack) {
    case 0: modeFrontBack0(); break;
    case 1: modeFrontBack1(); break;
    case 2: modeFrontBack2(); break;
    case 3: modeFrontBack3(); break;
    case 4: modeFrontBack4(); break;
    case 5: modeFrontBack5(); break;
    case -1: modeFrontBack_1(); break;
    case -2: modeFrontBack_2(); break;
    case -3: modeFrontBack_3(); break;
    default: modeFrontBack0(); break;
  }
  FastLED.show();
}