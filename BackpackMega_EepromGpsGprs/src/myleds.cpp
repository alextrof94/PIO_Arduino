/*
  modes: 
  0 - text 16x8 / anim 16x16 / text 16x8
  1 - anim 16x32
  2 - text 16x32 rotated
  3 - text 16x32 vertical
  4 - piano receive mode
  5 - bicycle mode: text 16x8 / anim16x16 / signals 16x8
  6 - audio-light
*/

#include <Arduino.h>
#include <mycolors.h>
#include <FastLED.h>
#include <main.h>

//extern uint8_t brightness;
//extern uint8_t mode;
//extern CRGBArray<256> leds;
//extern uint8_t PIN_LEDSTRIP_SEGMENT_SELECT[2];
//extern uint8_t LEDSTRIP_SEGMENT_COUNT;

const uint8_t textViewHeight[2] = {0, 24};
// textView [textViewCount][LEDSTRIP_SEGMENT_COUNT * 8][8]
uint8_t textView[2][16][8] = {
	{
		{1,0,0,0,0,0,0,0},
		{0,2,0,0,0,0,0,0},
		{0,0,3,0,0,0,0,0},
		{0,0,0,4,0,0,0,0},
		{0,0,0,0,5,0,0,0},
		{0,0,0,0,0,6,0,0},
		{0,0,0,0,0,0,7,0},
		{0,0,0,0,0,0,0,6},
		{0,0,0,0,0,0,5,0},
		{0,0,0,0,0,4,0,0},
		{0,0,0,0,3,0,0,0},
		{0,0,0,2,0,0,0,0},
		{0,0,1,0,0,0,0,0},
		{0,2,0,0,0,0,0,0}
	},
	{
		{1,0,0,0,0,0,0,0},
		{0,2,0,0,0,0,0,0},
		{0,0,3,0,0,0,0,0},
		{0,0,0,4,0,0,0,0},
		{0,0,0,0,5,0,0,0},
		{0,0,0,0,0,6,0,0},
		{0,0,0,0,0,0,7,0},
		{0,0,0,0,0,0,0,6},
		{0,0,0,0,0,0,5,0},
		{0,0,0,0,0,4,0,0},
		{0,0,0,0,3,0,0,0},
		{0,0,0,2,0,0,0,0},
		{0,0,1,0,0,0,0,0},
		{0,2,0,0,0,0,0,0}
	}
};

