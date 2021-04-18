#include <Arduino.h>

#include <Wire.h>
#include "LiquidCrystal_I2C.h"
#include "DHT.h"
#include "Adafruit_Sensor.h"
#include <iarduino_RTC.h>

//#define PIN_WATERFLOW 2 // important 2 or 3
#define PIN_RTC_RST 2
#define PIN_RTC_DAT 3
#define PIN_RTC_CLK 4
#define PIN_RELAY_WATER 5
#define PIN_MOTOR_WINDOW_FORWARD 6
#define PIN_MOTOR_WINDOW_BACKWARD 7
#define PIN_DHT11 8
#define PIN_RELAY_LIGHT 9
#define PIN_RELAY_FAN 11
#define PIN_LCD_CLK A4
#define PIN_LCD_DATA A5


#define WATERFLOW_TICKS_PER_LITR 5880

#define SETTINGS_TEMPERATURE_MAX 25 // -254 - 255
#define SETTINGS_HUMIDITY_MIN 60 // 0 - 100
#define SETTINGS_WATER_WORKING_MINUTES 5 // 1 - 60

LiquidCrystal_I2C lcd(0x3F,16,2);
DHT dht(PIN_DHT11, DHT11);
iarduino_RTC rtc(RTC_DS1302, PIN_RTC_RST, PIN_RTC_CLK, PIN_RTC_DAT);

float humidity = 0;
float temperature = 0;

uint32_t waterflowTicks = 0;
void waterflowProcess() {
  waterflowTicks++;
}

bool windowOldState = false;
void setWindowOpened(bool opened) {
  if (opened == windowOldState)
    return;
  if (opened) {
    digitalWrite(PIN_MOTOR_WINDOW_FORWARD, 1);
    digitalWrite(PIN_MOTOR_WINDOW_BACKWARD, 0);
    delay(1000);
    digitalWrite(PIN_MOTOR_WINDOW_FORWARD, 0);
    digitalWrite(PIN_MOTOR_WINDOW_BACKWARD, 0);
  } else {
    digitalWrite(PIN_MOTOR_WINDOW_FORWARD, 0);
    digitalWrite(PIN_MOTOR_WINDOW_BACKWARD, 1);
    delay(1000);
    digitalWrite(PIN_MOTOR_WINDOW_FORWARD, 0);
    digitalWrite(PIN_MOTOR_WINDOW_BACKWARD, 0);
  }
}

uint32_t dhtCheckTimer = 0;
const uint32_t dhtCheckDelay = 1000;
void dhtWork() {
  if (millis() < dhtCheckTimer)
    return;

  dhtCheckTimer = millis() + dhtCheckDelay;
  humidity = dht.readHumidity();
  temperature = dht.readTemperature();
  if (temperature > SETTINGS_TEMPERATURE_MAX) {// || humidity < SETTINGS_HUMIDITY_MIN) {
    digitalWrite(PIN_RELAY_FAN, 1);
    setWindowOpened(true);
  } else {
    digitalWrite(PIN_RELAY_FAN, 0);
    setWindowOpened(false);
  }
}

void rtcWork() {
  rtc.gettime();
  // light from 04-23
  if (rtc.Hours >= 4 && rtc.Hours < 23) {
    digitalWrite(PIN_RELAY_LIGHT, 1);
  } else {
    digitalWrite(PIN_RELAY_LIGHT, 0);
  }
  // water in 8 and 20 for 5 minutes
  if ((rtc.Hours == 8  && (rtc.minutes >= 0 && rtc.minutes < SETTINGS_WATER_WORKING_MINUTES)) 
   || (rtc.Hours == 20 && (rtc.minutes >= 0 && rtc.minutes < SETTINGS_WATER_WORKING_MINUTES))) {
    digitalWrite(PIN_RELAY_WATER, 1);
  } else {
    digitalWrite(PIN_RELAY_WATER, 0);
  }
}

uint32_t lcdTimer = 0;
const uint32_t lcdDelay = 500;
void lcdWork() {
  if (millis() < lcdTimer)
    return;

  lcdTimer = millis() + lcdDelay;
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(temperature);
  lcd.print("*C");
  lcd.setCursor(0,1);
  lcd.print(humidity);
  lcd.print("%");
  lcd.setCursor(10,1);
  lcd.print(rtc.gettime("H:i"));
  #ifdef PIN_WATERFLOW
    lcd.setCursor(10,0);
    lcd.print(waterflowTicks / WATERFLOW_TICKS_PER_LITR);
    lcd.print("L");
  #endif

  Serial.print("temperature");
  Serial.println(temperature);
  Serial.print("humidity");
  Serial.println(humidity);
  Serial.print("time");
  Serial.println(rtc.gettime("H:i"));
  #ifdef PIN_WATERFLOW
    Serial.print("litres");
    Serial.println(waterflowTicks / WATERFLOW_TICKS_PER_LITR);
  #endif
}

void setup() {
  Serial.begin(9600);
  dht.begin();
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("AutoFarm");
  delay(1000);
  Serial.println("Autofarm");
  rtc.begin();
  rtc.settime(0,18,20,13,4,21,2);// 0  сек, 0 мин, 20 час, 13, апрель, 2021 года, вторник
  #ifdef PIN_WATERFLOW
    attachInterrupt(digitalPinToInterrupt(PIN_WATERFLOW), waterflowProcess, RISING);
  #endif
}

void loop() {
  dhtWork();
  rtcWork();

  
  lcdWork();
}