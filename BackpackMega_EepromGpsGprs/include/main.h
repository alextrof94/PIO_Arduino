#ifndef __INC_MYMAIN
#define __INC_MYMAIN

#define PC Serial
#define GPS Serial1
#define GPRS Serial2

#include <Arduino.h>
#include <SD.h>
#include <FastLED.h>
#include "mygps.h"
#include "mygprs.h"
#include "myleds.h"
#include "mysd.h"
#include "mypc.h"

static CRGBArray<256> leds;
static const uint8_t LEDSTRIP_SEGMENT_COUNT = 2;
static const uint8_t PIN_LEDSTRIP_SEGMENT_SELECT[2] = {3,4};

static const uint8_t PIN_LIGHTSENSOR = A1;
static uint8_t brightness = 0;
static uint8_t brightnessSensorMultiplexer = 1;
static uint8_t brightnessMax = 100; 

const uint8_t PIN_SD_CHIPSELECT = 50;


/*
  modes: 
  0 - text 16x8 / anim 16x16 / text 16x8
  1 - anim 16x32
  2 - text 16x32 rotated
  3 - piano receive mode
  4 - bicycle mode: text 16x8 / anim16x16 / signals 16x8
  5 - audio-light
*/
static uint8_t mode = 0;
static bool gpsEnabled = true;
static bool gprsEnabled = true;
static bool sdEnabled = true;
static bool sensorEnabled = true;
#define URL "chkalovc.ru/p/bp/index.php"


#endif