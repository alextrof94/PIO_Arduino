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
Joystick_ Joystick = 
  Joystick_(
    JOYSTICK_DEFAULT_REPORT_ID + 10, 
    JOYSTICK_TYPE_JOYSTICK, 16, 0,
    true, true, true, true, true, true,
    true, true, true, true, true
  );

const uint8_t SEND_DELAY = 5;
uint32_t sendTimer = 0;

struct Button {
  bool pressed = false;
  bool oldState = false;
  uint32_t pressedOnTime = 0;
  uint32_t pressedTime = 0;
  uint32_t changeOnTime = 0; // for change mode
  uint32_t sendTimer = 0; // for change mode
  bool sended = false; // for change mode
  uint8_t sendMode = 0; // 0 - 1:1, 1 - inversed
  uint8_t buttonIndexOn = 0; // 0-31
};

const uint8_t ANALOG_COUNT = 6;
const uint8_t OUTS_COUNT = 8;
const uint8_t INS_COUNT = 2;

Button btns[INS_COUNT][OUTS_COUNT]; // 1,2,4,5,6,A4,A5 + 8,9,10,11,A2,A3 (=13) [btn][group]
uint16_t analogs[ANALOG_COUNT]; // A0-A3, A10 (=2)

const uint8_t PIN_OUTS[OUTS_COUNT] =  {2, 3, 4, 5, 6, 7, 8, 15};
const uint8_t PIN_INS[INS_COUNT] = {16, 14};
const uint8_t PIN_STICK_A[ANALOG_COUNT] = {A9, A10, A0, A1, A2, A3}; // A9 = d9, A10 = d10

uint16_t oldAnalogs[ANALOG_COUNT];
const uint8_t updateOldAnalogsDelay = 10;
const uint8_t analogsDrift = 5;
uint32_t updateOldAnalogsTimer;

void readAnalogs() {
  analogs[0] = analogRead(PIN_STICK_A[0]);
  analogs[1] = analogRead(PIN_STICK_A[1]);
  analogs[2] = analogRead(PIN_STICK_A[2]);
  analogs[3] = analogRead(PIN_STICK_A[3]);
  analogs[4] = analogRead(PIN_STICK_A[4]);
  analogs[5] = analogRead(PIN_STICK_A[5]);

  if (millis() > updateOldAnalogsTimer){
    updateOldAnalogsTimer = millis() + updateOldAnalogsDelay;
    for (uint8_t i = 0; i < ANALOG_COUNT; i++)
      oldAnalogs[i] = analogs[i];
  }
}

void readButtons() {
  // disable all groups
  for (uint8_t k = 0; k < OUTS_COUNT; k++)
    digitalWrite(PIN_OUTS[k], 0);
  for (uint8_t y = 0; y < OUTS_COUNT; y++) {
    // enable needed group
    digitalWrite(PIN_OUTS[y], 1);
    for (uint8_t x = 0; x < INS_COUNT; x++) {
      btns[x][y].pressed = digitalRead(PIN_INS[x]);
    }
    // disable needed group
    digitalWrite(PIN_OUTS[y], 0);
  }
  
  for (uint8_t y = 0; y < OUTS_COUNT; y++) {
    for (uint8_t x = 0; x < INS_COUNT; x++) {
      if (btns[x][y].pressed != btns[x][y].oldState) {
        btns[x][y].changeOnTime = millis();
        if (btns[x][y].pressed) {
          btns[x][y].pressedOnTime = millis();
        }
        if (!btns[x][y].pressed) {
          btns[x][y].pressedTime = millis() - btns[x][y].pressedTime;
        }
        btns[x][y].oldState = btns[x][y].pressed;
      }
    }
  }
}

void sendAll() {
  for (uint8_t y = 0; y < OUTS_COUNT; y++) {
    for (uint8_t x = 0; x < INS_COUNT; x++) {
      if (btns[x][y].sendMode == 0)
        Joystick.setButton(btns[x][y].buttonIndexOn, btns[x][y].pressed);
      if (btns[x][y].sendMode == 1)
        Joystick.setButton(btns[x][y].buttonIndexOn, !btns[x][y].pressed);
    }
  }

  Joystick.setXAxis(analogs[0]);
  Joystick.setYAxis(analogs[1]);
  Joystick.setZAxis(analogs[2]);
  Joystick.setRxAxis(analogs[3]);
  Joystick.setRyAxis(analogs[4]);
  Joystick.setRzAxis(analogs[5]);

  Joystick.setSteering(analogs[5]);
  Joystick.setAccelerator(analogs[5]);
  Joystick.setBrake(analogs[5]);
  Joystick.setRudder(analogs[5]);
  Joystick.setThrottle(analogs[5]);
  Joystick.sendState();
}

void setup() {
  Serial.begin(57600);
  Joystick.setXAxisRange(0, 1023);
  Joystick.setYAxisRange(0, 1023);
  Joystick.setZAxisRange(0, 1023);
  Joystick.setRxAxisRange(0, 1023);
  Joystick.setRyAxisRange(0, 1023);
  Joystick.setRzAxisRange(0, 1023);
  Joystick.setAcceleratorRange(0, 1023);
  Joystick.setRudderRange(0, 1023);
  Joystick.setSteeringRange(0, 1023);
  Joystick.setThrottleRange(0, 1023);
  Joystick.setSteeringRange(0, 1023);
  
  for (int i = 0; i < OUTS_COUNT; i++)
    pinMode(PIN_OUTS[i], OUTPUT);
  for (int i = 0; i < INS_COUNT; i++)
    pinMode(PIN_INS[i], INPUT);

  for (uint8_t y = 0; y < OUTS_COUNT; y++) {
    for (uint8_t x = 0; x < INS_COUNT; x++) {
      uint8_t d = y+x*OUTS_COUNT;
      if (d < 32) {
        btns[x][y].buttonIndexOn = d;
      }
    }
  }
  Joystick.begin(false);
}

void loop() {
  readButtons();
  readAnalogs();

  if (millis() > sendTimer) {
    sendTimer = millis() + SEND_DELAY;
    sendAll();
  }
}