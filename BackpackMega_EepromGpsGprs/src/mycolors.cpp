#include <Arduino.h>

const uint32_t colors[4] PROGMEM = {
	0x0, // 0
	0xffffff, // 1
	0xff0000, // 2
	0x00ff00  // 3
};

uint32_t getColorByByte(byte b) {
	return colors[(b%4)];
}
