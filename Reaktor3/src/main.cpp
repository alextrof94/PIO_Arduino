#include <Arduino.h>

#define PIN_ENC_CLK 2
#define PIN_ENC_DT  3 
#define PIN_BTN_ENC  4
#define PIN_DS18B20 5
#define PIN_BTN_1 6
#define PIN_BTN_2 7
#define PIN_RELAY_START 8 // to A1
#define PIN_SDA A4
#define PIN_SCL A5

#include <EEPROM.h>
#define EEPROM_ADDRESS_MODBUS_ID  1
#define EEPROM_ADDRESS_PH_START  100
#define EEPROM_ADDRESS_MODBUS_ID 50

#include "ModbusRtu.h"
uint8_t modbusId = 1;
Modbus slave(modbusId, 0, 0); 

#include "GyverEncoder.h"
Encoder enc1(PIN_ENC_CLK, PIN_ENC_DT, PIN_BTN_ENC);

#include <LiquidCrystal_I2C.h>
#include <Wire.h>
LiquidCrystal_I2C lcd(0x27, 20, 4); 

#include <Adafruit_ADS1015.h>
Adafruit_ADS1115 ads;
float multiplier = 0.1875f;

#include <OneWire.h>
#include <DallasTemperature.h>
OneWire oneWire(PIN_DS18B20);
DallasTemperature ds18b20(&oneWire);
DeviceAddress insideThermometer;

struct SensorsSet {
  uint8_t relay1Pin = 0;
  uint8_t relay2Pin = 0;
  bool relay1 = false;
  bool relay1Old = false;
  bool relay2 = false;
  bool relay2Old = false;
  uint16_t phLow = 700;
  uint16_t phHigh = 700;
  DeviceAddress tempAddress;
  float temp = 0;
};

uint8_t sensorsSetSelected = 0;
const uint8_t sensorsSetCount = 2; // max 4
SensorsSet sensorsSets[sensorsSetCount];

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

uint8_t btnsCount = 3;
Button btns[3];

uint32_t displayUpdateTimer = 0;
uint32_t displayUpdateDelay = 500;
uint32_t tempUpdateTimer = 0;
const int tempUpdateDelay = 1000;
int mode = 0;
bool firstProcessing = true;
bool pressed = false;
uint32_t pressTime = 0;
uint32_t pressTimeNeedForSettings = 500;
const uint16_t phAbsMax = 1400; // 14.00

void checkButtons() {
  for (uint8_t i = 0; i < btnsCount; i++) {
    btns[i].isPressed = digitalRead(btns[i].pin);
    if (btns[i].inversed)
      btns[i].isPressed = !btns[i].isPressed;
      
    if (btns[i].isPressed && !btns[i].isPressedOld) {
      btns[i].pressTime = millis();
      btns[i].releaseProcessed = false;
    } else
    if (!btns[i].isPressed && btns[i].isPressedOld) {
      btns[i].releaseTime = millis();
      btns[i].pressProcessed = false;
    }
    btns[i].isPressedOld = btns[i].isPressed;
  }
}

void displayPh(uint16_t ph) {
  if ((ph / 100) < 10)
    lcd.print(" ");
  lcd.print(ph / 100);
  lcd.print(".");
  if ((ph % 100) < 10)
    lcd.print("0");
  lcd.print(ph % 100);
}

void displayMv(uint16_t mv) {
  if (mv < 10000)
    lcd.print(" ");
  if (mv < 1000)
    lcd.print(" ");
  if (mv < 100)
    lcd.print(" ");
  if (mv < 10)
    lcd.print(" ");
  lcd.print(mv);
}

uint16_t phNow, phOriginal;
float phVoltage;
uint16_t getPh(int port) {
  phOriginal = ads.readADC_SingleEnded(port);
  phVoltage = (float)phOriginal * multiplier;
  phNow = phVoltage;
  return phNow;
}

uint16_t getPhVoltage(int port) {
  phOriginal = ads.readADC_SingleEnded(port);
  phVoltage = (float)phOriginal * multiplier;
  return phVoltage;
}

