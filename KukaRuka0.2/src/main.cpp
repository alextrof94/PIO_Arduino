#include <Arduino.h>

#include <main.h>
#include <lcd.h>
#include <servos.h>
#include <buttons.h>
#include <external.h>
#include "Dynamixel_Serial.h" 

extern uint8_t btnsCount;
extern Button btns[];

uint8_t mode = 0;
uint8_t modeOld = 0;
bool modeFirstStart = true;

String msg = "c00123456789\r\n";

void modeReady() {
  servosAnim(ANIMREADY);
  delay(1000);
  if (!btns[0].pressProcessed){
    btns[0].pressProcessed = true;
    if (btns[0].releaseTime - btns[0].pressTime > 1000)
      mode = 2;
    else
      mode = 1;  
  }
}

void modeWork() {
  if (modeFirstStart) {
    modeFirstStart = false;
    servosModeWork();
  }
  
  if (!btns[0].pressProcessed){
    btns[0].pressProcessed = true;
    if (btns[0].releaseTime - btns[0].pressTime > 1000)
      mode = 2;
    else
      mode = 0;  
  }

  servosHandSetEnable(0);
  servosAnim(ANIMTOGET);
  servosHandSetEnable(1);
  servosAnim(ANIMAFTERGET);
  servosAnim(ANIMCHECK);
  PORT_PC.print("recognize\r\n");
  while(PORT_PC.available() < 14)
    delay(1);
  msg = PORT_PC.readString();
  servosAnim(ANIMCHECKED);
  if (msg[1] == '1') {
    // barcode
    PORT_PC.println("barcode");
    if (msg[2] == '1'){
      PORT_PC.println("barcode1");
      servosAnim(ANIMTOBC1);
    }
    else {
      PORT_PC.println("barcode2");
      servosAnim(ANIMTOBC2);
    }
  } else if (msg[1] == '2') {
    // qr
    if (msg[2] == 'q')
      servosAnim(ANIMTOQR1);
    else
      servosAnim(ANIMTOQR2);
  } else {
    switch(msg[0]) {
      case 'r': servosAnim(ANIMTORED); break;
      case 'g': servosAnim(ANIMTOGREEN); break;
      case 'b': servosAnim(ANIMTOBLUE); break;
      case 'y': servosAnim(ANIMTOYELLOW); break;
      default: servosAnim(ANIMREADY); break;
    }
  }
  servosHandSetEnable(0);
  if (msg[1] == '1') {
    // barcode 
    if (msg[2] == '9')
      servosAnim(ANIMAFTERBC1);
    else
      servosAnim(ANIMAFTERBC2);
  } else if (msg[1] == '2') {
    // qr
    if (msg[2] == 'q')
      servosAnim(ANIMAFTERQR1);
    else
      servosAnim(ANIMAFTERQR2);
  } else {
    switch(msg[0]) {
      case 'r': servosAnim(ANIMAFTERRED); break;
      case 'g': servosAnim(ANIMAFTERGREEN); break;
      case 'b': servosAnim(ANIMAFTERBLUE); break;
      case 'y': servosAnim(ANIMAFTERYELLOW); break;
      default: servosAnim(ANIMREADY); break;
    }
  }
}

void modeReadPositions() {
  if (modeFirstStart) {
    modeFirstStart = false;
    servosModeOff();
  }
  
  if (!btns[0].pressProcessed){
    btns[0].pressProcessed = true;
    if (btns[0].releaseTime - btns[0].pressTime > 1000)
      mode = 0;
    else {
      PORT_PC.print("{");
      for (int i = 0; i < HARDWARE_SERVOS_COUNT; i++){
        int buf = Dynamixel.readPosition(i + 1);
        PORT_PC.print(buf);
        if (i < HARDWARE_SERVOS_COUNT - 1)
          PORT_PC.print(", ");
      }
      PORT_PC.println("},");
    }
  }
}

void modeHello() {
  if (modeFirstStart) {
    modeFirstStart = false;
    servosAnim(ANIMHELLOREADY);
  }
  
  servosAnim(ANIMHELLO);
}

void setup() {
  PORT_PC.begin(57600);
  lcdInit();
  externalInit();
  //ColorSensorInit();
  servosInit();
  buttonsInit();
    servosAnim(ANIMINIT);
  pinMode(PIN_LED, OUTPUT);
  analogWrite(PIN_LED, 255);
}

uint8_t nextAnim = ANIMREADY;
void loop() {
  if (mode != modeOld){
    modeFirstStart = true;
    PORT_PC.print("Mode = ");
    PORT_PC.println(mode);
    modeOld = mode;
  }
  
  switch (mode) {
    case 0: modeReady(); break;
    case 1: modeWork(); break;
    case 2: modeReadPositions(); break;
    case 3: modeHello(); break;
    default: modeReady(); break;
  }
}