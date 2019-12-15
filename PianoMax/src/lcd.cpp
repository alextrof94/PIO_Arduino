#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x3F,20,2);

uint32_t lcdUpdateTimer = 0;
const uint32_t LCD_UPDATE_DELAY = 100;

void lcdInit() {
	lcd.init();                 
	lcd.backlight();
	lcd.setCursor(0,0);
	lcd.print("Hello, world!");
	lcd.setCursor(1,1);
	lcd.print("Arduino!");
}

void lcdUpdate() {
  if (millis() > lcdUpdateTimer) {
    lcdUpdateTimer = millis() + LCD_UPDATE_DELAY;
    lcd.clear();
    lcd.setCursor(0,0);
    /* 
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
    /**/
  }
}