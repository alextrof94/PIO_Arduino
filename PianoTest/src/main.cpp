#include <Arduino.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include "FastLED.h"
#include "GyverEncoder.h"

const uint8_t PIN_BTNS[10] = {22, 23, 24, 25, 26, 27, 28, 29, 30, 31};

struct Btn {
  uint8_t pin;
  uint8_t mode = INPUT_PULLUP;
  bool isInversed = false;
  bool isPressed = false;
  bool isPressedOld = false;
  uint32_t timePressed;
  uint32_t timeReleased;
  bool isProcessed = true;
};

Btn btns[10];

const uint8_t PIN_BUZZER = 8;

Encoder enc1(2, 3, 4, TYPE1);

#define MIDI Serial3

bool buzzerEnable = false;
bool buzzerWaitOff = false;
uint32_t buzzerTimer = 0;
uint32_t buzzerDelay = 10000;
uint8_t buzzerSound = 255;
uint32_t metronomTimer = 0;
uint32_t metronomDelay = 500000;
uint16_t metronomBPM = 120;
uint32_t screenMetronomTimer = 0;
uint32_t screenSettingsDelay = 1000;

const uint8_t NOTE_FIRST = 36;

const uint8_t DATA_PIN = 7;
CRGB leds[61];
LiquidCrystal_I2C lcd(0x3F,20,2);

const uint32_t TIMEOUT_DELAY = 10;
uint32_t timeoutTime = 0;
bool timeoutWait = false;

struct MidiMsg {
  uint8_t ch = 0;
  uint8_t note = 0;
  uint8_t vel = 0;
};

const uint32_t NOTE_LIFETIME = 100;

struct Note {
  uint8_t status = 0; // 0 - dead, 1 - pressed, 2 - unpressed
  uint8_t statusPrev = 0;
  uint32_t lifetime = 0; // after unpress
};
Note notes[61];

const uint32_t LEDS_UPDATE_DELAY = 10;
uint32_t ledsUpdateTimer = 0;

const uint32_t LCD_UPDATE_DELAY = 200;
uint32_t lcdUpdateTimer = 0;

uint8_t simpleLedsIndex = 0;
const uint8_t SIMPLE_LEDS_PINS[] = {46, 44, 42, 40, 38, 36, 34, 32};

void isr() {
  enc1.tick();
}

void checkButtons() {
  for (uint8_t i = 0; i < 10; i++){
    btns[i].isPressed = digitalRead(btns[i].pin);
    if (btns[i].isInversed)
      btns[i].isPressed = !btns[i].isPressed;
    if (btns[i].isPressed && !btns[i].isPressedOld){
      btns[i].timePressed = millis();
    } 
    else 
    if (!btns[i].isPressed && btns[i].isPressedOld){
      btns[i].timeReleased = millis();
      btns[i].isProcessed = false;
    }
    btns[i].isPressedOld = btns[i].isPressed;
  }
}

int mode = -3;
bool modeFirstStart = true;

uint32_t mode_1UpdateTime;
uint32_t mode_1UpdateDelay = 100;
void mode_1(){
  if (modeFirstStart) {
    modeFirstStart = false;
    for (uint8_t i = 0; i < 60; i++) {
      notes[i].lifetime = 0;
      notes[i].status = 0;
      leds[i] = 0;
      FastLED.show();
    }
  }
  if (millis() > mode_1UpdateTime) {
    mode_1UpdateTime = millis() + mode_1UpdateDelay;
    mode_1UpdateDelay = random(100, 200);
    for (uint8_t i = 0; i < 60; i++)
      if (notes[i].status == 1)
        notes[i].status = 2;
    uint8_t r = random(1, 3);
    for (uint8_t i = 0; i < r; i++){
      notes[random(0, 60)].status = 1;
      notes[random(0, 60)].lifetime = 100;
    }
  }
  if (millis() > ledsUpdateTimer) {
    ledsUpdateTimer = millis() + LEDS_UPDATE_DELAY;
    for (uint8_t i = 0; i < 61; i++) {
      if (notes[i].status == 0)
        leds[i] = 0;
      if (notes[i].status == 1 && notes[i].statusPrev == 0) {
        switch (random(0, 6)){
          case 0: leds[i] = CRGB(random(200,255), 0, random(200,255)); break;
          case 1: leds[i] = CRGB(random(200,255), 0, 0); break;
          case 2: leds[i] = CRGB(0, 0, random(200,255)); break;
          case 3: leds[i] = CRGB(0, random(100,255), random(100,255)); break;
          case 4: leds[i] = CRGB(random(100,255), random(100,255), 0); break;
          case 5: leds[i] = CRGB(random(100,255), random(100,255), random(100,255)); break;
        }
      }
      if (notes[i].status == 2) {
        notes[i].lifetime--;
        if (notes[i].lifetime == 0)
          notes[i].status = 0;
        leds[i].fadeToBlackBy(3);
      }
      notes[i].statusPrev = notes[i].status;
    }
    FastLED.show();
  }
}

