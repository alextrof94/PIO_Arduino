#include <Arduino.h>

#include <external.h>
#include <main.h>

const uint8_t externalTypesCount = 2;

ExternalTypes externalType = ETDISPENSER;
bool ExternalReady = false;

void ExternalCheck() {
  if (PORT_EXTERNAL.available()) {
    char cmd = PORT_EXTERNAL.read();
    switch (cmd) {
      case 'i': // i'm externalType
	  	switch (PORT_EXTERNAL.read()) {
		  case 1:
			  externalType = ETDISPENSER;
			  break;
		  default: externalType = ETNONE;
			  break;
		}
        break;
      case 'r': // ready?
        ExternalReady = (PORT_EXTERNAL.read() == 1);
        break;
      case 'a': // additional
        // char a = EXT.read();
        // do something
        break;
    }
  }
  if (PORT_EXTERNAL.available())
    ExternalCheck(); // recursion for read all data
}


void ExternalInit() {
  PORT_EXTERNAL.begin(57600);
}