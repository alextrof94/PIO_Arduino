#include <Arduino.h>

#include <main.h>
#include <lcd.h>
#include <servos.h>
#include <buttons.h>
#include <external.h>
#include "Dynamixel_Serial.h" 

extern uint8_t btnsCount;
extern Button btns[];
extern uint8_t btnsHCCount;
extern Button btnsHC[];

uint8_t mode = 0;
uint8_t modeCount = 4;
uint8_t modeOld = 0;
bool modeFirstStart = true;

String msg = "c00123456789\r\n";


void modeManual() {
  if (modeFirstStart) {
    modeFirstStart = false;
    servosAnim(ANIMREADY);
    lcdClear(mode);
  }
  for (uint8_t i = 0; i < modeCount; i++){
    if (!btnsHC[i].pressProcessed){
      btnsHC[i].pressProcessed = true;
      switch (i){
        case 0: servosAnim(ANIMTORED); servosAnim(ANIMAFTERRED); break;
        case 1: servosAnim(ANIMTOGREEN); servosAnim(ANIMAFTERGREEN); break;
        case 2: servosAnim(ANIMTOBLUE); servosAnim(ANIMAFTERBLUE); break;
        case 3: servosAnim(ANIMTOYELLOW); servosAnim(ANIMAFTERYELLOW); break;
        case 4: servosAnim(ANIMTOQR1); servosAnim(ANIMAFTERQR1); break;
        case 5: servosAnim(ANIMTOQR2); servosAnim(ANIMAFTERQR2); break;
        case 6: servosAnim(ANIMTOBC1); servosAnim(ANIMAFTERBC1); break;
        case 7: servosAnim(ANIMTOBC2); servosAnim(ANIMAFTERBC2); break;
      }
    }
  }
}

void modeReady() {
  if (modeFirstStart) {
    modeFirstStart = false;
    servosAnim(ANIMREADY);
    lcdClear(mode);
  }

  for (uint8_t i = 0; i < modeCount; i++){
    if (!btnsHC[i].pressProcessed){
      btnsHC[i].pressProcessed = true;
      mode = i+1;
    }
  }
}

void modeWork() {
  if (modeFirstStart) {
    modeFirstStart = false;
    servosModeWork();
    lcdClear(mode);
  }

  lcdPrint("              ", 0, 2);
  lcdPrint("Getting", 0, 2);
  servosHandSetEnable(0);
  servosAnim(ANIMTOGET);
  servosHandSetEnable(1);
  servosAnim(ANIMAFTERGET);
  servosAnim(ANIMCHECK);
  lcdPrint("              ", 0, 2);
  lcdPrint("Recognizing", 0, 2);
  delay(500);
  PORT_PC.print("recognize\r\n");
  while(PORT_PC.available() < 14)
    delay(1);
  msg = PORT_PC.readString();
  servosAnim(ANIMCHECKED);
  lcdPrint("              ", 0, 2);
  lcdPrint("Moving", 0, 2);
  if (msg[1] == '1') {
    // barcode
    PORT_PC.println("barcode");
    if (msg[2] == '1'){
      PORT_PC.println("bc1");
      lcdPrint("BC1", 0, 3);
      servosAnim(ANIMTOBC1);
    }
    else {
      PORT_PC.println("bc2");
      lcdPrint("BC2", 0, 3);
      servosAnim(ANIMTOBC2);
    }
  } else if (msg[1] == '2') {
    // qr
    PORT_PC.println("qr");
    if (msg[2] == '1'){
      PORT_PC.println("qr1");
      lcdPrint("QR1", 0, 3);
      servosAnim(ANIMTOQR1);
    }
    else{
      PORT_PC.println("qr2");
      lcdPrint("QR2", 0, 3);
      servosAnim(ANIMTOQR2);
    }
  } else {
    switch(msg[0]) {
      case 'r': 
        lcdPrint("RED", 0, 3);
        servosAnim(ANIMTORED); 
        break;
      case 'g': 
        lcdPrint("GREEN", 0, 3);
        servosAnim(ANIMTOGREEN); 
        break;
      case 'b': 
        lcdPrint("BLUE", 0, 3);
        servosAnim(ANIMTOBLUE); 
        break;
      case 'y': 
        lcdPrint("YELLOW", 0, 3);
        servosAnim(ANIMTOYELLOW); 
        break;
      default: 
        lcdPrint("???", 0, 3);
        servosAnim(ANIMREADY); 
        break;
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
  lcdPrint("         ", 0, 3);
}

int modeReadPositionsBuf = 0;
char * modeReadPositionsCharBuf = "123";
void modeReadPositions() {
  if (modeFirstStart) {
    modeFirstStart = false;
    servosModeOff();
    lcdClear(mode);
  }

  if (!btnsHC[1].pressProcessed){
    btnsHC[1].pressProcessed = true;
    lcdClear(mode);
    if (btnsHC[1].releaseTime - btnsHC[1].pressTime > 10){
      PORT_PC.print("{");
      for (int i = 0; i < HARDWARE_SERVOS_COUNT; i++){
        modeReadPositionsBuf = Dynamixel.readPosition(i + 1);
        PORT_PC.print(modeReadPositionsBuf);
        if (i < HARDWARE_SERVOS_COUNT - 1)
          PORT_PC.print(", ");
        if (i < 3){
          itoa(modeReadPositionsBuf, modeReadPositionsCharBuf, 10);
          lcdPrint(modeReadPositionsCharBuf, i * 5, 2);
        } else {
          itoa(modeReadPositionsBuf, modeReadPositionsCharBuf, 10);
          lcdPrint(modeReadPositionsCharBuf, (i - 3) * 5, 3);
        }
      }
      PORT_PC.println("},");
    }
  }
}

void modeHello() {
  if (modeFirstStart) {
    modeFirstStart = false;
    servosAnim(ANIMHELLOREADY);
    lcdClear(mode);
  }
  lcdPrint("HELLO!", 0, 2);
  
  servosAnim(ANIMHELLO);
}

void setup() {
  PORT_PC.begin(57600);
  lcdInit();
  externalInit();
  servosInit();
  buttonsInit();
    servosAnim(ANIMINIT);
  pinMode(PIN_LED, OUTPUT);
  analogWrite(PIN_LED, 255);
}

uint8_t nextAnim = ANIMREADY;
void loop() {
  if (!btns[0].pressProcessed){
    btns[0].pressProcessed = true;
    if (btns[0].releaseTime - btns[0].pressTime > 500)
      mode = 0;  
  }

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
    case 3: modeManual(); break;
    case 4: modeHello(); break;
    default: modeReady(); break;
  }
}