uint8_t sensorsSetCompleted = 0;
void modeBindingDs18B20(){
  if (millis() > displayUpdateTimer){
    displayUpdateTimer = millis() + displayUpdateDelay;
    if (sensorsSetCompleted == 0) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("DISCONNECT ALL");
      lcd.setCursor(0, 1);
      lcd.print("TERMOMETERS");
      lcd.setCursor(0, 2);
      lcd.print("AND PRESS BUTTON SET");
    } else {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("CONNECT ");
      lcd.print(sensorsSetCompleted);
      lcd.setCursor(0, 1);
      lcd.print("TERMOMETER");
      lcd.setCursor(0, 2);
      lcd.print("AND PRESS BUTTON SET");
    }
  }
  if (!btns[1].pressProcessed) {
    btns[1].pressProcessed = true;
    if (sensorsSetCompleted == 0){
      sensorsSetCompleted++;
    } else {
      bool binded = false;
      // binding
      ds18b20.begin();
      uint8_t devicesCount = ds18b20.getDeviceCount();
      Serial.print("Devices found: ");
      Serial.println(devicesCount);
      bool newDevice = true;
      uint8_t addr[8];
      for (uint8_t i = 0; i < devicesCount; i++) {
        ds18b20.getAddress(addr, i);
        for (uint8_t j = 0; j < sensorsSetCount; j++){
          uint8_t eq = 0;
          for (uint8_t k = 0; k < 8; k++) {
            if (addr[k] == sensorsSets[j].tempAddress[k])
              eq++;
          }
          if (eq == 8)
            newDevice = false;
        }
        if (newDevice) {
          for (uint8_t k = 0; k < 8; k++)
            sensorsSets[sensorsSetCompleted - 1].tempAddress[k] = addr[k];
          binded = true;
          break;
        }
      }
      // check bind
      if (binded){
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("BINDED");
        delay(1000);
        sensorsSetCompleted++;
        if (sensorsSetCompleted == sensorsSetCount + 1) {
          sensorsSetCompleted = 0;
          mode = 2;
        }
      } else {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("DID NOT BINDED");
        lcd.setCursor(0, 1);
        lcd.print("RETRY");
        delay(1000);
      }
    }
  }
}

void loadFromEEPROM(){
  EEPROM.get(EEPROM_ADDRESS_MODBUS_ID, modbusId);
  for (uint8_t i = 0; i < sensorsSetCount; i++) {
    EEPROM.get(EEPROM_ADDRESS_PH_START + 30*i, sensorsSets[i].phLow);
    EEPROM.get(EEPROM_ADDRESS_PH_START + 30*i + 10, sensorsSets[i].phHigh);
    for (uint8_t j = 0; j < 8; j++)
      EEPROM.get(EEPROM_ADDRESS_PH_START + 30*i + 20+ j, sensorsSets[i].tempAddress[j]);
  }
}

void saveToEEPROM(){
  EEPROM.put(EEPROM_ADDRESS_MODBUS_ID, modbusId);
  for (uint8_t i = 0; i < sensorsSetCount; i++) {
    EEPROM.put(EEPROM_ADDRESS_PH_START + 30*i, sensorsSets[i].phLow);
    EEPROM.put(EEPROM_ADDRESS_PH_START + 30*i + 10, sensorsSets[i].phHigh);
    for (uint8_t j = 0; j < 8; j++)
      EEPROM.put(EEPROM_ADDRESS_PH_START + 30*i + 20 + j, sensorsSets[i].tempAddress[j]);
  }  
}