uint32_t modeRedUpdateTime;
uint32_t modeRedUpdateDelay = 50;
uint32_t modeRedIndex = 10;
uint32_t modeRedIndexDir = 1;
void modeRed(){
  if (modeFirstStart) {
    modeFirstStart = false;
    for (uint8_t i = 0; i < 60; i++) {
      notes[i].lifetime = 0;
      notes[i].status = 0;
      leds[i] = 0;
      FastLED.show();
    }
  }
  if (millis() > modeRedUpdateTime) {
    modeRedUpdateTime = millis() + modeRedUpdateDelay;
    modeRedIndex += modeRedIndexDir;
    if (modeRedIndex == 0 || modeRedIndex == 29)
      modeRedIndexDir = -modeRedIndexDir;
    notes[modeRedIndex].status = 2;
    notes[modeRedIndex].lifetime = 100;
    notes[59 - modeRedIndex].status = 2;
    notes[59 - modeRedIndex].lifetime = 100;
    
    leds[modeRedIndex] = CRGB(255,0,0);
    leds[59-modeRedIndex] = CRGB(255,0,0);
  }
  if (millis() > ledsUpdateTimer) {
    ledsUpdateTimer = millis() + LEDS_UPDATE_DELAY;
    for (uint8_t i = 0; i < 60; i++) {
      if (notes[i].status == 0)
        leds[i] = 0;
      if (notes[i].status == 2) {
        notes[i].lifetime--;
        if (notes[i].lifetime == 0)
          notes[i].status = 0;
        leds[i].fadeToBlackBy(20);
      }
    }
    FastLED.show();
  }
}

void modeRedLine(){
  if (modeFirstStart) {
    modeFirstStart = false;
    for (uint8_t i = 0; i < 60; i++) {
      leds[i] = CRGB(150,0,0);
    }
    FastLED.show();
  }
}

