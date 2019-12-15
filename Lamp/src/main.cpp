#include <Arduino.h>
#include "FastLED.h"
#include "IRremote.h"
#include <EEPROM.h>
#include "LowPower.h"

//CD02F up
//C30CF down
//CB04F left
//C708F right
//C08F7 center

// FFE01F ON
// FF609F OFF
// FFA05F LIGHTER
// FF20DF BLACKER
// FFC837 SMOOTH
// FFC827 FADE 86B0E697
// FFF00F FLASH 35A9425F

struct colorCode {
  uint32_t code1;
  uint32_t code2;
  CRGB color;
};

const colorCode colorCodes[16] = {
  {0xFF50AF, 0x2A89195F, 0x0000FF},
  {0xFF10EF, 0x8C22657B, 0x00FF00},
  {0xFF708F, 0x44C407DB, 0xFF55FF},
  {0xFF906F, 0xE5CFBD7F, 0xFF0000},
  {0xFFB04F, 0xF0C41643, 0xFF4500},
  {0xFFD02F, 0xD538681B, 0xFFFFFF},
  {0xFF9867, 0x97483BFB, 0x999900},
  {0xFF18E7, 0x3D9AE3F7, 0xADD8E6},
  {0xFF30CF, 0x9716BE3F, 0x90EE90},
  {0xFFA857, 0xA3C8EDDB, 0xFFA500},
  {0xFF6897, 0xC101E57B, 0xFF00FF},
  {0xFF8877, 0x9EF4941F, 0xFFFF55},
  {0xFF08F7, 0x45473C1B, 0x5555FF},
  {0xFF48B7, 0xF377C5B7, 0xEE82EE},
  {0xFF58A7, 0xDC0197DB, 0xEE82EE},
  {0xFF28D7, 0x13549BDF, 0x008080}
};

#define PIN_IR 2
IRrecv irrecv(PIN_IR);
decode_results results;

#define NUM_LEDS 10
#define PIN_LEDS 9
#define CURRENT_LIMIT 700

uint8_t brightness = 255;
uint8_t mode = 0;
CRGB selectedColor = 0xFF00FF;

uint32_t changeTimer = 0;
uint32_t changeDelay = 500;
uint8_t hue = 120;

CRGB leds [NUM_LEDS];

uint16_t batteryVal;

void setup() {
  Serial.begin(9600);
  Serial.println("LAMP");
  EEPROM.get(0, batteryVal);
  Serial.println(batteryVal);
  FastLED.addLeds<WS2812B, PIN_LEDS, GRB>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  if (CURRENT_LIMIT > 0) FastLED.setMaxPowerInVoltsAndMilliamps(5, CURRENT_LIMIT);
  FastLED.setBrightness(brightness);
  for (uint8_t i = 0; i < NUM_LEDS; i++) {
    leds[i] = 0xFFFFFF;
    FastLED.show();
    delay(100);
  }
  for (uint8_t i = 0; i < NUM_LEDS; i++) {
    leds[i] = 0x0;
  }
  FastLED.show();

  randomSeed(analogRead(0));

  analogReference(INTERNAL);
  pinMode(15, OUTPUT);
  digitalWrite(15, 0);
  
  pinMode(13, OUTPUT);

  irrecv.enableIRIn(); // Start the receiver
}

// 421 = 5v real
// 3.5v = 294

// 372 = ~4v real

uint32_t batTimer = 0;
uint32_t eepromTimer = 0;
uint32_t eepromDelay = 60000;


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

void wakeUp()
{
    // Just a handler for the pin interrupt.
}

void loop() {
  
  if (mode == 0) {
    if (millis() > changeTimer) {
      changeTimer = millis() + changeDelay;
      hue++;
      for (uint8_t i = 0; i < 10; i++)
        leds[i] = CHSV(hue, 255, 255);
      FastLED.show();
    }
  }

  /*
  if (millis() > batTimer) {
    batTimer = millis() + 100;
    batteryVal = analogRead(A5);
    //Serial.println(batteryVal);
  }

  if (batteryVal > 390) {
    // usb
  } else if (batteryVal > 300) {
    // batery
    if (millis() > changeTimer) {
      changeTimer = millis() + changeDelay;
      for (uint8_t i = 0; i < 10; i++)
        leds[i] = CRGB(map(batteryVal, 390, 80, 0, 255), map(batteryVal, 390, 80, 255, 0), 0);
    }
  } else { 
    // low battery
    if (millis() > eepromTimer) {
      for (uint8_t i = 0; i < 10; i++)
        leds[i] = CRGB(map(batteryVal, 390, 80, 0, 255), map(batteryVal, 390, 80, 255, 0), 0);
      eepromTimer += eepromDelay;
      EEPROM.put(0, batteryVal);
    }
  }
  FastLED.show();*/


  if (irrecv.decode(&results)) {
    digitalWrite(13, 1);
    Serial.println(results.value, HEX);
    //dump(&results);

  
    if (results.value == 0xFF609F) {
      for (uint8_t i = 0; i < 10; i++)
        leds[i] = 0;
      FastLED.show();
      delay(500);
      attachInterrupt(0, wakeUp, LOW);
      LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF); 
      detachInterrupt(0); 
    }

    if (results.value == 0xFFA05F) {
      if (brightness < 246)
        brightness += 10;
      FastLED.setBrightness(brightness);
      Serial.println("brightness ");
      Serial.println(brightness);
        FastLED.show();
    }
    if (results.value == 0xFF20DF) {
      if (brightness > 14)
        brightness -= 10;
      FastLED.setBrightness(brightness);
      Serial.println("brightness ");
      Serial.println(brightness);
        FastLED.show();
    }
    for (uint8_t k = 0; k < 16; k++) {
      if (results.value == colorCodes[k].code1 || results.value == colorCodes[k].code2) {
        mode = 1;
        selectedColor = colorCodes[k].color;
        for (uint8_t i = 0; i < 10; i++)
          leds[i] = selectedColor;
        FastLED.show();
      }
    }
    
    if (results.value == 0xFFC837) {
      mode = 0;
    }
    if (results.value == 0xFFF00F || results.value == 0x35A9425F) {
      mode = 0;
      changeDelay /= 2;
      if (changeDelay < 25)
        changeDelay = 25;
      Serial.println("changeDelay ");
      Serial.println(changeDelay);
    }
    if (results.value == 0xFFC827 || results.value == 0x86B0E697) {
      mode = 0;
      if (changeDelay < 20000)
        changeDelay *= 2; 
      Serial.println("changeDelay ");
      Serial.println(changeDelay);
    }
    
    digitalWrite(13, 0);
    irrecv.resume(); // принимаем следующую команду
  }
}