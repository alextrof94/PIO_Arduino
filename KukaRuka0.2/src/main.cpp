#include <Arduino.h>

#include <main.h>
#include <lcd.h>
#include <servos.h>
#include <buttons.h>
#include <external.h>

uint8_t mode = 0;
uint8_t modeOld = 0;
bool modeFirstStart = true;

String msg = "c00123456789\r\n";

void modeReady() {
    ServosAnim(ANIMREADY);
    delay(1000);
}

void modeWork() {
    ServosHandSetEnable(0);
    ServosAnim(ANIMTOGET);
    ServosHandSetEnable(1);
    ServosAnim(ANIMAFTERGET);
    ServosAnim(ANIMCHECK);
    PORT_PC.print("recognize\r\n");
    while(PORT_PC.available() < 14)
      delay(1);
    msg = PORT_PC.readString();
    ServosAnim(ANIMCHECKED);
    if (msg[1] == '1') {
      // barcode
      PORT_PC.println("barcode");
      if (msg[2] == '1'){
        PORT_PC.println("barcode1");
        ServosAnim(ANIMTOBC1);
      }
      else {
        PORT_PC.println("barcode2");
        ServosAnim(ANIMTOBC2);
      }
    } else if (msg[1] == '2') {
      // qr
      if (msg[2] == 'q')
        ServosAnim(ANIMTOQR1);
      else
        ServosAnim(ANIMTOQR2);
    } else {
      switch(msg[0]) {
        case 'r': ServosAnim(ANIMTORED); break;
        case 'g': ServosAnim(ANIMTOGREEN); break;
        case 'b': ServosAnim(ANIMTOBLUE); break;
        case 'y': ServosAnim(ANIMTOYELLOW); break;
        default: ServosAnim(ANIMREADY); break;
      }
    }
    ServosHandSetEnable(0);
    if (msg[1] == '1') {
      // barcode 
      if (msg[2] == '9')
        ServosAnim(ANIMAFTERBC1);
      else
        ServosAnim(ANIMAFTERBC2);
    } else if (msg[1] == '2') {
      // qr
      if (msg[2] == 'q')
        ServosAnim(ANIMAFTERQR1);
      else
        ServosAnim(ANIMAFTERQR2);
    } else {
      switch(msg[0]) {
        case 'r': ServosAnim(ANIMAFTERRED); break;
        case 'g': ServosAnim(ANIMAFTERGREEN); break;
        case 'b': ServosAnim(ANIMAFTERBLUE); break;
        case 'y': ServosAnim(ANIMAFTERYELLOW); break;
        default: ServosAnim(ANIMREADY); break;
      }
    }
}

void modeManual() {
  
}

void modeHello() {
  if (modeFirstStart) {
    modeFirstStart = false;
    ServosAnim(ANIMHELLOREADY);
  }
  
  ServosAnim(ANIMHELLO);
}

void setup() {
  PORT_PC.begin(57600);
  LcdInit();
  ExternalInit();
  //ColorSensorInit();
  ServosInit();
  ButtonsInit();
    ServosAnim(ANIMREADY);
  pinMode(PIN_LED, OUTPUT);
}

uint8_t nextAnim = ANIMREADY;
void loop() {
  if (mode != modeOld){
    modeFirstStart = true;
    PORT_PC.print("Mode = ");
    PORT_PC.println(mode);
  }
  
  switch (mode) {
    default: modeReady(); break;
    case 1: modeWork(); break;
    case 2: modeManual(); break;
    case 3: modeHello(); break;
  }
}