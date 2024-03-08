#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

const char* ssid = "MyHomeWifi";
const char* password = "impulse24";

int device_id = 1;
int session_id = 0;
String addressBase = "http://turnlive.ru/atpulso/";
String addressAddSession = "addsession.php";
String addressAddPulse = "addpulse.php";
//addsession.php?device_id=
//addpulse.php?session_id= &value=

unsigned long lastTime = 0;
unsigned long timerDelay = 800;

uint32_t lastPress = 0;
int pulseValue = 0;

ADC_MODE(ADC_TOUT);

IRAM_ATTR void pulsed() {
  if (lastPress > 0 && millis() - lastPress > 200) {
    pulseValue = 60000/(millis() - lastPress);
  }
  lastPress = millis();
}

void setup() {
  Serial.begin(115200); 
  //pinMode(A0, INPUT);
  return;
  Serial.println("Starting");
  /*WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
*/
  pinMode(12, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(12), pulsed, FALLING);
}

void addSession() {
    WiFiClient client;
    HTTPClient http;

    String serverPath = addressBase + addressAddSession + "?device_id=" + device_id;
    
    http.begin(client, serverPath.c_str());

    int httpResponseCode = http.GET();
    
    if (httpResponseCode > 0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      String payload = http.getString();
      Serial.println(payload);
      session_id = payload.toInt();
    }
    else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }
    http.end();
}

void addPulse() {
  if (pulseValue == 0) 
    return;

  Serial.println(pulseValue);
  WiFiClient client;
  HTTPClient http;

  String serverPath = addressBase + addressAddPulse + "?session_id=" + session_id + "&value=" + pulseValue;
  
  http.begin(client, serverPath.c_str());

  int httpResponseCode = http.GET();
  
  if (httpResponseCode > 0) {
    String payload = http.getString();
    Serial.println(payload);
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  http.end();
}

void loop() {
  int a = analogRead(A0);
  Serial.println(a);
  delay(50);
  return;
  Serial.println("NOT RETURNED");

  if ((millis() - lastTime) >= timerDelay) {
    if(WiFi.status()== WL_CONNECTED) {
      if (session_id == 0)
        addSession();

      addPulse();
    }
    else {
      Serial.println("WiFi Disconnected");
    }
    lastTime = millis();
  }
}