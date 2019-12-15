#include <Arduino.h>
#include <Joystick.h>
//#include <Keyboard.h>

/*
uint8_t hidReportId = JOYSTICK_DEFAULT_REPORT_ID,
		uint8_t joystickType = JOYSTICK_TYPE_JOYSTICK,
        uint8_t buttonCount = JOYSTICK_DEFAULT_BUTTON_COUNT,
		uint8_t hatSwitchCount = JOYSTICK_DEFAULT_HATSWITCH_COUNT,
		bool includeXAxis = true,
		bool includeYAxis = true,
		bool includeZAxis = true,
		bool includeRxAxis = true,
		bool includeRyAxis = true,
		bool includeRzAxis = true,
		bool includeRudder = true,
		bool includeThrottle = true,
		bool includeAccelerator = true,
		bool includeBrake = true,
		bool includeSteering = true);
*/
Joystick_ Joystick[2] = {
  Joystick_(
    JOYSTICK_DEFAULT_REPORT_ID, 
    JOYSTICK_TYPE_JOYSTICK, 32, 0,
    true, true, false, false, false, false,
    false, false, false, false, false
  ), 
  Joystick_(
    JOYSTICK_DEFAULT_REPORT_ID + 1, 
    JOYSTICK_TYPE_JOYSTICK, 32, 0,
    false, false, false, false, false, false,
    false, false, false, false, false
  )
};

const uint8_t BUTTON_SEND_DELAY = 200;
const uint8_t LED_OFF_DELAY = 2;
const uint8_t SEND_DELAY = 5;
uint32_t sendTimer = 0;

struct Led {
  bool enabled = false;
  uint32_t disableTimer = 0;
  uint8_t pin = 0;
};

struct Button {
  bool pressed = false;
  bool oldState = false;
  uint32_t pressedOnTime = 0;
  uint32_t pressedTime = 0;
  uint32_t changeOnTime = 0; // for change mode
  uint32_t sendTimer = 0; // for change mode
  bool sended = false; // for change mode
  uint8_t sendMode = 0; // 0 - 1:1, 1 - inversed, 2 - on change
  uint8_t joystickIndex = 0; // if need more than 32 buttons - use 1-st joystick.
  uint8_t buttonIndexOn = 0; // 0-31
  uint8_t buttonIndexOff = 0; // 0-31 for on change sendMode
  uint8_t ledAssigned = 0; // must be 0 pin
  uint8_t ledEnableMode = 0; // blinks: 0 - on change, 1 - on rise, 2 - on fall; enable: 3 - 1:1, 4 - inversed
};

const uint8_t ANALOG_COUNT = 2;
const uint8_t OUTS_COUNT = 6;
const uint8_t INS_COUNT = 7;
const uint8_t LEDS_COUNT = 2; // at least 1 on 0pin

Button btns[INS_COUNT][OUTS_COUNT]; // 2-6 + 8-12 + A2+A4 (=13) [btn][group]
Led leds[LEDS_COUNT]; // 13+A5 (=1)
uint16_t analogs[ANALOG_COUNT]; // A0-A1 (=2)

const uint8_t PIN_OUTS[] = {8, 9, 10, 11, A2, A3};
const uint8_t PIN_INS[] =  {2, 1, 4, 5, 6, A4, A5};
const uint8_t PIN_LEDS[] = {0, 12}; // 1-st must be 0 pin
const uint8_t PIN_STICK_A[] = {A0, A1};

uint16_t oldAnalogs[2];
const uint8_t updateOldAnalogsDelay = 10;
const uint8_t analogsDrift = 5;
uint32_t updateOldAnalogsTimer;

void readAnalogs() {
  analogs[0] = analogRead(PIN_STICK_A[0]);
  analogs[1] = analogRead(PIN_STICK_A[1]);

  if (millis() > updateOldAnalogsTimer){
    updateOldAnalogsTimer = millis() + updateOldAnalogsDelay;
    bool needBlink = false;
    for (uint8_t i = 0; i < ANALOG_COUNT; i++)
      if ((oldAnalogs[i] > analogs[i] && oldAnalogs[i] - analogs[i] > analogsDrift) 
          || (oldAnalogs[i] < analogs[i] && analogs[i] - oldAnalogs[i] > analogsDrift))
        needBlink = true;
    for (uint8_t i = 0; i < ANALOG_COUNT; i++)
      oldAnalogs[i] = analogs[i];
    
    if (needBlink){
      leds[0].enabled = 1;
      leds[0].disableTimer = millis() + LED_OFF_DELAY;
    }
  }
}

