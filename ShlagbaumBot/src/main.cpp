//#define TWO_BUTTON 1
//#define CUSTOM_RC 1

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <WiFiClientSecureBearSSL.h>
#include <ESP8266HTTPClient.h>
#include <UniversalTelegramBot.h>  
#include <WiFiClientSecure.h>
#include "RTClib.h"
#include "Time.h"
#include "Settings.h"
#include "MyStrings.h"
#ifdef CUSTOM_RC
  #include <RCSwitch.h>
#endif


#define PIN_LED_DOUBLER D0
#define PIN_BTN_DOUBLER D1
#define PIN_BTN_RESET D2
#define PIN_LED_TELEGRAMM D3
#define PIN_LED_POWER D4
#define PIN_LED_WIFI D5
#define PIN_OUT_OPEN D6
#ifdef CUSTOM_RC
  #define PIN_RC_IN D7 // 0-8
  #define PIN_RC_OUT D8 // 0-8
#endif
#ifdef TWO_BUTTON
  #define PIN_OUT_CLOSE 10 // S3
#endif


#ifdef CUSTOM_RC
  RCSwitch mySwitch = RCSwitch();
  char rcCmdOpen[20] = {'0', 'F', 'F', 'F', '0', 'F', 'F', 'F', 'F', 'F', 'F', 'F', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'};
  char rcCmdClose[20] = {'0', 'F', 'F', 'F', '0', 'F', 'F', 'F', 'F', 'F', 'F', 'F', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'};
  String lastRcMessage = "";
#endif

bool wifiConnected = false;
bool telegramStatus = false;

#define DEFAULT_AP_SSID "Shlagbaum"
#define DEFAULT_AP_PASS "Shlagbaum"
String hostSsid = DEFAULT_AP_SSID;
String hostPass = DEFAULT_AP_PASS;

char connectToSsid[20] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
#define EEPROM_CONNECT_TO_SSID_20 100
char connectToPass[20] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
#define EEPROM_CONNECT_TO_PASS_20 120
String botToken = SETTINGS_BOT_TOKEN_DEF;
String logChatId = SETTINGS_LOG_CHAT_ID;
bool botEnabled = false;
int timeZone = 3; // +3 Moskow
#define EEPROM_BOT_TOKEN_60 140


#define MODE_START 0
#define MODE_IDLE 1
#define MODE_CHECK_BOT 2
uint8_t mode = MODE_START;


#define ENTER_MODE_SIMPLE_COMMAND 0
#define ENTER_MODE_USER_DELETE 1
#define ENTER_MODE_USER_ADD 2
#define ENTER_MODE_ADMIN_DELETE 3
#define ENTER_MODE_ADMIN_ADD 4
uint8_t botEnterMode = ENTER_MODE_SIMPLE_COMMAND;

String telegramMessageOffset = SETTINGS_MESSAGE_OFFSET;

#define EEPROM_WIFI_MODE_1 1
#define WIFI_MODE_HOST 0
#define WIFI_MODE_CLIENT 1
uint8_t wifiMode = WIFI_MODE_HOST;

#define GATE_STATE_CLOSED 0
#define GATE_STATE_OPENED 1
uint8_t  gate_state = GATE_STATE_CLOSED;

#define TIME_TO_PRESS_BUTTON 2000

String lastUser = "ЗАПУСК";
X509List cert(TELEGRAM_CERTIFICATE_ROOT);

#define EEPROM_ADMIN_LIST_70 200
const uint8_t adminListLenght = 5;
String adminList[adminListLenght] = {SETTINGS_ADMIN_1,SETTINGS_ADMIN_2,"","",""};

#define EEPROM_WHITE_LIST_742 270
const uint8_t whiteListLenght = 30;
String whiteList[whiteListLenght] = {"","","","","","","","","","","","","","","","","","","","","","","","","","","","","",""};

String botEnterAdmin = "";
String userToModify = "";

WiFiClientSecure client;
UniversalTelegramBot bot(botToken, client);
ESP8266WebServer server(80);
IPAddress HostIp(192,168,1,1);
IPAddress HostSubnet(255,255,255,0);

void changeMode(uint8_t modeNew) {
  mode = modeNew;
  Serial.print("mode = ");
  Serial.println(modeNew);
}

String PrepareString(String source){
    String updatedSource = "";
    for(uint16_t i = 0; i < source.length(); i++) {
      if ((uint8_t)source[i] != 0 && (uint8_t)source[i] != 255 && source[i] != ' ')
       updatedSource += source[i];
    }
    return updatedSource;
}

void eepromWriteWhitelist() {
  String whiteListStr = "";
  for (int i = 0; i < whiteListLenght; i++) {
    whiteListStr += whiteList[i];
  }
  String preparedAdminListStr = PrepareString(whiteListStr);
  for (uint16_t i = 0; i < 742; i++) {
    if(preparedAdminListStr[i] == 255 || preparedAdminListStr[i] == 0)
      EEPROM.put(EEPROM_WHITE_LIST_742 + i, " ");
    else 
      EEPROM.put(EEPROM_WHITE_LIST_742 + i, preparedAdminListStr[i]);
  }
}
void eepromWriteAdminlist() {
  String adminListStr = "";
  for (uint8_t i = 0; i < adminListLenght; i++) {
    adminListStr += adminList[i];
  }
  String preparedAdminListStr = PrepareString(adminListStr);
  Serial.println(adminListStr);
  for (uint8_t i = 0; i < 70; i++) {
    if(preparedAdminListStr[i] == 255 || preparedAdminListStr[i] == 0)
      EEPROM.put(EEPROM_ADMIN_LIST_70 + i, " ");
    else 
      EEPROM.put(EEPROM_ADMIN_LIST_70 + i, preparedAdminListStr[i]);
  }
}
void eepromSave() {
  Serial.println("=== EEPROM SAVE ===");
  EEPROM.put(0, (uint8_t)127);
  Serial.println(wifiMode);
  EEPROM.put(EEPROM_WIFI_MODE_1, wifiMode);

  Serial.println(connectToSsid);
  for (uint8_t i = 0; i < 20; i++)
    EEPROM.put(EEPROM_CONNECT_TO_SSID_20 + i, connectToSsid[i]);

  Serial.println(connectToPass);
  for (uint8_t i = 0; i < 20; i++)
    EEPROM.put(EEPROM_CONNECT_TO_PASS_20 + i, connectToPass[i]);

  Serial.println(botToken);
  for (uint8_t i = 0; i < 60; i++)
    EEPROM.put(EEPROM_BOT_TOKEN_60 + i, botToken[i]);
  eepromWriteWhitelist();
  eepromWriteAdminlist();
  EEPROM.commit();
}

void eepromReadWhitelist() {  
  String whiteListStr = "";
  String word = "";
  for (uint16_t i = 0; i < 742; i++) {
    whiteListStr += " ";
  }
  int counter = 0;
  for (uint16_t i = 0; i < 742; i++) {
    EEPROM.get(EEPROM_WHITE_LIST_742 + i, whiteListStr[i]);
    if (whiteListStr[i] == botMsgUserPrefix[0] && i > 0) {
      whiteList[counter] = PrepareString(word);
      word = "";
      counter++;
    }
    if ((int)whiteListStr[i] != 255 && (int)whiteListStr[i] != 0){
      word += whiteListStr[i];
    }
  }
  if (counter>0){
    whiteList[counter] = PrepareString(word);
  }
  Serial.println(whiteListStr);
}
void eepromReadAdminlist() {  
  String adminListStr = "";
  String word = "";
  for (uint8_t i = 0; i < 70; i++) {
    adminListStr += " ";
  }
  int counter = 0;
  for (uint8_t i = 0; i < 70; i++) {
    EEPROM.get(EEPROM_ADMIN_LIST_70 + i, adminListStr[i]);
    if (adminListStr[i] == botMsgUserPrefix[0] && i > 0){
      adminList[counter] = PrepareString(word);
      word = "";
      counter++;
    }
    if ((int)adminListStr[i]!= 255){
      word += adminListStr[i];
    }
  }
  if (counter>0){
    adminList[counter] = PrepareString(word);
  }
  Serial.println(adminListStr);
}
void eepromLoad() {
  uint8_t erased = 0;
  EEPROM.get(0, erased);
  if (erased != 127) {
    eepromSave();
  }
  Serial.println("=== EEPROM LOAD ===");
  EEPROM.get(EEPROM_WIFI_MODE_1, wifiMode);
  Serial.println(wifiMode);

  for (uint8_t i = 0; i < 20; i++)
    EEPROM.get(EEPROM_CONNECT_TO_SSID_20 + i, connectToSsid[i]);
  Serial.println(connectToSsid);

  for (uint8_t i = 0; i < 20; i++)
    EEPROM.get(EEPROM_CONNECT_TO_PASS_20 + i, connectToPass[i]);
  Serial.println(connectToPass);

  for (uint8_t i = 0; i < 60; i++)
    EEPROM.get(EEPROM_BOT_TOKEN_60 + i, botToken[i]);
  Serial.println(botToken);

  eepromReadWhitelist();
  eepromReadAdminlist();
}

void pageRoot() {
  String message = "<h1>Shlagbaum-bot</h1><hr/>";
  
  message += "Connection to Wifi:<br/><form action='set_ssid' method='GET'>SSID (max len 20):<input type='text' name='ssid' value='";
  for (uint8_t i = 0; i < 20; i++)
    if (connectToSsid[i] != '\0')
      message += connectToSsid[i];
  message += "' required/><br/>PASSWORD (max len 20):<input type='password' name='password' value='' required/><br/><input type='submit' value='SAVE'></form><hr/>";

  message += "TelegramBot token:<br/><form action='set_token' method='GET'>Token (max len 60):<input type='password' name='token' value='' required/><br/><input type='submit' value='SAVE'></form><hr/>";

  message += "Admins:<br/><form action='set_admins' method='GET'>Admins (start UserNames with @, divide with \",\", max len 70 for all)<input type='text' name='admins' value='";
  for (uint8_t j = 0; j < adminListLenght; j++) {
    for (uint8_t i = 0; i < adminList[j].length(); i++) {
      if (adminList[j][i] != '\0') {
        message += adminList[j][i];
      }
    }
    message += ",";
  }
  message += "' required/><br/><input type='submit' value='SAVE'></form><hr/>";

  #ifdef CUSTOM_RC
    message += "RC commands:<br/><form action='set_rc_commands' method='GET'>Open (max len 20, use \"F\" or \"0\"):<input type='text' name='open' value='";
    for (uint8_t i = 0; i < 20; i++)
      if (rcCmdOpen[i] != '\0')
        message += rcCmdOpen[i];
    message += "' required/><br/>Close (max len 20, use \"F\" or \"0\"):<input type='text' name='close' value='";
    for (uint8_t i = 0; i < 20; i++)
      if (rcCmdClose[i] != '\0')
        message += rcCmdClose[i];
    message += "' required/><br/><input type='submit' value='SAVE'></form><hr/>";
    
    message += "Last received message from rc: ";
    message += lastRcMessage;
    message += "<hr/>";
  #endif

  message += "<form action='reload_to_client'><input type='submit' value='RELOAD TO CLIENT'></form><hr/>";
  server.send(200, "text/html", message);
}
void pageSetSsid() {
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
void pageSetToken() {
  String message = "";
  for (uint8_t i = 0; i < server.args(); i++) {
    String argName = server.argName(i);
    String arg = server.arg(i);
    if (argName == "token" && arg.length() > 0) {
      message += "Token is setted<br/>";
      for (uint8_t i = 0; i < 60; i++) {
        botToken[i] = arg[i];
      }
    }
  }
  eepromSave();
  message += httpRedirect;
  server.send(200, "text/html", message);
}
void pageSetAdmins() {
  String message = "";
  for (uint8_t i = 0; i < server.args(); i++) {
    String argName = server.argName(i);
    String arg = server.arg(i);
    message += argName + ": " + arg + "<br/>";
    if (argName == "admins") {
      int commaIndex = arg.indexOf(',');
      if (arg.length() > 0 && arg != ",") {
        message += "You must add 1 admin at least!";
      }
      bool canContinue = true;
      do {
        if (commaIndex == -1) {
          canContinue = false;
          commaIndex = (int)arg.length();
        }
        if (commaIndex > 0) {
          String admin = arg.substring(0, commaIndex);
        }
        arg = arg.substring(commaIndex + 1);
        commaIndex = arg.indexOf(',');
      } while (canContinue);
    }
  }
  eepromSave();
  message += httpRedirect;
  server.send(200, "text/html", message);
}
void pageReloadToClient() {
  String message = "Wifi hotspot disabled. If config is right - youcan use bot after few seconds.";
  server.send(200, "text/html", message);
  server.stop();
  Serial.println("pageReloadToClient");
  #ifdef CUSTOM_RC
    mySwitch.disableReceive();
  #endif
  wifiMode = WIFI_MODE_CLIENT;
  eepromSave();
  changeMode(MODE_START);
  WiFi.softAPdisconnect(true);
}
#ifdef CUSTOM_RC
  void pageSetRcCommands() {
    String message = "";
    for (uint8_t i = 0; i < server.args(); i++) {
      String argName = server.argName(i);
      String arg = server.arg(i);
      if (argName == "open" && arg.length() > 0) {
        message += "Open cmd:<br/>";
        for (uint8_t i = 0; i < 20; i++) {
          message += arg[i];
          rcCmdOpen[i] = arg[i];
        }
        message += "<br/>Close cmd:<br/>";
        for (uint8_t i = 0; i < 20; i++) {
          message += arg[i];
          rcCmdClose[i] = arg[i];
        }
      }
    }
    eepromSave();
    message += httpRedirect;
    server.send(200, "text/html", message);
  }
#endif

uint32_t ledWifiTimer = 0;
void ledWork() {
  analogWrite(PIN_LED_POWER, 511);

  if (wifiMode == WIFI_MODE_HOST) {
    if (millis() > ledWifiTimer) {
      ledWifiTimer = millis() + 100;
      digitalWrite(PIN_LED_WIFI, !digitalRead(PIN_LED_WIFI));
    }
  } else {
    digitalWrite(PIN_LED_WIFI, wifiConnected);
  }

  digitalWrite(PIN_LED_TELEGRAMM, telegramStatus);
}

void buttonWork() {
  if (!digitalRead(PIN_BTN_RESET)) {
    digitalWrite(PIN_LED_POWER, 0);
    digitalWrite(PIN_LED_WIFI, 1);
    digitalWrite(PIN_LED_TELEGRAMM, 1);
    delay(1000);
    if (!digitalRead(PIN_BTN_RESET)) {
      wifiMode = WIFI_MODE_HOST;
      eepromSave();
      changeMode(MODE_START);
      if (WiFi.status() == WL_CONNECTED)
        WiFi.disconnect();
      WiFi.mode(WIFI_OFF);
    }
  }
  if (!digitalRead(PIN_BTN_DOUBLER)) {
    lastUser = "ВРУЧНУЮ";
    #ifdef TWO_BUTTON
      if (gate_state == GATE_STATE_CLOSED) {
        gate_state = GATE_STATE_OPENED;
        digitalWrite(PIN_LED_DOUBLER, 1);
        digitalWrite(PIN_OUT_OPEN, 1);
        delay(1000);
        digitalWrite(PIN_OUT_OPEN, 0);
        digitalWrite(PIN_LED_DOUBLER, 0);
      } else {
        gate_state = GATE_STATE_CLOSED;
        digitalWrite(PIN_LED_DOUBLER, 1);
        digitalWrite(PIN_OUT_CLOSE, 1);
        delay(1000);
        digitalWrite(PIN_OUT_CLOSE, 0);
        digitalWrite(PIN_LED_DOUBLER, 0);
      }
    #else
      if (gate_state == GATE_STATE_CLOSED) {
        gate_state = GATE_STATE_OPENED;
      } else {
        gate_state = GATE_STATE_CLOSED;
      }
      digitalWrite(PIN_LED_DOUBLER, 1);
      digitalWrite(PIN_OUT_OPEN, 1);
      delay(1000);
      digitalWrite(PIN_OUT_OPEN, 0);
      digitalWrite(PIN_LED_DOUBLER, 0);
    #endif
    if (bot.connected) {
      String msgLog = "Нажата кнопка. ";
      if (gate_state == GATE_STATE_CLOSED) {
        msgLog += "Шлагбаум закрыт.";
      } else {
        msgLog += "Шлагбаум открыт.";
      }
      bot.sendMessage(logChatId, msgLog, "");
    }
  }
}


bool validateGateStatus(int _command){
  if (_command == gate_state){
    return false;
  }
  return true;
}

bool validateWhitelist(String userName){  
  Serial.println("Validating whiteList") ;
  for(int i = 0; i < whiteListLenght; i++){
    if (whiteList[i] == botMsgUserPrefix + userName){
      return true;
    }
  }
  return false;
}

bool validateAdminlist(String userName){
  Serial.println("Validating adminList") ;
  for(int i = 0; i < adminListLenght; i++){
    if (adminList[i] == botMsgUserPrefix + userName){
      return true;
    }
  }
  return false;
}
void EditEpromWhiteList(){
    eepromSave();
}
void EditEEptomAdminList(){
  eepromSave();
}

int GetLengthOfFilledEEpromWhiteList(){
    String strList = ""; 
    for(int i = 0; i < whiteListLenght; i++){
        if (whiteList[i] != ""){
          strList += whiteList[i];
      }
    }
    String updatedStrList = PrepareString(strList);
    int result = 742 - updatedStrList.length();
    return result;
}
int GetLengthOfFilledEEpromAdminList(){
    String strList = "";  
    for(int i = 0; i < adminListLenght; i++){
        if (adminList[i] != ""){
          strList += adminList[i];
      }
    }
    String updatedStrList = PrepareString(strList);
    int result = 70 - updatedStrList.length();
    return result;
}
String GetBaseMsg(String userName){
  String msg = "";
  msg += botMsgCommands;
  msg += botCmdOpen + botCmdOpenDescription;
  msg += "\r\n";
  msg += botCmdClose + botCmdCloseDescription;
  msg += "\r\n";
  msg += botCmdEditState + botCmdEditStateDescription;
  if (validateAdminlist(userName)){
    msg += "\r\n";
    msg += botCmdWhitelist + botCmdWhitelistDescription;
    msg+= "\r\n";
    msg += botCmdAdminlist + botCmdAdminlistDescription;
  }
  return msg;
}
void EraseEnterState(){
    userToModify = "";
    botEnterMode = ENTER_MODE_SIMPLE_COMMAND;
    botEnterAdmin = "";
}

void AddUserToWhiteList(String chat_id){
    String msg = botMsgComplete;
    for(int i = 0; i< whiteListLenght; ++i){
    if (whiteList[i] == "") {
        whiteList[i] = userToModify;        
        msg += GetBaseMsg(botEnterAdmin);
        bot.sendMessage(chat_id, msg, "");
        EraseEnterState();
        EditEpromWhiteList();
        return ;
      }
    }
    msg += GetBaseMsg(botEnterAdmin);
    bot.sendMessage(chat_id, msg, "");
    EraseEnterState();
}
void RemoveUserFromWhiteList(String chat_id) {
    String msg = botMsgComplete;
    for(int i = 0; i < whiteListLenght; i++) {
      if (PrepareString(whiteList[i]) == userToModify) {
          whiteList[i] = "";
        msg += GetBaseMsg(botEnterAdmin);
        bot.sendMessage(chat_id, msg, "");
        EraseEnterState();
        EditEpromWhiteList();
        return;
      }
    }
    msg += GetBaseMsg(botEnterAdmin);
    bot.sendMessage(chat_id, msg, "");
    EraseEnterState();
}
void AddUserToAdminList(String chat_id) {
  String msg = botMsgComplete;
    for(int i = 0; i < adminListLenght; i++) {
        if (adminList[i] == "") {
          adminList[i] = userToModify;
           msg = GetBaseMsg(botEnterAdmin);
          bot.sendMessage(chat_id, msg, "");
          EraseEnterState();
          EditEEptomAdminList();
          return ;
      }
    }
    msg += GetBaseMsg(botEnterAdmin);
    bot.sendMessage(chat_id, msg, "");
    EraseEnterState();
}
void RemoveUserFromAdminList(String chat_id) {
    String msg = botMsgComplete;
    for(int i = 0; i < adminListLenght; i++) {
      if (PrepareString(adminList[i]) == userToModify) {
        adminList[i] = "";
        msg += GetBaseMsg(botEnterAdmin);
        bot.sendMessage(chat_id, msg, "");
        EraseEnterState();
        EditEEptomAdminList();
        return;
      }
    }
    msg += GetBaseMsg(botEnterAdmin);
    bot.sendMessage(chat_id, msg, "");
    EraseEnterState();
}

void ProcessUnparsedMessage(String chat_id, String userName) {
      String msg ="";
      msg = "Команда не распознана.\n";
      msg += GetBaseMsg(userName);
      bot.sendMessage(chat_id, msg, "");
      EraseEnterState();
}
void ProcessStart(String chat_id, String from_name, String userName) {
      String msg = "Привет, " + from_name + "!\r\n";
      msg += GetBaseMsg(userName);
      bot.sendMessage(chat_id, msg, "");
      EraseEnterState();
}
void ProcessOpen(String chat_id, String from_name) {
   String msg = "";
   if (validateGateStatus(GATE_STATE_OPENED)) {
    gate_state = GATE_STATE_OPENED;
    lastUser = from_name;
    digitalWrite(PIN_LED_DOUBLER, 1);
    digitalWrite(PIN_OUT_OPEN, 1);
    delay(TIME_TO_PRESS_BUTTON);
    digitalWrite(PIN_OUT_OPEN, 0);
    digitalWrite(PIN_LED_DOUBLER, 0);
    msg = botMsgOpened;
    msg += botCmdClose + botCmdCloseDescription;
    #ifdef CUSTOM_RC   
      mySwitch.sendTriState(rcCmdOpen);
    #endif
  } else {        
    msg = "Шлагбаум уже открыт пользователем " + lastUser + ".\r\n";
    msg += botCmdClose + botCmdCloseDescription;
    msg += "Перед закрытием рекомендуем связаться с тем, кто открыл шлагбаум перед Вами.\r\n";
    msg += "\r\n";
    msg += botCmdEditState + botCmdEditStateDescription;
  }
  bot.sendMessage(chat_id, msg, "");
}
void ProcessClose(String chat_id, String from_name) {
  String msg;
  if (validateGateStatus(GATE_STATE_CLOSED)) {
    gate_state = GATE_STATE_CLOSED;
    lastUser = from_name;
    #ifdef TWO_BUTTON
      digitalWrite(PIN_LED_DOUBLER, 1);
      digitalWrite(PIN_OUT_CLOSE, 1);
      delay(TIME_TO_PRESS_BUTTON);
      digitalWrite(PIN_OUT_CLOSE, 0);
      digitalWrite(PIN_LED_DOUBLER, 0);
    #else
      digitalWrite(PIN_LED_DOUBLER, 1);
      digitalWrite(PIN_OUT_OPEN, 1);
      delay(TIME_TO_PRESS_BUTTON);
      digitalWrite(PIN_OUT_OPEN, 0);
      digitalWrite(PIN_LED_DOUBLER, 0);
    #endif
    msg = botMsgClosed;
    msg += botCmdOpen + botCmdOpenDescription;
    #ifdef CUSTOM_RC   
      mySwitch.sendTriState(rcCmdClose);
    #endif
  } else {        
    msg = "Шлагбаум уже закрыт пользователем " + lastUser + ".\r\n";
    msg += botCmdOpen + botCmdOpenDescription;
    msg += "\r\n";
    msg += botCmdEditState + botCmdEditStateDescription;
  }
  bot.sendMessage(chat_id, msg, "");
}

void ProcessEditState(String chat_id) {
  String msg;
  if (gate_state == GATE_STATE_OPENED)
    msg = "Текущее состояние - шлагбаум открыт пользователем " + lastUser + "\r\n";
  else
    msg = "Текущее состояние - шлагбаум закрыт\r\n";

  msg += botCmdSetOpened + botCmdSetOpenedDescription;
  msg += "\r\n";
  msg += botCmdSetClosed + botCmdSetClosedDescription;
  bot.sendMessage(chat_id, msg, "");
}
void ProcessSetOpened(String chat_id, String from_name) {
    gate_state = GATE_STATE_OPENED;
    lastUser = from_name; 
    String msg = botMsgOpened;
    msg += botCmdClose + botCmdCloseDescription;
    bot.sendMessage(chat_id, msg, "");
}
void ProcessSetClosed(String chat_id, String from_name) {
    gate_state = GATE_STATE_CLOSED;
    lastUser = from_name; 
    String msg = botMsgClosed;
    msg += botCmdOpen + botCmdOpenDescription;
    bot.sendMessage(chat_id, msg, "");
}

void ProcessWhiteList(String chat_id,  String userName) {
  String msg = "Список пользователей бота:\r\n";  
  for(uint8_t i = 0; i < whiteListLenght; i++) {    
    if (whiteList[i] != emptyString) {
      msg += PrepareString(whiteList[i])+ "\r\n";
    }
  }
  msg += "\r\nСвободно " + (String)GetLengthOfFilledEEpromWhiteList() + " символов памяти из 742. Максимальное количество пользователей = " + whiteListLenght + "\r\n";
  msg += botCmdWhitelistAdd + botCmdWhitelistAddDescription;
  msg += botCmdWhitelistRemove + botCmdWhitelistRemoveDescription;
  msg += botCmdWhitelistExit + botCmdWhitelistExitDescription;  

  bot.sendMessage(chat_id, msg, "");
}
void ProcessWhiteListAdd(String chat_id, String userName) {
    botEnterMode = ENTER_MODE_USER_ADD;
    botEnterAdmin = userName;
    String msg = botMsgEnterWhitelistUsername;
    bot.sendMessage(chat_id, msg, "");
}
void ProcessWhiteListRemove(String chat_id, String userName) {
    botEnterMode = ENTER_MODE_USER_DELETE;
    botEnterAdmin = userName;
    String msg = botMsgEnterWhitelistUsername;
    bot.sendMessage(chat_id, msg, "");
}

void ProcessAdminList(String chat_id,  String userName) {  
  String msg = "Список администраторов бота:\r\n";  
  for(uint8_t i = 0; i < adminListLenght; i++) {    
    if (adminList[i] != emptyString) {
      msg += PrepareString(adminList[i]) + "\r\n";
    }
  }
  msg += "\r\nСвободно " + (String)GetLengthOfFilledEEpromAdminList() + " символов памяти из 70. Максимальное количество администраторов = " + adminListLenght + "\r\n";
  msg += botCmdAdminlistAdd + botCmdAdminlistAddDescription;
  msg += botCmdAdminlistRemove + botCmdAdminlistRemoveDescription;
  msg += botCmdAdminlistExit + botCmdAdminlistExitDescription;  

  bot.sendMessage(chat_id, msg, "");
}
void ProcessAdminListAdd(String chat_id, String userName){
    botEnterMode = ENTER_MODE_ADMIN_ADD;
    botEnterAdmin = userName;
    String msg = botMsgEnterAdminlistUsername;
    bot.sendMessage(chat_id, msg, "");
}
void ProcessAdminListRemove(String chat_id, String userName){
    botEnterMode = ENTER_MODE_ADMIN_DELETE;
    botEnterAdmin = userName;
    String msg = botMsgEnterAdminlistUsername;
    bot.sendMessage(chat_id, msg, "");
}

void ProcessTextCommand(String chat_id, String command){
  if (command == "" || ((botEnterMode == ENTER_MODE_USER_ADD || botEnterMode == ENTER_MODE_ADMIN_ADD) && command[0] != botMsgUserPrefix[0])){
    EraseEnterState();
    bot.sendMessage(chat_id, "Некорректное имя пользователя", "");
    return;
  }
  String msg = "";
  if (botEnterMode == ENTER_MODE_USER_ADD)
    msg = "Добавить пользователя " + command + " в писок доступа к шлагбауму?";
  if (botEnterMode == ENTER_MODE_USER_DELETE)
    msg = "Удалить у пользователя " + command + " доступ к шлагбауму?";
  else if (botEnterMode == ENTER_MODE_ADMIN_ADD)
    msg = "Добавить пользователя " + command + " в писок доступа к боту?";
  else if (botEnterMode == ENTER_MODE_ADMIN_DELETE)
    msg = "Удалить у пользователя " + command + " доступ к боту?";

   msg += "\r\n\r\n";
   msg += botCmdYes + botCmdYesDescription;
   msg += "\r\n";
   msg += botCmdNo + botCmdNoDescription;
   bot.sendMessage(chat_id, msg, "");
   userToModify = PrepareString(command);
}
void ProcessYes(String chat_id){
  if (botEnterMode == ENTER_MODE_USER_ADD)
    AddUserToWhiteList(chat_id);
  if (botEnterMode == ENTER_MODE_USER_DELETE)
    RemoveUserFromWhiteList(chat_id);
  else if (botEnterMode == ENTER_MODE_ADMIN_ADD)
    AddUserToAdminList(chat_id);
  else if (botEnterMode == ENTER_MODE_ADMIN_DELETE)
    RemoveUserFromAdminList(chat_id);
}

String timestampToDate(long timestamp) {
  DateTime dt(timestamp);
  String res = "";
  if (dt.hour() < 10)
    res += "0";
  res += String(dt.hour(), DEC);
  res += ":";
  if (dt.minute() < 10)
    res += "0";
  res += String(dt.minute(), DEC);
  res += ":";
  if (dt.second() < 10)
    res += "0";
  res += String(dt.second(), DEC);
  res += " ";
  if (dt.day() < 10)
    res += "0";
  res += String(dt.day(), DEC);
  res += ".";
  if (dt.month() < 10)
    res += "0";
  res += String(dt.month(), DEC);
  res += ".";
  res += String(dt.year(), DEC);
  return res;
}

void handleNewMessages(int numNewMessages) {
  Serial.println("handleNewMessages");
  Serial.println(numNewMessages);

  for (int i = 0; i < numNewMessages; i++) {
    long timestamp = bot.messages[i].date.toInt() + (timeZone * 3600);
    String msgLog = "[" + timestampToDate(timestamp) + "] ";
    msgLog += "@" + bot.messages[i].userName + " (" + bot.messages[i].from_name + "): " + bot.messages[i].text;
    bot.sendMessage(logChatId, msgLog, "");
    String chat_id = String(bot.messages[i].chat_id);
    if (!validateWhitelist(bot.messages[i].userName) && !validateAdminlist(bot.messages[i].userName)){
      bot.sendMessage(chat_id, "У Вас нет доступа к шлагбауму. Обратитесь к администратору.", "");
      return;
    }

    String text = bot.messages[i].text;
    String from_name = bot.messages[i].from_name;
    Serial.print(from_name);
    Serial.print(": ");
    Serial.println(text);

    if (text == botCmdStart || text == botCmdWhitelistExit || text == botCmdAdminlistExit || text == botCmdNo) {
      ProcessStart(chat_id, from_name, bot.messages[i].userName); 
      return;     
    }
    if (text == botCmdOpen) {    
      ProcessOpen(chat_id, from_name);
      return;     
    }
    if (text == botCmdClose) {
      ProcessClose(chat_id, from_name);
      return;     
    }

    if (text == botCmdEditState) {
      ProcessEditState(chat_id);
      return;     
    }
    if (text == botCmdSetOpened) {
      ProcessSetOpened(chat_id, from_name);
      return;     
    }
    if (text == botCmdSetClosed) {
      ProcessSetClosed(chat_id, from_name);
      return;     
    }
    if (validateAdminlist(bot.messages[i].userName)) {
      if (text == botCmdWhitelist){
        ProcessWhiteList(chat_id, bot.messages[i].userName);
        return;     
      }
      if (text == botCmdWhitelistAdd){
        ProcessWhiteListAdd(chat_id,bot.messages[i].userName);
        return;     
      }
      if (text == botCmdWhitelistRemove){
        ProcessWhiteListRemove(chat_id,bot.messages[i].userName);
        return;     
      }
      if (text == botCmdAdminlist){       
        ProcessAdminList(chat_id, bot.messages[i].userName);
        return;
      }
      if (text == botCmdAdminlistAdd){
        ProcessAdminListAdd(chat_id,bot.messages[i].userName);
        return;     
      }
      if (text == botCmdAdminlistRemove){
        ProcessAdminListRemove(chat_id,bot.messages[i].userName);
        return;     
      }
      //почему то не переваривает в условии ENTER_MODE_xxxx      
      if (botEnterAdmin == bot.messages[i].userName && 
            (botEnterMode == ENTER_MODE_ADMIN_ADD || botEnterMode == ENTER_MODE_ADMIN_DELETE || botEnterMode == ENTER_MODE_USER_ADD || botEnterMode == ENTER_MODE_USER_DELETE)
          ) {
        if (text == "/yes") {
          ProcessYes(chat_id);
          return;
        }
        ProcessTextCommand(chat_id,text);
        return;
      }
    }
    ProcessUnparsedMessage(chat_id, bot.messages[i].userName);
    
  }
}


void modeStart() {
  eepromLoad();
  if (wifiMode == WIFI_MODE_HOST) {
    WiFi.config(HostIp, HostIp, HostSubnet);
    WiFi.softAP(hostSsid, hostPass);
    server.on("/", pageRoot);
    server.on("/set_ssid", pageSetSsid);
    server.on("/reload_to_client", pageReloadToClient);
    server.on("/set_token", pageSetToken);
    server.on("/set_admins", pageSetAdmins);
    #ifdef CUSTOM_RC
      server.on("/set_rc_commands", pageSetRcCommands);
    #endif
    server.begin(80);
    #ifdef CUSTOM_RC
      mySwitch.enableReceive(digitalPinToInterrupt(PIN_RC_IN));
    #endif
  }
  else {
    WiFi.mode(WIFI_STA);
    String ssid = "";
    String pass = "";
    uint8_t i = 0;
    while (connectToSsid[i] != '\0') {
      ssid += connectToSsid[i];
      i++;
    }
    i = 0;
    while (connectToPass[i] != '\0') {
      pass += connectToPass[i];
      i++;
    }
    WiFi.begin(ssid, pass);
    Serial.print("Connecting to ");
    Serial.println(ssid);
    bot.updateToken(botToken);
  }
  changeMode(MODE_IDLE);
}

uint32_t modeIdle_CheckBotTimer = 1000;
const uint32_t modeIdle_CheckBotDelay = 1000;
void modeIdle() {
  if (wifiMode == WIFI_MODE_HOST) {
    #ifdef CUSTOM_RC
      if (mySwitch.available()) {
        lastRcMessage = mySwitch.getReceivedValue();
      }
    #endif
    server.handleClient();
  }
  wifiConnected = (WiFi.status() == WL_CONNECTED);
  if ((millis() >= modeIdle_CheckBotTimer && modeIdle_CheckBotTimer >= modeIdle_CheckBotDelay) || (millis() < 1000 && modeIdle_CheckBotTimer < modeIdle_CheckBotDelay)) {
     changeMode(MODE_CHECK_BOT);
  }
}

void modeCheckBot() {
  if (!wifiConnected) {
    changeMode(MODE_IDLE);
    modeIdle_CheckBotTimer = millis() + modeIdle_CheckBotDelay;
    return;
  }
  int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  telegramStatus = bot.connected;
  if (!botEnabled && bot.connected) {
    botEnabled = true;
    String msgLog = "Бот перезагружен";
    bot.sendMessage(logChatId, msgLog, "");
  }

  while(numNewMessages) {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  }
  changeMode(MODE_IDLE);
  modeIdle_CheckBotTimer = millis() + 1000;
}

void setup() {
  delay(2000);
  EEPROM.begin(1024);
  Serial.begin(115200);
  Serial.println();
  Serial.println("Shlagbaum-bot");
  
  pinMode(PIN_LED_POWER, OUTPUT);
  pinMode(PIN_LED_WIFI, OUTPUT);
  pinMode(PIN_LED_TELEGRAMM, OUTPUT);
  pinMode(PIN_LED_DOUBLER, OUTPUT);
  pinMode(PIN_OUT_OPEN, OUTPUT);
  #ifdef TWO_BUTTON
    pinMode(PIN_OUT_CLOSE, OUTPUT);
  #endif
  pinMode(PIN_BTN_RESET, INPUT_PULLUP);
  pinMode(PIN_BTN_DOUBLER, INPUT_PULLUP);
  
  // super reset (set values to eeprom from firmware)
  if (!digitalRead(PIN_BTN_RESET) && !digitalRead(PIN_BTN_DOUBLER)) {
    digitalWrite(PIN_LED_POWER, 0);
    digitalWrite(PIN_LED_WIFI, 1);
    digitalWrite(PIN_LED_TELEGRAMM, 1);
    delay(1000);
    if (!digitalRead(PIN_BTN_RESET) && !digitalRead(PIN_BTN_DOUBLER)) {
      eepromSave(); 
      digitalWrite(PIN_LED_POWER, 1);
      digitalWrite(PIN_LED_WIFI, 0);
      digitalWrite(PIN_LED_TELEGRAMM, 0);
    }
  }
  
  configTime(0, 0, "pool.ntp.org"); // get UTC time via NTP
  client.setTrustAnchors(&cert); // Add root certificate for api.telegram.org
}

void loop() {  
  ledWork();
  buttonWork();
  switch (mode){
    case MODE_START: modeStart(); break;
    case MODE_IDLE: modeIdle(); break;
    case MODE_CHECK_BOT: modeCheckBot(); break;
  }
}