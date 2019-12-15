#include <Arduino.h>

#include <Wire.h>
#include "LiquidCrystal_I2C.h"
LiquidCrystal_I2C lcd(0x3F,20,4);

void lcdPrint(char* str) {
  lcd.print(str);
}

void lcdInit() {
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0,0);
  //         12345678901234567890
  lcd.print("MegaHand by Chkalov!");
}