#ifndef __INC_MYGPS
#define __INC_MYGPS

#include <TinyGps++.h>
#include "main.h"


static uint32_t gpsTimeSavedLocation;
static int gpsLat, gpsLng;
static uint32_t gpsTimeSavedDate;
static int gpsDay, gpsMonth, gpsYear;
static uint32_t gpsTimeSavedTime;
static int gpsHour, gpsMinute, gpsSeconds;
void mygpsWork();

#endif