void mode0(){
  if (modeFirstStart) {
    modeFirstStart = false;
    for (uint8_t i = 0; i < 60; i++) {
      notes[i].lifetime = 0;
      notes[i].status = 0;
      leds[i] = 0;
      FastLED.show();
    }
  }
  if (enc1.isClick()) {
    buzzerEnable = !buzzerEnable;
    screenMetronomTimer = millis() + screenSettingsDelay;
  }
  if (enc1.isRightH()) {
    if (buzzerSound < 255){
      if (buzzerSound > 250)
        buzzerSound += 1;
      else
      if (buzzerSound > 150)
        buzzerSound += 10;
      else
      if (buzzerSound > 50)
        buzzerSound += 5;
      else
      if (buzzerSound > 10)
        buzzerSound += 2;
      else
        buzzerSound += 1;
    }
    screenMetronomTimer = millis() + screenSettingsDelay;
  }
  if (enc1.isLeftH()) {
    if (buzzerSound > 0){
      if (buzzerSound > 150)
        buzzerSound -= 10;
      else
      if (buzzerSound > 50)
        buzzerSound -= 5;
      else
      if (buzzerSound > 10)
        buzzerSound -= 2;
      else
        buzzerSound -= 1;
    }
    if (buzzerSound > 0)
      buzzerSound--;
    screenMetronomTimer = millis() + screenSettingsDelay;
  }
  if (enc1.isRight() && !enc1.isHold()) {
    metronomBPM++;
    metronomDelay = 60000000 / metronomBPM;
    metronomTimer = micros() + metronomDelay;
    screenMetronomTimer = millis() + screenSettingsDelay;
  }
  if (enc1.isFastR() && !enc1.isHold()) {
    metronomBPM+=3;
    metronomDelay = 60000000 / metronomBPM;
    metronomTimer = micros() + metronomDelay;
    screenMetronomTimer = millis() + screenSettingsDelay;
  }
  if (enc1.isLeft() && !enc1.isHold()) {
    metronomBPM--;
    if (metronomBPM < 1)
      metronomBPM = 1;
    metronomDelay = 60000000 / metronomBPM;
    metronomTimer = micros() + metronomDelay;
    screenMetronomTimer = millis() + screenSettingsDelay;
  }
  if (enc1.isFastL() && !enc1.isHold()) {
    metronomBPM-=3;
    if (metronomBPM < 1 || metronomBPM > 65000)
      metronomBPM = 1;
    metronomDelay = 60000000 / metronomBPM;
    metronomTimer = micros() + metronomDelay;
    screenMetronomTimer = millis() + screenSettingsDelay;
  }
  if (micros() >= metronomTimer) {
    uint32_t m = micros();
    metronomTimer = m + metronomDelay - (m - metronomTimer);
    digitalWrite(SIMPLE_LEDS_PINS[simpleLedsIndex], 0);
    simpleLedsIndex++;
    if (simpleLedsIndex > 7)
      simpleLedsIndex = 0;
    digitalWrite(SIMPLE_LEDS_PINS[simpleLedsIndex], 1);
    if (buzzerEnable) {
      analogWrite(PIN_BUZZER, buzzerSound);
      buzzerTimer = m + buzzerDelay;
      buzzerWaitOff = true;
    }
  }
  if ((buzzerEnable || buzzerWaitOff) && micros() >= buzzerTimer) {
    digitalWrite(PIN_BUZZER, 0);
    buzzerWaitOff = false;
  }
  if (timeoutWait && millis() > timeoutTime) {
    while (MIDI.available())
      MIDI.read();
    timeoutWait = false;
  }
  if (MIDI.available() > 0) {
    timeoutTime = millis() + TIMEOUT_DELAY;
    timeoutWait = true;
  }
  if (MIDI.available() > 2) {
    MidiMsg msg;
    msg.ch = MIDI.read();
    msg.note = MIDI.read();
    msg.vel = MIDI.read();
    /* 
    Serial.print(msg.ch);
    Serial.print(" ");
    Serial.print(msg.note);
    Serial.print(" ");
    Serial.println(msg.vel);
    /* */
    timeoutWait = false;
    if (msg.vel > 0) {
      notes[60 - (msg.note - NOTE_FIRST)].status = 1;
      notes[60 - (msg.note - NOTE_FIRST)].lifetime = 0;
      notes[60 - (msg.note - NOTE_FIRST)].statusPrev = 0;
    } else {
      notes[60 - (msg.note - NOTE_FIRST)].status = 2;
      notes[60 - (msg.note - NOTE_FIRST)].lifetime = NOTE_LIFETIME;
    }
  }
  
  if (millis() > lcdUpdateTimer) {
    lcdUpdateTimer = millis() + LCD_UPDATE_DELAY;
    lcd.clear();
    lcd.setCursor(0,0);
    if (screenMetronomTimer > millis()) {
      lcd.print("BPM: ");
      lcd.print(metronomBPM);
      lcd.setCursor(0,1);
      lcd.print("EN: ");
      lcd.print(buzzerEnable);
      lcd.print(" SND: ");
      lcd.print(buzzerSound);
      lcd.print(" ");
    } else {
      uint8_t a = 0;
      for (uint8_t i = 0; i < 61 && a < 14; i++) {
        if (notes[i].status == 1) {
          lcd.print((60 - i) + NOTE_FIRST);
          lcd.print(" ");
          a++;
          if (a == 7)
            lcd.setCursor(0,1);
        }
      }
    }
  }

  if (millis() > ledsUpdateTimer) {
    ledsUpdateTimer = millis() + LEDS_UPDATE_DELAY;
    for (uint8_t i = 0; i < 61; i++) {
      if (notes[i].status == 0)
        leds[i] = 0;
      if (notes[i].status == 1 && notes[i].statusPrev == 0) {
        leds[i] = CRGB(random(100,255), random(100,255), random(100,255));
      }
      if (notes[i].status == 2) {
        notes[i].lifetime--;
        if (notes[i].lifetime == 0)
          notes[i].status = 0;
        leds[i].fadeToBlackBy(10);
      }
      notes[i].statusPrev = notes[i].status;
    }
    FastLED.show();
  }
}

void setup() {
  attachInterrupt(digitalPinToInterrupt(2), isr, CHANGE);
  for (uint8_t i = 0; i < 10; i++){
    btns[i].pin = PIN_BTNS[i];
    btns[i].mode = INPUT_PULLUP;
    btns[i].isInversed = true;
    pinMode(btns[i].pin, btns[i].mode);
  }
  Serial.begin(57600);
  Serial.println("MEGA PIANO!");
  FastLED.addLeds<WS2812B, 7, GRB>(leds, 60);
  for (uint8_t i = 0; i < 60; i++) {
    leds[i] = 0x222222;
    FastLED.show();
  }
  for (uint8_t i = 0; i < 60; i++) {
    leds[i] = 0;
  }
  FastLED.show();
  MIDI.begin(31250);
  lcd.init();                 
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("Hello, world!");
  lcd.setCursor(1,1);
  lcd.print("Arduino!");
}

void loop() {
  enc1.tick();
  checkButtons();
  if (!btns[0].isProcessed){
    mode++;
    if (mode > 0)
      mode = -3;
    Serial.print("Mode = ");
    Serial.println(mode);
    modeFirstStart = true;
    btns[0].isProcessed = true;
  }

  switch (mode){
    default: mode0(); break;
    case -1: mode_1(); break;
    case -2: modeRed(); break;
    case -3: modeRedLine(); break;
  }
}