void readButtons() {
  for (uint8_t y = 0; y < OUTS_COUNT; y++) {
    // disable all groups
    for (uint8_t k = 0; k < OUTS_COUNT; k++)
      digitalWrite(PIN_OUTS[k], 1);
    // enable needed group
    digitalWrite(PIN_OUTS[y], 0);
    for (uint8_t x = 0; x < INS_COUNT; x++) {
      btns[x][y].pressed = !digitalRead(PIN_INS[x]);
    }
  }
  
  for (uint8_t y = 0; y < OUTS_COUNT; y++) {
    for (uint8_t x = 0; x < INS_COUNT; x++) {
      if (btns[x][y].pressed != btns[x][y].oldState) {
        btns[x][y].changeOnTime = millis();
        if (btns[x][y].sendMode == 2) {
          btns[x][y].sended = false;
          btns[x][y].sendTimer = btns[x][y].changeOnTime + BUTTON_SEND_DELAY;
        }
        if (btns[x][y].ledEnableMode == 0) {
          leds[btns[x][y].ledAssigned].enabled = 1;
          leds[btns[x][y].ledAssigned].disableTimer = millis() + LED_OFF_DELAY;
        }
        if (btns[x][y].pressed) {
          btns[x][y].pressedOnTime = millis();
          if (btns[x][y].ledEnableMode == 1) {
            leds[btns[x][y].ledAssigned].enabled = 1;
            leds[btns[x][y].ledAssigned].disableTimer = millis() + LED_OFF_DELAY;
          }
        }
        if (!btns[x][y].pressed) {
          btns[x][y].pressedTime = millis() - btns[x][y].pressedTime;
          if (btns[x][y].ledEnableMode == 2) {
            leds[btns[x][y].ledAssigned].enabled = 1;
            leds[btns[x][y].ledAssigned].disableTimer = millis() + LED_OFF_DELAY;
          }
        }
        btns[x][y].oldState = btns[x][y].pressed;
      }
      if (btns[x][y].ledEnableMode == 3)
        leds[btns[x][y].ledAssigned].enabled = btns[x][y].pressed;
      else if (btns[x][y].ledEnableMode == 4)
        leds[btns[x][y].ledAssigned].enabled = !btns[x][y].pressed;
    }
  }
}


void checkLeds() {
  for (uint8_t i = 0; i < LEDS_COUNT; i++) {
    digitalWrite(leds[i].pin, leds[i].enabled);
    if (millis() > leds[i].disableTimer)
      leds[i].enabled = false;
  }
}

void sendAll() {
  for (uint8_t y = 0; y < OUTS_COUNT; y++) {
    for (uint8_t x = 0; x < INS_COUNT; x++) {
      if (btns[x][y].sendMode == 0)
        Joystick[btns[x][y].joystickIndex].setButton(btns[x][y].buttonIndexOn, btns[x][y].pressed);
      if (btns[x][y].sendMode == 1)
        Joystick[btns[x][y].joystickIndex].setButton(btns[x][y].buttonIndexOn, !btns[x][y].pressed);
      if (btns[x][y].sendMode == 2) {
        if (millis() <= btns[x][y].sendTimer || btns[x][y].sended == false) {
          if (btns[x][y].pressed)
            Joystick[btns[x][y].joystickIndex].setButton(btns[x][y].buttonIndexOn, 1);
          else 
            Joystick[btns[x][y].joystickIndex].setButton(btns[x][y].buttonIndexOff, 1);
          btns[x][y].sended = true;
        } else {
          if (btns[x][y].pressed)
            Joystick[btns[x][y].joystickIndex].setButton(btns[x][y].buttonIndexOn, 0);
          else 
            Joystick[btns[x][y].joystickIndex].setButton(btns[x][y].buttonIndexOff, 0);
        }
      }
      if (btns[x][y].pressed) {
        Serial.print(x);
        Serial.print(y);
      }
    }
  }

  Joystick[0].setYAxis(analogs[0]);
  //Serial.print(analogs[0]); Serial.print(" ");
  Joystick[0].setXAxis(1023 - analogs[1]);
  //Serial.print(analogs[1]); Serial.print(" ");
  Serial.println();
  Joystick[0].sendState();
  Joystick[1].sendState();
}

void setup() {
  Serial.begin(57600);
  Joystick[0].setXAxisRange(0, 1023);
  Joystick[0].setYAxisRange(0, 1023);
  
  for (int i = 0; i < OUTS_COUNT; i++)
    pinMode(PIN_OUTS[i], OUTPUT);
  for (int i = 0; i < INS_COUNT; i++)
    pinMode(PIN_INS[i], INPUT_PULLUP);

  for (int i = 0; i < LEDS_COUNT; i++) {
    pinMode(PIN_LEDS[i], OUTPUT);
    leds[i].pin = PIN_LEDS[i];
  }

  /*
    for normal buttons set:
    buttonIndexOn
    ledAssigned = 0 (13 pin)
    ledEnableMode = 0 (on change)
    sendMode = 0 (1:1)

    for fixed-buttons or switches set:
    buttonIndexOn
    buttonIndexOff - set different if need
    ledAssigned = 0 (13 pin)
    ledEnableMode = 0 (on change)
    sendMode = 2 (on change for FSX) if Sim can work with switches you can set 0/1.
  */
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
  
  for (uint8_t y = 0; y < OUTS_COUNT; y++) {
    for (uint8_t x = 0; x < INS_COUNT; x++) {
      uint8_t d = y*INS_COUNT+x;
      if (d < 32) {
        btns[x][y].buttonIndexOn = d;
        btns[x][y].buttonIndexOff = d;
        btns[x][y].joystickIndex = 0;
      } else {
        btns[x][y].buttonIndexOn = d - 32;
        btns[x][y].buttonIndexOff = d - 32;
        btns[x][y].joystickIndex = 1;
      }
    }
  }
  btns[2][2].ledAssigned = 1;
  btns[2][2].ledEnableMode = 3;
  // switches
  btns[0][2].sendMode = 2;
  btns[6][4].sendMode = 2;
  btns[0][5].sendMode = 2;
  btns[5][5].sendMode = 2;
  btns[6][5].sendMode = 2;

  Joystick[0].begin(false);
  Joystick[1].begin(false);
}

void loop() {
  readButtons();
  readAnalogs();
  
  checkLeds();
  if (millis() > sendTimer) {
    sendTimer = millis() + SEND_DELAY;
    sendAll();
  }
}