/*
void workText2() {
  switch (effect2) {
    case 0: break;
    case 1: 
      colColor2++;
      if (colColor2 >= 7)
        colColor2 = 1; 
      break;
  }
  for (uint8_t i = 0; i < LEDSTRIP_SEGMENT_COUNT * 8 - 1; i++) {
    for (uint8_t j = 0; j < 8; j++) {
      textView2[i][j] = textView2[i + 1][j];
    }
  }

  // set new col
  uint8_t lastColIndex2 = LEDSTRIP_SEGMENT_COUNT * 8 - 1;
  charCol2++;
  if (textInLength < 2) {
    // for flashed text
    if (charCol2 < pgm_read_byte(&alphabetLengths[assignedAlphabetCharIndex2])) {
      // next col in char
      for (uint8_t j = 0; j < 8; j++)
        textView2[lastColIndex2][j] = ((pgm_read_byte(&chars[assignedAlphabetCharStart2 + charCol2]) >> j) & 1)? colColor2 : 0;
    } else if (charCol2 > pgm_read_byte(&alphabetLengths[assignedAlphabetCharIndex2]) + spaceBetweenChars) {
      // new char
      switch (effect2) {
        case 2: 
          colColor2++;
          if (colColor2 >= 7)
            colColor2 = 1; 
          break;
      }
      charCol2 = 0;
      textCharPos2++;
      if (textCharPos2 > strlen(text2) - 1) {
        // new string
        switch (effect2) {
          case 3: 
            colColor2++;
            if (colColor2 >= 7)
              colColor2 = 1; 
            break;
        }
        textCharPos2 = 0;
        textCycle2++;
        if (textCycle2 >= textCycleCount) {
          textCycle2 = 0;
          // DO SOMETHING EVERY SOME CYCLES
          switch (effect2) {
            case 4: 
              colColor2++;
              if (colColor2 >= 7)
                colColor2 = 1; 
              break;
          }
        }
      }
      
      // INTEXT effects <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
      
      if ((char)pgm_read_byte(&text2[textCharPos2]) == '~') {
        textCharPos2++;
        char ch = (char)pgm_read_byte(&text2[textCharPos2++]);
        if (ch == 'c') { // color
            uint8_t numberC = 0;
            numberC += (pgm_read_byte(&text2[textCharPos2++]) - '0')*100;
            numberC += (pgm_read_byte(&text2[textCharPos2++]) - '0')*10;
            numberC += pgm_read_byte(&text2[textCharPos2++]) - '0';
            colColor2 = numberC;
        } else
        if (ch == 'd') { // speed
            uint32_t numberS = 0;
            numberS += (pgm_read_byte(&text2[textCharPos2++]) - '0')*100000;
            numberS += (pgm_read_byte(&text2[textCharPos2++]) - '0')*10000;
            numberS += (pgm_read_byte(&text2[textCharPos2++]) - '0')*1000;
            numberS += (pgm_read_byte(&text2[textCharPos2++]) - '0')*100;
            numberS += (pgm_read_byte(&text2[textCharPos2++]) - '0')*10;
            numberS += pgm_read_byte(&text2[textCharPos2++]) - '0';
            textColSwitchingDelay2 = numberS;
        }
      }
      // INTEXT EFFECTS END
      assignedAlphabetCharIndex2 = 255;
      assignedAlphabetCharStart2 = 0;
      for (int i = 0 ; i < ALPHABET_COUNT; i++) {
        if ((char)pgm_read_byte(&text2[textCharPos2]) == (char)pgm_read_byte(&alphabet[i])) {
          assignedAlphabetCharIndex2 = i;
          break;
        }
        assignedAlphabetCharStart2 += (int)pgm_read_byte(&alphabetLengths[i]);
      }
      if (assignedAlphabetCharIndex2 == 255) {
        assignedAlphabetCharIndex2 = 0;
        assignedAlphabetCharStart2 = 0;
      }
      // set col
      for (uint8_t j = 0; j < 8; j++)
        textView2[lastColIndex2][j] = ((pgm_read_byte(&chars[assignedAlphabetCharStart2 + charCol2]) >> j) & 1)? colColor2 : 0;
    } else {
      // just space
      for (uint8_t i = 0; i < 8; i++) {
        textView2[lastColIndex2][i] = 0;
      }
    }
  } else {
    // for INPUTTED text
    if (charCol2 < pgm_read_byte(&alphabetLengths[assignedAlphabetCharIndex2])) {
      // next col in char
      for (uint8_t j = 0; j < 8; j++)
        textView2[lastColIndex2][j] = ((pgm_read_byte(&chars[assignedAlphabetCharStart2 + charCol2]) >> j) & 1)? colColor2 : 0;
    } else if (charCol2 > pgm_read_byte(&alphabetLengths[assignedAlphabetCharIndex2]) + spaceBetweenChars) {
      // new char
      switch (effect2) {
        case 2: 
          colColor2++;
          if (colColor2 >= 7)
            colColor2 = 1; 
          break;
      }
      charCol2 = 0;
      textCharPos2++;
      if (textCharPos2 > textInLength - 1) {
        // new string
        switch (effect2) {
          case 3: 
            colColor2++;
            if (colColor2 >= 7)
              colColor2 = 1; 
            break;
        }
        textCharPos2 = 0;
        textCycle2++;
        if (textCycle2 >= textCycleCount) {
          textCycle2 = 0;
          // DO SOMETHING EVERY SOME CYCLES
          switch (effect2) {
            case 4: 
              colColor2++;
              if (colColor2 >= 7)
                colColor2 = 1; 
              break;
          }
        }
      }
      
      // INTEXT effects <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<      
      if (EEPROM.read(textInAddress + textCharPos2) == '~') {
        textCharPos2++;
        char ch = EEPROM.read(textInAddress + textCharPos2++);
        if (ch == 'c') { // color
            uint8_t numberC = 0;
            numberC += (EEPROM.read(textInAddress + textCharPos2++) - '0')*100;
            numberC += (EEPROM.read(textInAddress + textCharPos2++) - '0')*10;
            numberC += EEPROM.read(textInAddress + textCharPos2++) - '0';
            colColor2 = numberC;
        } else
        if (ch == 'd') { // speed
            uint32_t numberS = 0;
            numberS += (EEPROM.read(textInAddress + textCharPos2++) - '0')*100000;
            numberS += (EEPROM.read(textInAddress + textCharPos2++) - '0')*10000;
            numberS += (EEPROM.read(textInAddress + textCharPos2++) - '0')*1000;
            numberS += (EEPROM.read(textInAddress + textCharPos2++) - '0')*100;
            numberS += (EEPROM.read(textInAddress + textCharPos2++) - '0')*10;
            numberS += EEPROM.read(textInAddress + textCharPos2++) - '0';
            textColSwitchingDelay2 = numberS;
        }
      }
      // INTEXT EFFECTS END
      assignedAlphabetCharIndex2 = 255;
      assignedAlphabetCharStart2 = 0;
      for (int i = 0 ; i < ALPHABET_COUNT; i++) {
        if (EEPROM.read(textInAddress + textCharPos2) == (char)pgm_read_byte(&alphabet[i])) {
          assignedAlphabetCharIndex2 = i;
          break;
        }
        assignedAlphabetCharStart2 += (int)pgm_read_byte(&alphabetLengths[i]);
      }
      if (assignedAlphabetCharIndex2 == 255) {
        assignedAlphabetCharIndex2 = 0;
        assignedAlphabetCharStart2 = 0;
      }
      // set col
      for (uint8_t j = 0; j < 8; j++)
        textView2[lastColIndex2][j] = ((pgm_read_byte(&chars[assignedAlphabetCharStart2 + charCol2]) >> j) & 1)? colColor2 : 0;
    } else {
      // just space
      for (uint8_t i = 0; i < 8; i++) {
        textView2[lastColIndex2][i] = 0;
      }
    }
  }
}
/**/

