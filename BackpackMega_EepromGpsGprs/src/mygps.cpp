#include "main.h"
uint32_t updateTimer;
const uint32_t updateTimerDelay = 1000;
TinyGPSPlus gps;

void update() {
	while (GPS.available() > 0) {
		if (gps.encode(GPS.read())) {
			// data getted
			if (gps.location.isValid()) {
				// save time of latest update
				gpsTimeSavedLocation = millis();
				gpsLat = gps.location.lat();
				gpsLng = gps.location.lng();
			}
			if (gps.date.isValid()) {
				// save time of latest update
				gpsTimeSavedDate = millis();
				gpsDay = gps.date.day();
				gpsMonth = gps.date.month();
				gpsYear = gps.date.year();
			}
			if (gps.time.isValid()) {
				// save time of latest update
				gpsTimeSavedTime = millis();
				gpsHour = gps.time.hour();
				gpsMinute = gps.time.minute();
				gpsSeconds = gps.time.second();
			}
		}
	}
}

void mygpsWork() {
	if (millis() > updateTimer) {
		updateTimer = millis() + updateTimerDelay;
		update();
	}
}