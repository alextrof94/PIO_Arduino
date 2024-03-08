#include <Arduino.h>
#include <SoftwareSerial.h>
#include <IRremote.hpp>

const int PIN_IR_RECEIVER = 2;
const int PIN_IR_SENDER = 3;
const int PIN_LED = 13;
const int PIN_BT_RX = 4;
const int PIN_BT_TX = 5;

SoftwareSerial bt(PIN_BT_RX, PIN_BT_TX);



void setup() {
  pinMode(PIN_BT_RX, INPUT);
  pinMode(PIN_BT_TX, OUTPUT);
  Serial.begin(9600);
  bt.begin(9600);

  IrReceiver.begin(PIN_IR_RECEIVER, true, PIN_LED);
  IrSender.begin(PIN_IR_SENDER, true, PIN_LED);

  delay(1000);
  Serial.println("BTPR started");
}

void loop() {
  if (IrReceiver.decode()) {
    if (IrReceiver.decodedIRData.decodedRawData > 0) {
      Serial.println(IrReceiver.decodedIRData.decodedRawData, HEX);
      bt.println(IrReceiver.decodedIRData.decodedRawData, HEX);
    }
    IrReceiver.resume();
  }
  if (bt.available()) {
    String cmd = bt.readString();
    Serial.println(cmd);
    if (cmd == "BTPS\n") {
      bt.println("OK");
      Serial.println("CONNECTED");
    }
    else {
      uint32_t irCmd = strtoul(cmd.c_str(), NULL, 16);
      Serial.println(irCmd, HEX);
      if (irCmd > 0)
        IrSender.sendNECRaw(irCmd, 3);
    }
  }
  delay(10);
}