void textViewToLeds(uint8_t whichTextView, uint8_t whichSegment) {
  for (uint8_t i = 0; i < 8; i++) {
    if (i % 2) {
      for (uint8_t j = 0; j < 8; j++) {
        leds[(31 - (textViewHeight[whichTextView] + i)) * 8 + j] = getColorByByte(textView[whichTextView][whichSegment * 8 + j][i]);
      }
    } else {
      for (uint8_t j = 0; j < 8; j++) {
        leds[(31 - (textViewHeight[whichTextView] + i)) * 8 + j] = getColorByByte(textView[whichTextView][whichSegment * 8 + 7 - j][i]);
      }
    }
  }
}

void textViewToLedsRotatedFull(uint8_t whichTextView, uint8_t whichSegment) {
	// NOT COMLETED
	for (uint8_t i = 0; i < 8; i++) {
		if (i % 2) {
			for (uint8_t j = 0; j < 8; j++) {
				leds[(31 - (textViewHeight[whichTextView] + i)) * 8 + j] = getColorByByte(textView[whichTextView][whichSegment * 8 + j][i]);
			}
		} else {
			for (uint8_t j = 0; j < 8; j++) {
				leds[(31 - (textViewHeight[whichTextView] + i)) * 8 + j] = getColorByByte(textView[whichTextView][whichSegment * 8 + 7 - j][i]);
			}
		}
	}
}

void myledsWork() {
	// CHECK SPEED TEST
	if (mode == 0) {
		// do 0
		for (uint8_t i = 0; i < LEDSTRIP_SEGMENT_COUNT; i++) {
			// off all segments
			for (uint8_t j = 0; j < LEDSTRIP_SEGMENT_COUNT; j++)
				digitalWrite(PIN_LEDSTRIP_SEGMENT_SELECT[j], 0);
			// on only selected
			digitalWrite(PIN_LEDSTRIP_SEGMENT_SELECT[i], 1);
			// send data
			textViewToLeds(0, i);
			textViewToLeds(1, i);
			//animToLeds(i); -<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
			// set brightness
			leds.fadeToBlackBy(255 - brightness);
			// show
			FastLED.show();
		}
	} else if (mode == 1) {
		// do 1: anim 16x32
	} else if (mode == 2) {
		// do 2: text 16x32 rotated
		for (uint8_t i = 0; i < LEDSTRIP_SEGMENT_COUNT; i++) {
			// off all segments
			for (uint8_t j = 0; j < LEDSTRIP_SEGMENT_COUNT; j++)
				digitalWrite(PIN_LEDSTRIP_SEGMENT_SELECT[j], 0);
			// on only selected
			digitalWrite(PIN_LEDSTRIP_SEGMENT_SELECT[i], 1);
			// send data
			textViewToLedsRotatedFull(0, i);
			// set brightness
			leds.fadeToBlackBy(255 - brightness);
			// show
			FastLED.show();
		}
	} else if (mode == 3) {
		// do 3: text 16x32 vertical
	} else if (mode == 4) {
		// do 4: piano receive
	} else if (mode == 5) {
		// do 5: bicycle mode: text 16x8 / anim16x16 / signals 16x8
	} else if (mode == 6) {
		// do 6: audio-light
	}
}