const uint8_t settingsCount = 14;
String settingsMessages[settingsCount] = {
// 01234567890123456789
  " 1 Ph NOW  =        ", 
  " 1 Ph mV   =        ", 
  " 1 Ph Low  =        ", 
  " 1 Ph High =        ", 
  " 2 Ph NOW  =        ", 
  " 2 Ph mV   =        ", 
  " 2 Ph Low  =        ", 
  " 2 Ph High =        ", 
  " Modbus ID =        ", 
  " T1 =               ", 
  " T2 =               ", 
  " Binding DS18B20    ", 
  " LOAD FROM EEPROM   ", 
  " SAVE TO EEPROM     "
};
bool currentSettingSelected = false;
uint8_t settingsSelected = 0;
uint8_t settingsStringOnScreenStart = 0;
uint8_t settingsStringOnScreenEnd = 3;
uint32_t tempUpdateDelaySettings = 5000;
void modeSettings() {
  if (millis() > tempUpdateTimer) {
    tempUpdateTimer = millis() + tempUpdateDelaySettings;
    ds18b20.requestTemperatures();
    for (uint8_t i = 0; i < sensorsSetCount; i++)
      sensorsSets[i].temp = ds18b20.getTempC(sensorsSets[i].tempAddress);
  }
  if (millis() > displayUpdateTimer){
    displayUpdateTimer = millis() + displayUpdateDelay;
    for (uint8_t i = 0; i < 4; i++) {
      lcd.setCursor(0, i);
      lcd.print(settingsMessages[i + settingsStringOnScreenStart]);
      lcd.setCursor(13, i);
      switch (i + settingsStringOnScreenStart) {
        case 0: displayPh(getPh(0)); break;
        case 1: displayMv(getPhVoltage(0)); break;
        case 2: displayPh(sensorsSets[0].phLow); break;
        case 3: displayPh(sensorsSets[0].phHigh); break;
        case 4: displayPh(getPh(1)); break;
        case 5: displayMv(getPhVoltage(1)); break;
        case 6: displayPh(sensorsSets[1].phLow); break;
        case 7: displayPh(sensorsSets[1].phHigh); break;
        case 8: lcd.print(modbusId); break;
        case 9: lcd.print((int)sensorsSets[0].temp); break;
        case 10: lcd.print((int)sensorsSets[1].temp); break;
        case 11: break;
        case 12: break;
        case 13: break;
        default: lcd.print("ERROR"); break;
      }
    }
    lcd.setCursor(0, settingsSelected - settingsStringOnScreenStart);
    lcd.print("[");
    lcd.setCursor(19, settingsSelected - settingsStringOnScreenStart);
    lcd.print("]");
    if (currentSettingSelected) {
      lcd.setCursor(18, settingsSelected - settingsStringOnScreenStart);
      lcd.print("<");
    }
  }

  if (!currentSettingSelected) {
    if (enc1.isRight()) {
      if (settingsSelected < settingsCount - 1)
        settingsSelected++;
      if (settingsSelected > settingsStringOnScreenEnd)
        if (settingsStringOnScreenEnd < settingsCount - 1) {
          settingsStringOnScreenStart++;
          settingsStringOnScreenEnd++;
        }
    }
    if (enc1.isLeft()) {
      if (settingsSelected > 0)
        settingsSelected--;
      if (settingsSelected < settingsStringOnScreenStart)
        if (settingsStringOnScreenStart > 0) {
          settingsStringOnScreenStart--;
          settingsStringOnScreenEnd--;
        }
    }
    if (!btns[1].pressProcessed){
      btns[1].pressProcessed = true;
      switch (settingsSelected) {
        case 12: 
          if (btns[1].pressTime > 1000){
            loadFromEEPROM();
            mode = 1;
          }
          break;
        case 13: 
          if (btns[1].pressTime > 1000){
            saveToEEPROM();
            mode = 1;
          }
          break;
        case 11: 
          for (uint8_t i = 0; i < sensorsSetCount; i++)
            for (uint8_t j = 0; j < 8; j++)
              sensorsSets[i].tempAddress[j] = 0;
          mode = 3; 
          break;
        case 2: case 3: case 6: case 7: case 8: currentSettingSelected = true; break;
        default: break;
      }
    }
  } else {
    // do with current
    if (enc1.isFastR()) {
      switch (settingsSelected){
        case 2:  
          if (sensorsSets[0].phLow < phAbsMax - 10) 
            sensorsSets[0].phLow += 10;
          break;
        case 3: 
          if (sensorsSets[0].phHigh < phAbsMax - 10) 
            sensorsSets[0].phHigh += 10; 
          break;
        case 6: 
          if (sensorsSets[1].phLow < phAbsMax - 10) 
            sensorsSets[1].phLow += 10; 
          break;
        case 7:  
          if (sensorsSets[1].phHigh < phAbsMax - 10) 
            sensorsSets[1].phHigh += 10;
          break;
        case 8:  
          if (modbusId < 246) 
            modbusId += 10;
          break;
      }
    } else
    if (enc1.isRight()) {
      switch (settingsSelected){
        case 2: 
          if (sensorsSets[0].phLow < phAbsMax ) 
            sensorsSets[0].phLow++; 
          break;
        case 3: 
          if (sensorsSets[0].phHigh < phAbsMax) 
            sensorsSets[0].phHigh++; 
          break;
        case 6: 
          if (sensorsSets[1].phLow < phAbsMax) 
            sensorsSets[1].phLow++; 
          break;
        case 7: 
          if (sensorsSets[1].phHigh < phAbsMax) 
            sensorsSets[1].phHigh++; 
          break;
        case 8:  
          if (modbusId < 254) 
            modbusId++;
          break;
      }
    }
    if (enc1.isFastL()) {
      switch (settingsSelected){
        case 2: 
          if (sensorsSets[0].phLow > 10) 
            sensorsSets[0].phLow -= 10;
          break;
        case 3:
          if (sensorsSets[0].phHigh > 10) 
            sensorsSets[0].phHigh -= 10;
          break;
        case 6: 
          if (sensorsSets[1].phLow > 10) 
            sensorsSets[1].phLow -= 10; 
          break;
        case 7: 
          if (sensorsSets[1].phHigh > 10) 
            sensorsSets[1].phHigh -= 10; 
          break;
        case 8: 
          if (modbusId > 10) 
            modbusId -= 10; 
          break;
      }
    } else
    if (enc1.isLeft()) {
      switch (settingsSelected){
        case 2: 
          if (sensorsSets[0].phLow > 0) 
            sensorsSets[0].phLow--;
          break;
        case 3: 
          if (sensorsSets[0].phHigh > 0) 
            sensorsSets[0].phHigh--; 
          break;
        case 6: 
          if (sensorsSets[1].phLow > 0) 
            sensorsSets[1].phLow--; 
          break;
        case 7: 
          if (sensorsSets[1].phHigh > 0) 
            sensorsSets[1].phHigh--; 
          break;
        case 8: 
          if (modbusId > 1) 
            modbusId--; 
          break;
      }
    }
    if (!btns[1].pressProcessed){
      btns[1].pressProcessed = true;
      currentSettingSelected = false;
    }
  }
}

