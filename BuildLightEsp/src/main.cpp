#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <WiFiClientSecureBearSSL.h>
#include <ESP8266HTTPClient.h>
#include <UniversalTelegramBot.h>  
#include <WiFiClientSecure.h>
#include "Strings.h"

#define DEBUG

#define PIN_LED 2
#define PIN_BTN 0
bool buttonPressed = false;

#define SERIAL_SPEED 115200 // by relay-IC

#define DEFAULT_AP_SSID "BuildLight"
#define DEFAULT_AP_PASS "BuildLight"
String hostSsid = DEFAULT_AP_SSID;
String hostPass = DEFAULT_AP_PASS;

int8_t buildLightState = 0; // 0 - off, 1 - red, 2 - green, 3 - both
int8_t buildLightStateOld = 0;
int8_t buildLightId = 0;
#define EEPROM_BUILD_LIGHT_ID_1 1


uint8_t serverIp[4] = {192, 168, 4, 1};
#define EEPROM_SERVER_IP_4 12
uint8_t serverSubnet[4] = {255, 255, 255, 0};
#define EEPROM_SERVER_SUBNET_4 16

char endpointAddress[80] = {'1', '9', '2', '.', '1', '6', '8', '.', '1', '.', '2', '/', 'e', 'n', 'd', '.', 'p', 'h', 'p', '\0'};
#define EEPROM_ENDPOINT_ADDRESS_80 20
char connectToSsid[20] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '\0'};
#define EEPROM_CONNECT_TO_SSID_20 100
char connectToPass[20] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '\0'};
#define EEPROM_CONNECT_TO_PASS_20 120
char hotspotPass[20] = {'B', 'u', 'i', 'l', 'd', 'L', 'i', 'g', 'h', 't', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'};
#define EEPROM_HOTSPOT_PASS_20 140


#define MODE_START 0
#define MODE_IDLE 1
#define MODE_RESET_TO_DEFAULT 10
uint8_t mode = MODE_START;
bool modeChanged = true;

#define BUILD_LIGHT_MODE_NORMAL 0
#define BUILD_LIGHT_MODE_BLINK 1
uint8_t buildLightMode = BUILD_LIGHT_MODE_NORMAL;
uint32_t buildLightModeDelay = 1000;
uint32_t buildLightModeTimer = 0;
uint8_t buildLightWork_buildLightStateBuf;

ESP8266WebServer server(80);
bool wifiConnected = false;

uint8_t relayData[4] = {0xA0, 0, 0, 0xA0}; // 0 - 0xA0 (static), 1 - number of relay (1-4), 2 - state (0/1), 3 - summ of 0-2 bytes // exmp: 0xA0, 2, 1, 0xA3   


void eepromSave() {
  uint8_t erased = 0;
  EEPROM.get(0, erased);
  if (erased != 127) {
    EEPROM.put(0, (uint8_t)127);
  }
  EEPROM.put(EEPROM_BUILD_LIGHT_ID_1, buildLightId);
  for (uint8_t i = 0; i < 4; i++)
  EEPROM.put(EEPROM_SERVER_IP_4, serverIp);
  for (uint8_t i = 0; i < 4; i++)
  EEPROM.put(EEPROM_SERVER_SUBNET_4, serverSubnet);
  for (uint8_t i = 0; i < 80; i++)
    EEPROM.put(EEPROM_ENDPOINT_ADDRESS_80 + i, endpointAddress[i]);
  for (uint8_t i = 0; i < 20; i++)
    EEPROM.put(EEPROM_CONNECT_TO_SSID_20 + i, connectToSsid[i]);
  for (uint8_t i = 0; i < 20; i++)
    EEPROM.put(EEPROM_CONNECT_TO_PASS_20 + i, connectToPass[i]);
  for (uint8_t i = 0; i < 20; i++)
    EEPROM.put(EEPROM_HOTSPOT_PASS_20 + i, hotspotPass[i]);
  EEPROM.commit();
}

void eepromLoad() {
  // check eeprom data
  uint8_t erased = 0;
  EEPROM.get(0, erased);
  if (erased != 127) {
    eepromSave();
  }
  // load
  EEPROM.get(EEPROM_BUILD_LIGHT_ID_1, buildLightId);
  for (uint8_t i = 0; i < 4; i++)
  EEPROM.get(EEPROM_SERVER_IP_4, serverIp);
  for (uint8_t i = 0; i < 4; i++)
  EEPROM.get(EEPROM_SERVER_SUBNET_4, serverSubnet);
  for (uint8_t i = 0; i < 80; i++)
    EEPROM.get(EEPROM_ENDPOINT_ADDRESS_80 + i, endpointAddress[i]);
  for (uint8_t i = 0; i < 20; i++)
    EEPROM.get(EEPROM_CONNECT_TO_SSID_20 + i, connectToSsid[i]);
  for (uint8_t i = 0; i < 20; i++)
    EEPROM.get(EEPROM_CONNECT_TO_PASS_20 + i, connectToPass[i]);
  for (uint8_t i = 0; i < 20; i++)
    EEPROM.get(EEPROM_HOTSPOT_PASS_20 + i, hotspotPass[i]);

  #ifdef DEBUG
    Serial.println(buildLightId);
    Serial.print(serverIp[0], DEC); Serial.print(".");
    Serial.print(serverIp[1], DEC); Serial.print(".");
    Serial.print(serverIp[2], DEC); Serial.print(".");
    Serial.println(serverIp[3], DEC);
    Serial.print(serverSubnet[0], DEC); Serial.print(".");
    Serial.print(serverSubnet[1], DEC); Serial.print(".");
    Serial.print(serverSubnet[2], DEC); Serial.print(".");
    Serial.println(serverSubnet[3], DEC);
    Serial.println(endpointAddress);
    Serial.println(connectToSsid);
    Serial.println(connectToPass);
    Serial.println(hotspotPass);
  #endif
}


void switchRelay(int numberOfRelay, bool enabled) {
  if (numberOfRelay < 1 || numberOfRelay > 4)
    return;
  relayData[1] = numberOfRelay;
  relayData[2] = (enabled) ? 1 : 0;
  relayData[3] = relayData[0] + relayData[1] + relayData[2];
  Serial.write(relayData, 4);
  delay(100);
}

void updateLightState() {
  if (buildLightStateOld == buildLightState) 
    return;
  #ifdef DEBUG
    Serial.print("New state = ");
    Serial.println(buildLightState);
  #endif
  buildLightStateOld = buildLightState;
  switch (buildLightState) {
    case 10: switchRelay(1, 1); break;
    case 11: switchRelay(2, 1); break;
    case 12: switchRelay(3, 1); break;
    case 13: switchRelay(4, 1); break;
    case 1: switchRelay(3, 1); switchRelay(4, 0); break;
    case 2: switchRelay(3, 0); switchRelay(4, 1); break;
    case 3: switchRelay(3, 1); switchRelay(4, 1); break;
    default: switchRelay(1, 0); switchRelay(2, 0); switchRelay(3, 0); switchRelay(4, 0); break;
  }
}


void pageRoot() {
  String message = "<h1>BuildLight</h1><hr/>";
  
  message += "Connection to Wifi:<br/><form action='set_wifi' method='GET'>SSID (max len 20):<input type='text' name='ssid' value='";
  for (uint8_t i = 0; i < 20; i++)
    if (connectToSsid[i] != '\0')
      message += connectToSsid[i];
  message += "' required/><br/>PASSWORD (max len 20):<input type='password' name='password' value='' required/><br/><input type='submit' value='SAVE'></form><hr/>";
  
  message += "Endpoint address:<br/><form action='set_endpoint' method='GET'>Address (max len 80):<input type='text' name='address' value='";
  for (uint8_t i = 0; i < 80; i++)
    if (endpointAddress[i] != '\0')
      message += endpointAddress[i];
  message += "' required/><br/><input type='submit' value='SAVE'></form><hr/>";

  message += "BuildLight ID:<br/><form action='set_id' method='GET'>Id:<input type='number' name='id' value='";
  message += buildLightId;
  message += "' required/><br/><input type='submit' value='SAVE'></form><hr/>";

  message += "HOTSPOT PASS (for this device):<br/><form action='set_hotspot_pass' method='GET'>PASSWORD (max len 20):<input type='password' name='password' value='' required/><br/><input type='submit' value='SAVE'></form><hr/>";

  message += "SERVER ADDRESS:<br/><form action='set_server_address' method='GET'>IP:<input type='number' name='ip0' value='";
  message += serverIp[0];
  message += "' required/><input type='number' name='ip1' value='";
  message += serverIp[1];
  message += "' required/><input type='number' name='ip2' value='";
  message += serverIp[2];
  message += "' required/><input type='number' name='ip3' value='";
  message += serverIp[3];
  message += "' required/><br/>Subnet:<input type='number' name='subnet0' value='";
  message += serverSubnet[0];
  message += "' required/><input type='number' name='subnet1' value='";
  message += serverSubnet[1];
  message += "' required/><input type='number' name='subnet2' value='";
  message += serverSubnet[2];
  message += "' required/><input type='number' name='subnet3' value='";
  message += serverSubnet[3];
  message += "' required/><br/><input type='submit' value='SAVE'></form><hr/>";

  message += "<hr/><form action='restart'><input type='submit' value='RESTART DEVICE'></form><hr/>";

  server.send(200, "text/html", message);
}

void pageRestart() {
  ESP.restart();
}

void pageSetWifi() {
  String message = "";
  for (uint8_t i = 0; i < server.args(); i++) {
    String argName = server.argName(i);
    String arg = server.arg(i);
    if (argName == "ssid" && arg.length() > 0) {
      message += "SSID is setted<br/>";
      for (uint8_t i = 0; i < 20; i++) {
        connectToSsid[i] = arg[i];
      }
    }
    if (argName == "password" && arg.length() > 0) {
      message += "PASSWORD is setted<br/>";
      for (uint8_t i = 0; i < 20; i++) {
        connectToPass[i] = arg[i];
      }
    }
  }
  eepromSave();
  message += httpRedirect;
  server.send(200, "text/html", message);
}

void pageSetEndpointAddress() {
  String message = "";
  for (uint8_t i = 0; i < server.args(); i++) {
    String argName = server.argName(i);
    String arg = server.arg(i);
    if (argName == "address" && arg.length() > 0) {
      message += "Address is setted<br/>";
      for (uint8_t i = 0; i < 80; i++) {
        endpointAddress[i] = arg[i];
      }
    }
  }
  eepromSave();
  message += httpRedirect;
  server.send(200, "text/html", message);
}

void pageSetId() {
  String message = "";
  for (uint8_t i = 0; i < server.args(); i++) {
    String argName = server.argName(i);
    String arg = server.arg(i);
    if (argName == "id" && arg.length() > 0) {
      message += "Id is setted<br/>";
      buildLightId = arg.toInt();
    }
  }
  eepromSave();
  message += httpRedirect;
  server.send(200, "text/html", message);
}

void pageSetHotspotPass() {
  String message = "";
  for (uint8_t i = 0; i < server.args(); i++) {
    String argName = server.argName(i);
    String arg = server.arg(i);
    if (argName == "password" && arg.length() > 0) {
      message += "PASSWORD is setted<br/>";
      for (uint8_t i = 0; i < 20; i++) {
        hotspotPass[i] = arg[i];
      }
    }
  }
  eepromSave();
  message += httpRedirect;
  server.send(200, "text/html", message);
}

void pageSetServerAddress() {
  String message = "";
  for (uint8_t i = 0; i < server.args(); i++) {
    String argName = server.argName(i);
    String arg = server.arg(i);
    if (argName == "ip0" && arg.length() > 0) {
      serverIp[0] = arg.toInt();
    }
    if (argName == "ip1" && arg.length() > 0) {
      serverIp[1] = arg.toInt();
    }
    if (argName == "ip2" && arg.length() > 0) {
      serverIp[2] = arg.toInt();
    }
    if (argName == "ip3" && arg.length() > 0) {
      serverIp[3] = arg.toInt();
    }
    if (argName == "subnet0" && arg.length() > 0) {
      serverSubnet[0] = arg.toInt();
    }
    if (argName == "subnet1" && arg.length() > 0) {
      serverSubnet[1] = arg.toInt();
    }
    if (argName == "subnet2" && arg.length() > 0) {
      serverSubnet[2] = arg.toInt();
    }
    if (argName == "subnet3" && arg.length() > 0) {
      serverSubnet[3] = arg.toInt();
    }
  }
  message += httpRedirect;
  server.send(200, "text/html", message);
}

void pageSetState() {
  String message = "";
  for (uint8_t i = 0; i < server.args(); i++) {
    String argName = server.argName(i);
    String arg = server.arg(i);
    if (argName == "state" && arg.length() > 0) {
      message += "state is setted<br/>";
      buildLightState = arg.toInt();
      buildLightWork_buildLightStateBuf = buildLightState;
      updateLightState();
    }
    if (argName == "mode" && arg.length() > 0) {
      message += "mode is setted<br/>";
      buildLightMode = arg.toInt();
      updateLightState();
    }
    if (argName == "delay" && arg.length() > 0) {
      message += "delay is setted<br/>";
      buildLightDelay = arg.toInt();
      if (buildLightDelay < 1000)
        buildLightDelay = 1000;
      updateLightState();
    }
  }
  message += httpRedirect;
  server.send(200, "text/html", message);
}


bool getLed() {
  return digitalRead(PIN_LED);
}
void setLed(bool state) {
  digitalWrite(PIN_LED, state);
}

void btnWork() {
  buttonPressed = !digitalRead(PIN_BTN);
}

void changeMode(uint8_t newMode) {
  modeChanged = true;
  mode = newMode;
  #ifdef DEBUG
    Serial.print("Mode = ");
    Serial.println(mode);
  #endif
}

void buildLightWork() {
  if (buildLightMode == BUILD_LIGHT_MODE_BLINK) {
    if (millis() >= buildLightModeTimer) {
      buildLightModeTimer = millis() + buildLightModeDelay;
      if (buildLightState != 0)
        buildLightState = 0;
      else 
        buildLightState = buildLightWork_buildLightStateBuf;
      updateLightState();
    }
  }
}

void modeStart() {
  setLed(1);
  eepromLoad();

  WiFi.mode(WIFI_AP_STA);
  // config server
  IPAddress HostIp(serverIp[0], serverIp[1], serverIp[2], serverIp[3]);
  IPAddress HostSubnet(serverSubnet[0], serverSubnet[1], serverSubnet[2], serverSubnet[3]);
  WiFi.config(HostIp, HostIp, HostSubnet);
  hostPass = "";
  for (uint8_t i = 0; i < 20; i++) {
    if (hotspotPass[i] == '\0')
      break;
    hostPass += hotspotPass[i];
  }
  WiFi.softAP(hostSsid, hostPass);
  server.on("/", pageRoot);
  server.on("/set_wifi", pageSetWifi);
  server.on("/set_endpoint", pageSetEndpointAddress);
  server.on("/set_id", pageSetId);
  server.on("/set_hotspot_pass", pageSetHotspotPass);
  server.on("/set_server_address", pageSetServerAddress);
  server.on("/set_state", pageSetState);
  server.on("/restart", pageRestart);
  server.begin(80);

  // config client
  String ssid = "";
  String pass = "";
  for (uint8_t i = 0; i < 20; i++) {
    if (connectToSsid[i] == '\0')
      break;
    ssid += connectToSsid[i];
  }
  for (uint8_t i = 0; i < 20; i++) {
    if (connectToPass[i] == '\0')
      break;
    pass += connectToPass[i];
  }
  WiFi.begin(ssid, pass);

  changeMode(MODE_IDLE);
}

uint32_t modeIdle_LedTimer;
const uint32_t modeIdle_LedDelay = 500;
void modeIdle() {
  server.handleClient();
  wifiConnected = (WiFi.status() == WL_CONNECTED);
  if (wifiConnected) {
    setLed(1);
  } else {
    if (millis() > modeIdle_LedTimer) {
      modeIdle_LedTimer = millis() + modeIdle_LedDelay;
      setLed(!getLed());
    }
  }
  buildLightWork();
}

uint32_t modeResetToDefault_PressTimer;
const uint32_t modeResetToDefault_PressDelay = 3000;
uint32_t modeResetToDefault_LedSwitchTimer;
const uint32_t modeResetToDefault_LedSwitchDelay = 100;
void modeResetToDefault() {
  if (modeChanged) {
    modeChanged = false;
    modeResetToDefault_PressTimer = millis() + modeResetToDefault_PressDelay;
    modeResetToDefault_LedSwitchTimer = millis() + modeResetToDefault_LedSwitchDelay;
    #ifdef DEBUG
      Serial.println("Pressing for default");
    #endif
  }
  if (millis() >= modeResetToDefault_LedSwitchTimer) {
    modeResetToDefault_LedSwitchTimer = millis() + modeResetToDefault_LedSwitchDelay;
    setLed(!getLed());
  }
  if (millis() >= modeResetToDefault_PressTimer) {
    eepromSave();
    #ifdef DEBUG
      Serial.println("RESETTED TO DEFAULT");
    #endif
    changeMode(MODE_START);
    return;
  }
  if (!buttonPressed) {
    #ifdef DEBUG
      Serial.println("Unpressed");
    #endif
    changeMode(MODE_START);
    return;
  }
}

void setup() {
  EEPROM.begin(512);
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_BTN, INPUT_PULLUP);
  Serial.begin(SERIAL_SPEED);
  Serial.println();
  Serial.println("BuildLight");
  delay(2000);
  btnWork();
  if (buttonPressed) {
    changeMode(MODE_RESET_TO_DEFAULT);
    return;
  }
  configTime(0, 0, "pool.ntp.org"); // get UTC time via NTP
  changeMode(MODE_START);
}

void loop() {
  btnWork();
  switch(mode) {
    case MODE_START: modeStart(); break;
    case MODE_IDLE: modeIdle(); break;
    case MODE_RESET_TO_DEFAULT: modeResetToDefault(); break;
  }
}