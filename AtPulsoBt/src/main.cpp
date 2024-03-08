#include <Arduino.h>
#include <SoftwareSerial.h>
#define USE_ARDUINO_INTERRUPTS 1
#include <PulseSensorPlayground.h>

SoftwareSerial bt(2, 3);

const int PulseWire = 0; // A0
const int LED13 = 13; // D13
int Threshold = 520;
PulseSensorPlayground pulseSensor;


void setup() {
  pinMode(2,INPUT);
  pinMode(3,OUTPUT);
  Serial.begin(9600);
  bt.begin(9600);

  delay(1000);
  Serial.println("AtPulsoBt started");
  //Serial.println("AtPulsoBt password: 1271");
  //bt.println("at+name=AtPulsoBt");
  //bt.println("at+pswd=1271");
  
  pulseSensor.analogInput(PulseWire);   
  pulseSensor.blinkOnPulse(LED13); // auto-magically blink Arduino's LED with heartbeat.
  pulseSensor.setThreshold(Threshold);
  if (pulseSensor.begin()) {
    Serial.println("PulseSensor object created");  //This prints one time at Arduino power-up,  or on Arduino reset.  
  }
}

int signal;
uint32_t beatTimeOld = 0;
uint32_t beatTime = 0;
bool signalled = false;
int interval = 60;

void loop() {
  signal = analogRead(PulseWire);
  Serial.print(signal);
  Serial.print(",");
  Serial.println(interval);

  if(signal > Threshold) {                          
    digitalWrite(LED13, HIGH);
    if (!signalled) {
      signalled = true;
      beatTime = millis();
      interval = 60000/((beatTime - beatTimeOld));
      beatTimeOld = beatTime;
      bt.println(interval);
    }
  } 
  else {
    digitalWrite(LED13, LOW);
    signalled = false;                
  }
  delay(10);
}