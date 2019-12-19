#include <Arduino.h>
#include <main.h>
#include <servos.h>
#include <buttons.h>
#include "Dynamixel_Serial.h" 

const uint8_t btnsCount = 1;
Button btns[btnsCount];
const uint8_t btnsHCCount = 8;
Button btnsHC[btnsHCCount];

void read74HC165(){
  digitalWrite(PIN_74HC165_INH, 1);
  digitalWrite(PIN_74HC165_LOAD, 0);
  delayMicroseconds(SETTINGS_74HC165_DELAY);
  digitalWrite(PIN_74HC165_LOAD, 1);
  digitalWrite(PIN_74HC165_INH, 0);

  for(uint8_t i = 0; i < btnsHCCount; i++){
      btnsHC[i].isPressed = digitalRead(PIN_74HC165_DATA);
      /*PORT_PC.print(i);
      PORT_PC.print(" ");
      PORT_PC.println(btnsHC[i].isPressed);*/
      digitalWrite(PIN_74HC165_CLOCK, 1);
      delayMicroseconds(SETTINGS_74HC165_DELAY);
      digitalWrite(PIN_74HC165_CLOCK, 0);
      if (btnsHC[i].isPressed && !btnsHC[i].isPressedOld) {
        btnsHC[i].pressTime = millis();
        btnsHC[i].releaseProcessed = false;
      } else
      if (!btnsHC[i].isPressed && btnsHC[i].isPressedOld) {
        btnsHC[i].releaseTime = millis();
        btnsHC[i].pressProcessed = false;
      }
      btnsHC[i].isPressedOld = btnsHC[i].isPressed;
  }
  //PORT_PC.println();
}

// Interrupt by timer 
SIGNAL(TIMER4_COMPA_vect) {
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
  read74HC165();
}

void buttonsInit() { 
  pinMode(PIN_74HC165_INH, OUTPUT);
  pinMode(PIN_74HC165_CLOCK, OUTPUT);
  pinMode(PIN_74HC165_LOAD, OUTPUT);
  pinMode(PIN_74HC165_DATA, INPUT);
  btns[0].pin = A15;
  btns[0].inversed = true;
  btns[0].pullup = true;
  for (uint8_t i = 0; i < btnsCount; i++)
    pinMode(btns[i].pin, (btns[i].pullup) ? INPUT_PULLUP : INPUT);
  // set interrupt by timer every 1ms
  OCR4A = 0xAF;
  TIMSK4 |= _BV(OCIE4A);
}