#ifndef __MY_SERVOS
#define __MY_SERVOS

#define PIN_SERVOS_CONTROL 4
#define HARDWARE_SERVOS_COUNT 6
#define SETTINGS_SEND_DELAY 500 // microseconds
#define SETTINGS_ACCURACITY_L 0x50
#define SETTINGS_ACCURACITY_N 0x18
#define SETTINGS_ACCURACITY_H 0x10 // units (1 unit for 0.29 degrees)
#define HARDWARE_SERVOS_BAUDRATE 1000000    // Baud rate speed for Dynamixels
#define TIME_FOR_ERROR_OUT 3000

enum HandTypes {
  HTCLAW, HTDRILL, HTLASERPOINTER
};

enum Anims {
  ANIMINIT = 0, ANIMREADY = 1, 
  ANIMTOGET = 2, ANIMAFTERGET = 3, 
  ANIMCHECK, ANIMCHECKED, 
  ANIMTORED, ANIMAFTERRED, 
  ANIMTOGREEN, ANIMAFTERGREEN,
  ANIMTOBLUE, ANIMAFTERBLUE,
  ANIMTOYELLOW, ANIMAFTERYELLOW,
  ANIMHELLOREADY, ANIMHELLO,
  ANIMTOQR1, ANIMAFTERQR1, 
  ANIMTOQR2, ANIMAFTERQR2,
  ANIMTOBC1, ANIMAFTERBC1, 
  ANIMTOBC2, ANIMAFTERBC2
};

bool servosHandCheckHall();
bool servosAllIsReady();
void servosSetPoses(int pos, uint8_t spd = 2);
void servosAnim(uint8_t anim, int forceSpeed = -1);
void servosHandSetType(HandTypes type);
bool servosHandIsReady();
void servosHandSetEnable(uint8_t pos = 2, uint8_t spd = 1);
void servosInit();
void servosModeWork();
void servosModeOff();

#endif