#include <Arduino.h>
#include "page.h"
#include <SPI.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
//#include <ESP8266WebServer.h>
#include <Adafruit_ADS1X15.h>

const uint8_t PIN_ADS_SCL = D1;
const uint8_t PIN_ADS_SDA = D2;
const uint8_t PIN_LEDS_PWM = D3;
const uint8_t PIN_LEDS_BRIGHTNESS = A0;

uint32_t ledTimer = 0;
uint32_t ledTimerDelay = 500;

Adafruit_ADS1115 ads;
float adc0, adc1, adc2, adc3;
float k = 0.1;  // коэффициент фильтрации, 0.0-1.0
uint32_t adcTimer = 0;
uint32_t adcTimerDelay = 4;

const char* ssid     = "MyHomeWifi";
const char* password = "impulse24";
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
uint32_t clientId = 0;
uint32_t notifyTimer = 0;
uint32_t notifyTimerDelay = 80;

float expRunningAverage(float newVal, float oldVal) {
  oldVal += (newVal - oldVal) * k;
  return oldVal;
}

void notifyClient() {
  ws.text(clientId, "{\"e0\":" + String((int)adc0) + ",\"e1\":" + String((int)adc1) + ",\"e2\":" + String((int)adc2) + ",\"e3\":" + String((int)adc3) + "}");
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  /*AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    if (strcmp((char*)data, "toggle") == 0) {
      //ledState = !ledState;
      notifyClients();
    }
  }*/
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len) {
    switch (type) {
      case WS_EVT_CONNECT:
        Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
        clientId = client->id();
        break;
      case WS_EVT_DISCONNECT:
        Serial.printf("WebSocket client #%u disconnected\n", client->id());
        break;
      case WS_EVT_DATA:
        //handleWebSocketMessage(arg, data, len);
        //break;
      case WS_EVT_PONG:
      case WS_EVT_ERROR:
        break;
  }
}

void initWebSocket() {
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

String processor(const String& var){
  return String();
}

void onIndex(AsyncWebServerRequest *request) {
  request->send_P(200, "text/html", index_html, processor);
}

void setup() {
  Serial.begin(115200);
  delay(10);
  Serial.println('\n');

  analogWriteFreq(10000);
  analogWriteRange(1024);

  ads.setGain(GAIN_ONE);
  ads.setDataRate(RATE_ADS1115_860SPS);
  if (!ads.begin()) {
    Serial.println("Failed to initialize ADS.");
    while (1);
  }
  
  WiFi.begin(ssid, password);
  Serial.print("Connecting to ");
  Serial.println(ssid);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print('.');
  } 

  Serial.print("Connected! IP: ");
  Serial.println(WiFi.localIP()); 

  initWebSocket();
  
  //server.on("/", onIndex);
  server.begin();
}

void loop() {
  if (millis() >= adcTimer) {
    adcTimer = millis() + adcTimerDelay;
    adc0 = expRunningAverage((float)ads.readADC_SingleEnded(0), adc0);
    adc1 = expRunningAverage((float)ads.readADC_SingleEnded(1), adc1);
    adc2 = expRunningAverage((float)ads.readADC_SingleEnded(2), adc2);
    adc3 = expRunningAverage((float)ads.readADC_SingleEnded(3), adc3);
  }

  if (millis() > notifyTimer) {
    notifyTimer = millis() + notifyTimerDelay;
    notifyClient();
  }

  if (millis() > ledTimer) {
    ledTimer = millis() + ledTimerDelay;
    analogWrite(PIN_LEDS_PWM, analogRead(PIN_LEDS_BRIGHTNESS));
  }

  ws.cleanupClients();
  //server.handleClient();  
}