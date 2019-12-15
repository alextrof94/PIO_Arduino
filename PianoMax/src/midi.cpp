#include <Arduino.h>
#define MIDI Serial3

struct MidiMsg {
	uint8_t ch = 0;
	uint8_t note = 0;
	uint8_t vel = 0;
};

void midiInit() {
	MIDI.begin(31250);
}