void modeLoading() {
  lcd.clear();
  /* FIRST START */
  uint8_t buf = 0;
  EEPROM.get(0, buf);
  if (buf != 127){
    lcd.setCursor(0,0);
    lcd.print("FIRST START");
    lcd.setCursor(0,1);
    lcd.print("GO TO SETTINGS");
    delay(2000);   
    mode = 2;
    return;
  }
  
  /* FULL RESET */
  if (enc1.isHold()){
    mode = -1;
    return;
  }

  /* WORK MODE */
  loadFromEEPROM();
  lcd.setCursor(0, 0);
  lcd.print("WORK MODE");
  delay(2000);   
  mode = 1;
  
}

// 01234567890123456789
// Ph   = 12.34  R1 = 0
// Temp = 12 *C  R2 = 0
// PhL=12.34  PhH=12.34
// SCREEN 1
void modeWork() {
  if (firstProcessing){
    lcd.clear();
    firstProcessing = false;
    lcd.setCursor(0, 0);
    lcd.print("Ph   =");
    lcd.setCursor(14, 0);
    lcd.print("R1 =");

    lcd.setCursor(0, 1);
    lcd.print("Temp =");
    lcd.setCursor(14, 1);
    lcd.print("R2 =");

    lcd.setCursor(0, 2);
    lcd.print("PhL=");
    lcd.setCursor(11, 2);
    lcd.print("PhH=");

    lcd.setCursor(0, 3);
    lcd.print("SCREEN");
  }
  
  /* TEMP */
  if (millis() > tempUpdateTimer) {
    tempUpdateTimer = millis() + tempUpdateDelay;
    ds18b20.requestTemperatures();
    sensorsSets[sensorsSetSelected].temp = ds18b20.getTempC(sensorsSets[sensorsSetSelected].tempAddress);
  }

  if (phNow < sensorsSets[sensorsSetSelected].phLow){
    sensorsSets[sensorsSetSelected].relay1 = true;
    sensorsSets[sensorsSetSelected].relay2 = false;
  } else
  if (phNow > sensorsSets[sensorsSetSelected].phHigh){
    sensorsSets[sensorsSetSelected].relay1 = false;
    sensorsSets[sensorsSetSelected].relay2 = true;
  } else {
    sensorsSets[sensorsSetSelected].relay1 = false;
    sensorsSets[sensorsSetSelected].relay2 = false;    
  }

  digitalWrite(sensorsSetSelected * 2 + PIN_RELAY_START, sensorsSets[sensorsSetSelected].relay1);
  digitalWrite(sensorsSetSelected * 2 + PIN_RELAY_START + 1, sensorsSets[sensorsSetSelected].relay2);

  if (millis() > displayUpdateTimer){
    displayUpdateTimer = millis() + displayUpdateDelay;
    lcd.setCursor(7, 0);
    displayPh(getPh(sensorsSetSelected));
    lcd.setCursor(19, 0); 
    lcd.print(sensorsSets[sensorsSetSelected].relay1);
    lcd.setCursor(7, 1);
    lcd.print((int)sensorsSets[sensorsSetSelected].temp); lcd.print("*C");
    lcd.setCursor(19, 1); 
    lcd.print(sensorsSets[sensorsSetSelected].relay2);
    lcd.setCursor(4, 2); 
    displayPh(sensorsSets[sensorsSetSelected].phLow);
    lcd.setCursor(15, 2); 
    displayPh(sensorsSets[sensorsSetSelected].phHigh);
    lcd.setCursor(7, 3); 
    lcd.print(sensorsSetSelected+1);
  }
  
  if (!btns[0].pressProcessed) {
    btns[0].pressProcessed = true;
    if (btns[0].pressTime > pressTimeNeedForSettings){
      firstProcessing = true;
      mode = 2;
    }
  }
  if (!btns[1].pressProcessed) {
    btns[1].pressProcessed = true;
    if (sensorsSetSelected > 0)
      sensorsSetSelected--;
  }
  if (!btns[2].pressProcessed) {
    btns[2].pressProcessed = true;
    if (sensorsSetSelected < sensorsSetCount - 1)
      sensorsSetSelected++;
  }
}

