#include <Arduino.h>

#include <Wire.h>
#include "LiquidCrystal_I2C.h"
LiquidCrystal_I2C lcd(0x27,20,4);

void lcdClear(int mode) {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("KukaRuka v0.2");
  lcd.setCursor(0,1);
  lcd.print("Mode = ");
  char *buf = "123";
  itoa(mode, buf, 10);
  lcd.setCursor(7,1);
  lcd.print(buf);
}

void lcdPrint(char* str, int x = 0, int y = 0) {
  lcd.setCursor(x,y);
  lcd.print(str);
}

void lcdInit() {
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0,0);
  //         12345678901234567890
  lcd.print("KukaRuka v0.2");
}