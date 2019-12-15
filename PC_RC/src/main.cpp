#include <Arduino.h>
#include "Keyboard.h"
#include <IRremote.h>

struct CMD {
  uint32_t codes[];
  int cmd;
}

CMD cmds[11];

const uint32_t CMD_STOP = 0x20DF8D72;
const uint32_t CMD_PLAY = 0x20DF0DF2;
const uint32_t CMD_PAUSE = 0x20DF5DA2;
const uint32_t CMD_REC = 0x20DFBD42;
const uint32_t CMD_PREV = 0x20DFF10E;
const uint32_t CMD_NEXT = 0x20DF718E;

const uint32_t CMD_LEFT = 0x20DFE01F;
const uint32_t CMD_RIGHT = 0x20DF609F;
const uint32_t CMD_UP = 0x20DF02FD;
const uint32_t CMD_DOWN = 0x20DF827D;
const uint32_t CMD_REPEAT = 0xFFFFFFFF;

const uint32_t CMD_RED = 0x20DF4EB1;


#define PIN_LED 9
#define PIN_LED1 13
#define PIN_IR 0

IRrecv irrecv(PIN_IR);
decode_results results;

void setup() {
  // put your setup code here, to run once:
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_LED1, OUTPUT);
  Keyboard.begin();
  irrecv.enableIRIn();
  Serial.begin(57600);
}



void dump(decode_results *results) {
  // Dumps out the decode_results structure.
  // Call this after IRrecv::decode()
  int count = results->rawlen;
  if (results->decode_type == UNKNOWN) {
    Serial.print("Unknown encoding: ");
  }
  else if (results->decode_type == NEC) {
    Serial.print("Decoded NEC: ");

  }
  else if (results->decode_type == SONY) {
    Serial.print("Decoded SONY: ");
  }
  else if (results->decode_type == RC5) {
    Serial.print("Decoded RC5: ");
  }
  else if (results->decode_type == RC6) {
    Serial.print("Decoded RC6: ");
  }
  else if (results->decode_type == PANASONIC) {
    Serial.print("Decoded PANASONIC - Address: ");
    Serial.print(results->address, HEX);
    Serial.print(" Value: ");
  }
  else if (results->decode_type == LG) {
    Serial.print("Decoded LG: ");
  }
  else if (results->decode_type == JVC) {
    Serial.print("Decoded JVC: ");
  }
  else if (results->decode_type == AIWA_RC_T501) {
    Serial.print("Decoded AIWA RC T501: ");
  }
  else if (results->decode_type == WHYNTER) {
    Serial.print("Decoded Whynter: ");
  }
  Serial.print(results->value, HEX);
  Serial.print(" (");
  Serial.print(results->bits, DEC);
  Serial.println(" bits)");
  Serial.print("Raw (");
  Serial.print(count, DEC);
  Serial.print("): ");

  for (int i = 1; i < count; i++) {
    if (i & 1) {
      Serial.print(results->rawbuf[i]*USECPERTICK, DEC);
    }
    else {
      Serial.write('-');
      Serial.print((unsigned long) results->rawbuf[i]*USECPERTICK, DEC);
    }
    Serial.print(" ");
  }
  Serial.println();
}


bool altMode = false;
uint8_t cmd = 0;
uint8_t lastCmd = 0;
uint8_t altPressed = 0;
bool enabled = false;

void loop() {
  // put your main code here, to run repeatedly:
  if (irrecv.decode(&results)) {
    if (results.value == CMD_RED) {
      enabled = !enabled;
      if (enabled) {
        digitalWrite(PIN_LED, 1);
        delay(100);
        digitalWrite(PIN_LED, 0);
        delay(100);
        digitalWrite(PIN_LED, 1);
        delay(100);
        digitalWrite(PIN_LED, 0);
        delay(100);
        digitalWrite(PIN_LED, 1);
        delay(100);
        digitalWrite(PIN_LED, 0);
        delay(100);
      } else {
        digitalWrite(PIN_LED1, 1);
        delay(100);
        digitalWrite(PIN_LED1, 0);
        delay(100);
        digitalWrite(PIN_LED1, 1);
        delay(100);
        digitalWrite(PIN_LED1, 0);
        delay(100);
        digitalWrite(PIN_LED1, 1);
        delay(100);
        digitalWrite(PIN_LED1, 0);
        delay(100);
      }
      delay(300);
    }
    if (!enabled) {
      digitalWrite(PIN_LED1, 1);
      delayMicroseconds(100);
      digitalWrite(PIN_LED1, 0);
      irrecv.resume();
      return;
    }
    digitalWrite(PIN_LED, 1);
    Serial.println(results.value, HEX);
    //dump(&results);
    if (results.value == CMD_REC)
      cmd = 100;
    if (results.value == CMD_PLAY)
      cmd = 101;
    if (results.value == CMD_STOP)
      cmd = 102;
    if (results.value == CMD_REPEAT)
      cmd = lastCmd;
      
    if (results.value == CMD_PAUSE)
      cmd = 1;
    if (results.value == CMD_PREV)
      cmd = 3;
    if (results.value == CMD_NEXT)
      cmd = 4;
    if (results.value == CMD_UP)
      cmd = 5;
    if (results.value == CMD_DOWN)
      cmd = 6;
    if (results.value == CMD_LEFT)
      cmd = 7;
    if (results.value == CMD_RIGHT)
      cmd = 8;;
    
    switch (cmd) {
      case 1: 
        Keyboard.write(' '); 
        delay(300);
        break;
      case 3:
        Keyboard.press(KEY_LEFT_ARROW);
        delay(100);
        Keyboard.release(KEY_LEFT_ARROW);
        break;
      case 4:
        Keyboard.press(KEY_RIGHT_ARROW);
        delay(100);
        Keyboard.release(KEY_RIGHT_ARROW);
        break;
      case 5:
        Keyboard.press(KEY_UP_ARROW);
        delay(100);
        Keyboard.release(KEY_UP_ARROW);
        break;
      case 6:
        Keyboard.press(KEY_DOWN_ARROW);
        delay(100);
        Keyboard.release(KEY_DOWN_ARROW);
        break;
      case 7:
        Keyboard.press(KEY_LEFT_ARROW);
        delay(100);
        Keyboard.release(KEY_LEFT_ARROW);
        break;
      case 8:
        Keyboard.press(KEY_RIGHT_ARROW);
        delay(100);
        Keyboard.release(KEY_RIGHT_ARROW);
        break;
        
      case 100:
        altMode = !altMode;
        digitalWrite(PIN_LED1, altMode);
        if (altMode)
          Keyboard.press(KEY_LEFT_ALT);
        else
          Keyboard.release(KEY_LEFT_ALT);
        delay(500);
        break;
      case 101:
        Keyboard.press(KEY_RETURN);
        delay(100);
        Keyboard.release(KEY_RETURN);
        delay(300);
        break;
      case 102:
        Keyboard.press(KEY_TAB);
        delay(100);
        Keyboard.release(KEY_TAB);
        delay(200);
        break;
    }
    Serial.print("altMode = ");
    Serial.println(altMode);
    Serial.print("cmd = ");
    Serial.println(cmd);
    Serial.print("altPressed = ");
    Serial.println(altPressed);
    lastCmd = cmd;
    cmd = 0;
    digitalWrite(PIN_LED, 0);
    irrecv.resume(); // Receive the next value
  }
}