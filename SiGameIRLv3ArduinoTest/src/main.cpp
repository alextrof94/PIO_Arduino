#include <Arduino.h>
#include <FastLED.h>

const char VERSION = '3';

#define PACKAGE_SIZE 10
// "/VMXXXFXXX": / - const start package, V - version, M - mode number, XXX - delay in seconds, F - falsestart t/f, XXX - delay in seconds

#define NUM_LEDS 9
#define DATA_PIN 10
CRGB leds[NUM_LEDS];

#define MODE_ERROR 0
#define MODE_IDLE 1
#define MODE_TABLE 2
#define MODE_REACTION 3
#define MODE_ANSWERING 4
#define MODE_JUDJING 5

int mode = MODE_JUDJING;
int modeDelay = 10;
bool falsestartEnabled = true;
int falsestartDelay = 2;

uint32_t modeChangedTime = 0;
uint32_t modeNextChangeTimer = 0;


int ledTestStage = 0;
uint32_t ledTestTimer = 0;
int ledTestCount = 0;
void ledTest() {
  if (millis() > ledTestTimer) {
    switch (ledTestStage)
    {
    case 0:
        if (ledTestCount < mode) {
          ledTestStage = 1;
          ledTestTimer = millis() + 200;
          digitalWrite(LED_BUILTIN, 1);
        } else {
          ledTestCount = 0;
          ledTestTimer = millis() + 2000;
          digitalWrite(LED_BUILTIN, 0);
        }
      break;
    case 1:
        if (ledTestCount < mode) {
          ledTestStage = 0;
          ledTestTimer = millis() + 200;
          digitalWrite(LED_BUILTIN, 0);
          ledTestCount++;
        }
      break; 
    default:
      ledTestStage = 0;
      break;
    }
  }
}

void modeError() {
  ledTest();
}

void modeIdle() {
  ledTest();
}

void modeTable() {
  ledTest();
}

void modeReaction() {
  ledTest();

  int ledsCount = (float)(millis() - modeChangedTime)/(float)(modeNextChangeTimer-modeChangedTime) * (float)NUM_LEDS;
  if (ledsCount <= NUM_LEDS) {
    for (int i = 0; i < NUM_LEDS; i++)
      leds[i] = 0;
    for (int i = 0; i  < NUM_LEDS - ledsCount; i++)
      leds[i] = CRGB(0, 0, 10);
    FastLED.show();
  }

  if (!digitalRead(4)) {
    Serial.println("/p0");
    delay(1000);
  } else
  if (!digitalRead(3)) {
    Serial.println("/p1");
    delay(1000);
  }
}

void modeAnswering() {
  ledTest();
}

void modeJudjing() {
  ledTest();
}

void modeChanged() {
  digitalWrite(LED_BUILTIN, 0);
  ledTestCount = 0;
  ledTestStage = 0;

  modeChangedTime = millis();
  modeNextChangeTimer = modeChangedTime + (modeDelay * 1000);
  switch (mode)
  {
  case MODE_REACTION:
    for (int i = 0; i  < NUM_LEDS; i++)
      leds[i] = CRGB(0, 0, 10);
    FastLED.show();
    break;
  
  default:
    break;
  }
}

void changeMode(int newMode) {
  if (newMode != mode) {
    mode = newMode;
    modeChanged();
  }
}

int decodeDelayFromSerial(char v1, char v2, char v3) {
  int result = 0;
  result += (v1 - '0') * 100;
  result += (v2 - '0') * 10;
  result += (v3 - '0');
  return result;
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(3, INPUT_PULLUP);
  pinMode(4, INPUT_PULLUP);
  pinMode(DATA_PIN, OUTPUT);

  FastLED.addLeds<WS2812, DATA_PIN, GRB>(leds, NUM_LEDS);
  for (int i = 0; i < NUM_LEDS; i++)
    leds[i] = CRGB(10, 10, 10);
  FastLED.show();
  
  Serial.begin(51400);
  delay(1000);
  Serial.println("SiGameIRL v3 ArduinoTEST");
  for (int i = 0; i < NUM_LEDS; i++)
    leds[i] = 0;
  FastLED.show();
}

String str = "";
char terminalChar = '\r';
bool cmdComplete = false;

void loop() {
  if (Serial.available()) {
    str += (char)Serial.read();
  }
  if (str[str.length() - 1] == terminalChar) {
    cmdComplete = true;
    for (int i = 0; i < NUM_LEDS; i++)
      leds[i] = CRGB(0, 10, 0);
    FastLED.show();
    delay(10);
    for (int i = 0; i < NUM_LEDS; i++)
      leds[i] = 0;
    FastLED.show();
  }
  if (cmdComplete) {
    Serial.println(str);
    if (str.length() != PACKAGE_SIZE + 1) {
      Serial.println("/!elength_" + String(str.length()));
      cmdComplete = false;
      str = "";
      return;
    } 
    if (str[0] != '/') {
      Serial.println("/!estart_" + str[0]);
      cmdComplete = false;
      str = "";
      return;
    }
    if (str[1] != VERSION) {
      Serial.println("/!eversion_" + str[1]);
      cmdComplete = false;
      str = "";
      return;
    }

    modeDelay = decodeDelayFromSerial(str[3], str[4], str[5]);

    switch (str[2])
    {
      case 'i': changeMode(MODE_IDLE); break;
      case 't': changeMode(MODE_TABLE); break;
      case 'r': changeMode(MODE_REACTION); break;
      case 'a': changeMode(MODE_ANSWERING); break;
      case 'j': changeMode(MODE_JUDJING); break;
      
      default: 
        Serial.println("/!emode_" + str[2]);
        changeMode(MODE_ERROR);
        cmdComplete = false;
        str = "";
        return;
        break;
    }


    falsestartEnabled = (str[6] == 't');
    falsestartDelay = decodeDelayFromSerial(str[7], str[8], str[9]);
    str = "";
    cmdComplete = false;
  }

  switch (mode)
  {
    case MODE_IDLE: modeIdle(); break;
    case MODE_TABLE: modeTable(); break;
    case MODE_REACTION: modeReaction(); break;
    case MODE_ANSWERING: modeAnswering(); break;
    case MODE_JUDJING: modeJudjing(); break;
    
    default: modeError(); break;
  }
}