void modeUnwork() {
  for (uint8_t i = 0; i < sensorsSetCount; i++){
    sensorsSets[sensorsSetSelected].relay1 = false;
    sensorsSets[sensorsSetSelected].relay2 = false;
    digitalWrite(i * 2 + PIN_RELAY_START, false);
    digitalWrite(i * 2 + PIN_RELAY_START + 1, false);
  }
}

uint8_t isHardReset = false;
void modeHardReset() {
  if (firstProcessing) {
    firstProcessing = false;
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("YOU REALLY WANT");
    lcd.setCursor(0,1);
    lcd.print("HARD RESET?");
    delay(1000);
  }
  
  if (enc1.isRight())
    isHardReset = 1;
  if (enc1.isLeft())
    isHardReset = 0;
    
  lcd.setCursor(0,2);
  lcd.print(isHardReset);
  
  if (!btns[0].pressProcessed) {
    btns[0].pressProcessed = true;
    if (btns[0].pressTime > 1000){
      firstProcessing = true;
      // TODO:
      // reset all eeprom
      EEPROM.put(0, 255);
      mode = 0;
    }
  }
}

void isrCLK() {
  enc1.tick();  // отработка в прерывании
}
void isrDT() {
  enc1.tick();  // отработка в прерывании
}

void setup() {
  pinMode(13, OUTPUT);
  for (uint8_t i = 0; i < 8; i++)
    pinMode(PIN_RELAY_START + i, OUTPUT);
  
  lcd.init();
  lcd.backlight();
  
  Serial.begin(57600);
  
  enc1.setType(TYPE2);
  attachInterrupt(0, isrCLK, CHANGE);    // прерывание на 2 пине! CLK у энка
  attachInterrupt(1, isrDT, CHANGE);    // прерывание на 3 пине! DT у энка
  
  ads.setGain(GAIN_TWOTHIRDS); multiplier = 0.1875f;    // 2/3x gain +/- 6.144V  1 bit = 0.1875mV (default)
//  ads.setGain(GAIN_ONE);       multiplier = 0.125f;     // 1x gain   +/- 4.096V  1 bit = 0.125mV
//  ads.setGain(GAIN_TWO);       multiplier = 0.0625f;    // 2x gain   +/- 2.048V  1 bit = 0.0625mV
//  ads.setGain(GAIN_FOUR);      multiplier = 0.03125f;   // 4x gain   +/- 1.024V  1 bit = 0.03125mV
//  ads.setGain(GAIN_EIGHT);     multiplier = 0.015625f;  // 8x gain   +/- 0.512V  1 bit = 0.015625mV
//  ads.setGain(GAIN_SIXTEEN);   multiplier = 0.0078125f; // 16x gain  +/- 0.256V  1 bit = 0.0078125mV
  ads.begin();

  ds18b20.begin();
  Serial.print("FOUND DS18B20: ");
  Serial.println(ds18b20.getDeviceCount(), DEC);
  if (!ds18b20.getAddress(insideThermometer, 0)) 
    Serial.println("Unable to find address for Device 0"); 
  
  ds18b20.setResolution(insideThermometer, 12);
  
  btns[0].pin = PIN_BTN_ENC;
  btns[0].inversed = false;
  btns[0].pullup = false;
  btns[1].pin = PIN_BTN_1;
  btns[1].inversed = true;
  btns[1].pullup = true;
  btns[2].pin = PIN_BTN_2;
  btns[2].inversed = true;
  btns[2].pullup = true;

  for (uint8_t i = 1; i < 3; i++)
    pinMode(btns[i].pin, (btns[i].pullup) ? INPUT_PULLUP : INPUT);
}

void loop() {
  enc1.tick();
  checkButtons();
  switch(mode) {
    case -1: modeHardReset(); break;
    case 0: modeLoading(); break;
    case 1: modeWork(); break;
    case 2: modeSettings(); break;
    case 3: modeBindingDs18B20(); break;
    default: break;
  }
  if (mode != 1)
    modeUnwork();
}