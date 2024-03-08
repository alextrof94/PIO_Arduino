#include <Arduino.h>
#include <SoftwareSerial.h>
#include <FastLED.h>
#include <fonts.h>
#include <EEPROM.h>

#define NUM_LEDS 256
#define NUM_LEDS_ALL_STRIP 292 //256+36
CRGB leds[NUM_LEDS_ALL_STRIP];

const uint8_t VR_R = 15;
const uint8_t VR_B = 30;

const int PIN_LEDS = 2;
const int PIN_LED = 13;
const int PIN_BT_RX = 4;
const int PIN_BT_TX = 5;
const int maxBrightness = 2; // 1-100

SoftwareSerial bt(PIN_BT_RX, PIN_BT_TX);

const uint8_t spaceBetweenLetters = 1;
uint32_t timerLedsUpdateDelay = 100;

uint32_t timerLedsUpdate = 0;
const char alphabet[] = "F<DULT:PBQRKVYJGHCNEA{WXIO}SM\">Zf,dult;pbqrkvyjghcnea[wxio]sm'.z"; // АБВ..Яабв..я

const uint8_t doFollowLength = 20;
const char doFollow[] = "1G1j1l1g1b1i1b1c1m0!";

char text[250] = "1G1j1l1g1b1i1b1c1m0!";
uint8_t textLength = 20;
CRGB textColor = CRGB(3, 3, 0);
char preTextAnim = 's'; // slide, fireworks

const int defStartColumn = 35;
int runningStringStartColumn = defStartColumn;

uint8_t currentAnim = 120;
uint32_t animRandomPixelTimer;
uint8_t animFillingPos = 0;
CRGB animFillingColor;
uint32_t animBallTimer;
struct Ball {
  CRGB Color;
  uint8_t X;
  uint8_t Y;
  bool VX;
  bool VY;
};
Ball balls[3];
struct Firework
{
  uint8_t startX;
  uint8_t startY;
  CRGB color;
  uint8_t delayInFrames;
  uint8_t frameNow = 0;
  bool ended; 
};
const uint8_t fireworksCount = 3;
Firework fireworks[fireworksCount];
const uint8_t fireworksFadeoutFrameDelay = 5;
uint8_t fireworksFadeoutFrame = 0;

const uint8_t badPixelCount = 10;
uint8_t badPixelIndexToRewrite = 0;
struct BadPixel
{
  uint8_t x;
  uint8_t y;
  CRGB color;
};
BadPixel badPixels[badPixelCount];

void setup() {
  pinMode(PIN_BT_RX, INPUT);
  pinMode(PIN_BT_TX, OUTPUT);
  Serial.begin(9600);
  bt.begin(9600);
  FastLED.addLeds<WS2812B, PIN_LEDS, GRB>(leds, NUM_LEDS_ALL_STRIP);  // GRB ordering is typical

  for (int i = 0; i < NUM_LEDS_ALL_STRIP; i++)
    leds[i] = CRGB(1,1,1);
  FastLED.show();
  delay(900);
  for (int i = 0; i < NUM_LEDS_ALL_STRIP; i++)
    leds[i] = CRGB::Black;
  FastLED.show();

  EEPROM.get(100, badPixelIndexToRewrite);
  if (badPixelIndexToRewrite > badPixelCount) {
    badPixelIndexToRewrite = 0;
    EEPROM.put(100, badPixelIndexToRewrite);
  }
  for (uint8_t i = 0; i < badPixelCount; i++) {
    EEPROM.get(100 + sizeof(badPixelIndexToRewrite) + (i * sizeof(badPixels[i])), badPixels[i]);
  }
  

  delay(100);
  Serial.println("BTPR started");
}

void fillPixel(uint8_t x, uint8_t y, CRGB color) {
  if (x < 0 || x > 31)
    return;
  if (y < 0 || y > 7)
    return;
  
  if (x % 2)
    leds[x * 8 + y] = color;
  else 
    leds[x * 8 + 7-y] = color;
}

CRGB GetColor(String s) {
  Serial.println(s);
  if (s == "red")
    return CRGB::Red;
  if (s == "gre")
    return CRGB::Green;
  if (s == "blu")
    return CRGB::Blue;
  if (s == "yel")
    return CRGB::Yellow;
  if (s == "cya")
    return CRGB::Cyan;
  if (s == "pur")
    return CRGB::Purple;
  if (s == "whi")
    return CRGB::White;
  return CRGB::Black;
}

CRGB PostProcessColor(CRGB inC) {
  CRGB outC;
  outC.r = (inC.r * maxBrightness) / 100;
  outC.g = (inC.g * maxBrightness) / 100;
  outC.b = (inC.b * maxBrightness) / 100;
  return outC;
}

CRGB getRandomColor() {
  CRGB res = 0;
  do {
    res = PostProcessColor(CRGB(rand() % 255, rand() % 255, rand() % 255));
  } while (res.r == 0 && res.g == 0 && res.b == 0);
  return res;
}

void clearLeds() {
  for (int i = 0; i < NUM_LEDS_ALL_STRIP; i++)
    leds[i] = 0;
  FastLED.show();
}

void drawColumn(uint8_t columnIndex, byte column) {
  if (columnIndex % 2 == 0) {
    for (uint8_t i = 0; i < 8; i++)
      leds[(NUM_LEDS - 1) - (i + ((columnIndex) * 8))] = ((column >> (7-i)) & 1) ? textColor : CRGB::Black;
  } 
  else {
    for (uint8_t i = 0; i < 8; i++)
      leds[(NUM_LEDS - 1) - (((columnIndex) * 8) + 7 - i)] = ((column >> (7-i)) & 1) ? textColor : CRGB::Black;
  }
}

void drawLetter(uint8_t letterIndex, uint8_t letterPos) {
  for (uint8_t c = 0; c < 5; c++) {
    int currentColumn = runningStringStartColumn + c + letterPos * (5 + spaceBetweenLetters);
    if (currentColumn < 0 || currentColumn > 31) {
      continue;
    }
    byte column = pgm_read_byte(&(fontHEX[letterIndex][c]));
    drawColumn(currentColumn, column);
  }
}

uint8_t getLetterIndexByChar(char enc, char c) {
  uint8_t res = 0;
  if (enc == '0') {
    res = c - '0' + 16;
  }
  if (enc == '1') {
    for (int i = 0; i < 64; i++)
      if (alphabet[i] == c) {
        res = i + 95;
        break;
      }
  }
  return res;
}

void drawString() {
  for (int i = 0; i < NUM_LEDS; i++)
    leds[i] = 0;
  for (uint8_t c = 0; c < textLength; c+=2) {
    uint8_t letterIndex = getLetterIndexByChar(text[c], text[c+1]);
    drawLetter(letterIndex, c/2);
  }
  FastLED.show();
}

void animPrepareRandomPixel(uint8_t nextAnim) {
  currentAnim = nextAnim;
  timerLedsUpdateDelay = 50;
  animRandomPixelTimer = millis() + (rand() % 20000) + 10000;
}

void animRandomPixel(uint8_t nextAnim) {
  leds[rand() % (NUM_LEDS - 1)] = PostProcessColor(CRGB(rand() % 255, rand() % 255, rand() % 255));
  FastLED.show();
  if (millis() > animRandomPixelTimer) {
    currentAnim = nextAnim;
  }
}

void animPrepareSlide(uint8_t nextAnim) {
  currentAnim = nextAnim;
  timerLedsUpdateDelay = 50;
  animFillingColor = getRandomColor();
  animFillingPos = 0;
}

void animSlide(uint8_t nextAnim) {
  if (animFillingPos < 32) {
    for (uint8_t i = 0; i < 8; i++)
      leds[animFillingPos * 8 + i] = animFillingColor;
  }
  else if (animFillingPos < 64) {
    for (uint8_t i = 0; i < 8; i++)
      leds[(animFillingPos - 32) * 8 + i] = 0;
  } else {
    currentAnim = nextAnim;
  }
  FastLED.show();
  animFillingPos++;
}

void animPreparePhrase(uint8_t nextAnim, bool resetPhrase) {
  timerLedsUpdateDelay = 100;
  runningStringStartColumn = defStartColumn;
  if (resetPhrase) {
    textLength = doFollowLength;
    textColor = getRandomColor();
    for (uint8_t i = 0; i < doFollowLength; i++) {
      text[i] = doFollow[i];
    }
  }
  currentAnim = nextAnim;
}

void animPhrase(uint8_t nextAnim) {
    runningStringStartColumn--;
    drawString();
    if (runningStringStartColumn < -(textLength/2) * (5 + spaceBetweenLetters)) {
      currentAnim = 0;
    }
}

void animPrepareBalls(uint8_t nextAnim) {
  timerLedsUpdateDelay = 50;
  animBallTimer = millis() + (rand() % 20000) + 10000;
  for (uint8_t b = 0; b < 3; b++) {
    balls[b].Color = getRandomColor();
    balls[b].X = rand() % 29 + 1;
    balls[b].Y = rand() % 5 + 1;
    balls[b].VX = (rand() % 2 == 1) ? true : false;
    balls[b].VY = (rand() % 2 == 1) ? true : false;
  }
  currentAnim = nextAnim;
}

void animBalls(uint8_t nextAnim) {
  for (int i = 0; i < NUM_LEDS; i++)
    leds[i] = 0;
  
  for (uint8_t b = 0; b < 3; b++) {
    balls[b].X += (balls[b].VX)?1:-1;
    balls[b].Y += (balls[b].VY)?1:-1;
    if (balls[b].X == 0 || balls[b].X == 30)
      balls[b].VX = !balls[b].VX;
    if (balls[b].Y == 0 || balls[b].Y == 6)
      balls[b].VY = !balls[b].VY;
    for (uint8_t x = balls[b].X; x < balls[b].X + 2; x++)
    for (uint8_t y = balls[b].Y; y < balls[b].Y + 2; y++) {
      if (x % 2)
        leds[x * 8 + y] = balls[b].Color;
      else
        leds[x * 8 + 7-y] = balls[b].Color;
    }
  }
  FastLED.show();

  if (millis() > animBallTimer) {
    currentAnim = nextAnim;
  }
}

void animPrepareFireworks(uint8_t nextAnim, bool needClear) {
  if (needClear)
    for (int i = 0; i < NUM_LEDS; i++)
      leds[i] = 0;

  for (int i = 0; i < fireworksCount; i++) {
    fireworks[i].color = getRandomColor();
    fireworks[i].delayInFrames = rand() % 20;
    fireworks[i].startX = rand() % 32;
    fireworks[i].startY = rand() % 6 + 1;
    fireworks[i].frameNow = 0;
    fireworks[i].ended = false;
  }

  timerLedsUpdateDelay = 80;
  fireworksFadeoutFrame = 0;
  currentAnim = nextAnim;
}

void animFireworks(uint8_t nextAnim) {
  fireworksFadeoutFrame++;
  if (fireworksFadeoutFrame >= fireworksFadeoutFrameDelay) {
    for (int i = 0; i < NUM_LEDS; i++){
      leds[i].r = (leds[i].r > 0) ? leds[i].r - 1 : leds[i].r;
      leds[i].g = (leds[i].g > 0) ? leds[i].g - 1 : leds[i].g;
      leds[i].b = (leds[i].b > 0) ? leds[i].b - 1 : leds[i].b;
    }
    fireworksFadeoutFrame = 0;
  }

  for (int i = 0; i < fireworksCount; i++) {
    if (fireworks[i].frameNow >= fireworks[i].delayInFrames) {
      // draw firework
      if (fireworks[i].frameNow - fireworks[i].delayInFrames == 0) {
        fillPixel(fireworks[i].startX, fireworks[i].startY, fireworks[i].color);
      }

      if (fireworks[i].frameNow - fireworks[i].delayInFrames == 2) {
        if (i % 2 == 1) {
          fillPixel(fireworks[i].startX-1, fireworks[i].startY-1, fireworks[i].color);
          fillPixel(fireworks[i].startX+1, fireworks[i].startY-1, fireworks[i].color);
          fillPixel(fireworks[i].startX-1, fireworks[i].startY+1, fireworks[i].color);
          fillPixel(fireworks[i].startX+1, fireworks[i].startY+1, fireworks[i].color);
        }
        else {
          fillPixel(fireworks[i].startX-1, fireworks[i].startY, fireworks[i].color);
          fillPixel(fireworks[i].startX+1, fireworks[i].startY, fireworks[i].color);
          fillPixel(fireworks[i].startX, fireworks[i].startY-1, fireworks[i].color);
          fillPixel(fireworks[i].startX, fireworks[i].startY+1, fireworks[i].color);
        }
      }

      if (fireworks[i].frameNow - fireworks[i].delayInFrames == 5) {
        if (i % 2 == 0) {
          fillPixel(fireworks[i].startX-2, fireworks[i].startY-2, fireworks[i].color);
          fillPixel(fireworks[i].startX+2, fireworks[i].startY-2, fireworks[i].color);
          fillPixel(fireworks[i].startX-2, fireworks[i].startY+2, fireworks[i].color);
          fillPixel(fireworks[i].startX+2, fireworks[i].startY+2, fireworks[i].color);
        }
        else {
          fillPixel(fireworks[i].startX-2, fireworks[i].startY, fireworks[i].color);
          fillPixel(fireworks[i].startX+2, fireworks[i].startY, fireworks[i].color);
          fillPixel(fireworks[i].startX, fireworks[i].startY-2, fireworks[i].color);
          fillPixel(fireworks[i].startX, fireworks[i].startY+2, fireworks[i].color);
        }
      }

      if (fireworks[i].frameNow - fireworks[i].delayInFrames == 9) {
        if (i % 2 == 0) {
          fillPixel(fireworks[i].startX-3, fireworks[i].startY, fireworks[i].color);
          fillPixel(fireworks[i].startX+3, fireworks[i].startY, fireworks[i].color);
          fillPixel(fireworks[i].startX, fireworks[i].startY-3, fireworks[i].color);
          fillPixel(fireworks[i].startX, fireworks[i].startY+3, fireworks[i].color);
        }
      }

      // check ended
      if (fireworks[i].frameNow - fireworks[i].delayInFrames > 15)
        fireworks[i].ended = true;
    }
    fireworks[i].frameNow++;
  }
  FastLED.show();

  bool notEnded = false;
  for (int i = 0; i < fireworksCount; i++) {
    if (!fireworks[i].ended)
      notEnded = true;
  }
  if (!notEnded)
    currentAnim = nextAnim;
}

void updateLeds() {
  if (currentAnim == 0) {
    int whichRandom = rand() % 2;
    switch(whichRandom) {
      case 1:
        currentAnim = 10;
      break;
      default:
        currentAnim = 20;
      break; 
    }
    return;
  }

  if (currentAnim == 1) { // SLIDE TO DEFAULT PHRASE
    animPrepareSlide(2);
    return;
  }
  if (currentAnim == 2) {
    animSlide(3);
    return;
  }
  if (currentAnim == 3) { // DEFAULT PHRASE
    animPreparePhrase(4, true);
    return;
  }
  if (currentAnim == 4) {
    animPhrase(0);
    return;
  }

  // FILLERS ANIMS
  if (currentAnim == 10) { // FILLER RANDOM PIXEL
    animPrepareRandomPixel(11);
    return;
  }
  if (currentAnim == 11) {
    animRandomPixel(1);
    return;
  }

  if (currentAnim == 20) { // FILLER BALLS
    animPrepareBalls(21);
    return;
  }
  if (currentAnim == 21) {
    animBalls(1);
    return;
  }



  if (currentAnim == 100) { // FROM BT TO ANIM
    switch (preTextAnim)
    {
    case 's':
      currentAnim = 110;
      break;
    case 'f':
      currentAnim = 120;
      break;
    
    default:
      currentAnim = 110;
      break;
    }
    return;
  }
  
  // PRE TEXT ANIMS
  if (currentAnim == 110) { // SLIDE TO PHRASE
    animPrepareSlide(111);
    return;
  }
  if (currentAnim == 111) {
    animSlide(200);
    return;
  }

  if (currentAnim == 120) { // FIREWORKS TO FIREWORKS
    animPrepareFireworks(121, true);
    return;
  }
  if (currentAnim == 121) {
    animFireworks(122);
    return;
  }
  if (currentAnim == 122) { // FIREWORKS TO FIREWORKS
    animPrepareFireworks(123, false);
    return;
  }
  if (currentAnim == 123) {
    animFireworks(124);
    return;
  }
  if (currentAnim == 124) { // FIREWORKS TO FIREWORKS
    animPrepareFireworks(125, false);
    return;
  }
  if (currentAnim == 125) {
    animFireworks(126);
    return;
  }
  if (currentAnim == 126) { // FIREWORKS TO FIREWORKS
    animPrepareFireworks(127, false);
    return;
  }
  if (currentAnim == 127) {
    animFireworks(128);
    return;
  }
  if (currentAnim == 128) { // FIREWORKS TO PHRASE
    animPrepareFireworks(129, false);
    return;
  }
  if (currentAnim == 129) {
    animFireworks(200);
    return;
  }

  if (currentAnim == 200) { // PHRASE
    animPreparePhrase(201, false);
    return;
  }
  if (currentAnim == 201) {
    animPhrase(0);
    return;
  }
}

void updateVr() {
  leds[256] = CRGB(0,0,VR_B);
  leds[257] = CRGB(0,0,VR_B);
  leds[258] = CRGB(0,0,VR_B);
  leds[259] = CRGB(0,0,VR_B);

  leds[260] = CRGB(VR_R,0,0);
  leds[261] = CRGB(VR_R,0,0);
  leds[262] = CRGB(VR_R,0,0);
  leds[263] = CRGB(VR_R,0,0);

  leds[264] = CRGB(0,0,VR_B);
  leds[265] = CRGB(0,0,VR_B);
  leds[266] = CRGB(0,0,VR_B);

  leds[267] = CRGB(VR_R,0,0);
  leds[268] = CRGB(VR_R,0,0);
  leds[269] = CRGB(VR_R,0,0);
  leds[270] = CRGB(VR_R,0,0);

  leds[271] = CRGB(0,0,0);

  leds[272] = CRGB(0,0,VR_B);
  leds[273] = CRGB(0,0,VR_B);
  
  leds[274] = CRGB(0,0,0);
  leds[275] = CRGB(0,0,0);

  leds[276] = CRGB(0,0,VR_B);
  leds[277] = CRGB(0,0,VR_B);

  leds[278] = CRGB(VR_R,0,0);
  leds[279] = CRGB(VR_R,0,0);
  leds[280] = CRGB(VR_R,0,0);
  leds[281] = CRGB(VR_R,0,0);

  leds[282] = CRGB(0,0,0);
  leds[283] = CRGB(0,0,0);

  leds[284] = CRGB(0,0,VR_B);
  leds[285] = CRGB(0,0,VR_B);
  leds[286] = CRGB(0,0,VR_B);
  leds[287] = CRGB(0,0,VR_B);

  leds[288] = CRGB(VR_R,0,0);
  leds[289] = CRGB(VR_R,0,0);
  leds[290] = CRGB(VR_R,0,0);
  leds[291] = CRGB(VR_R,0,0);
  //FastLED.show();
}

void updateBadPixels() {
  for (uint8_t bpi = 0; bpi < badPixelCount; bpi++) {
    if (badPixels[bpi].color.r == 0 && badPixels[bpi].color.g == 0 && badPixels[bpi].color.b == 0)
      continue;
    if (badPixels[bpi].x % 2 == 0) {
        leds[(NUM_LEDS - 1) - ((badPixels[bpi].x) * 8)] = badPixels[bpi].color;
    } 
    else {
        leds[(NUM_LEDS - 1) - ((badPixels[bpi].x) * 8) + 7] = badPixels[bpi].color;
    }
  }
}

uint8_t getCoordByChars(char c1, char c2) {
  uint8_t o = 0;
  if (c1 < '0' || c1 > '9' || c2 < '0' || c2 > '9')
    return 100;
  o = (c1 - '0') * 10;
  o += c2 - '0';
  return o;
}

void updateBadPixelInEeprom(uint8_t x, uint8_t y) {
  badPixels[badPixelIndexToRewrite].x = x;
  badPixels[badPixelIndexToRewrite].y = y;
  badPixels[badPixelIndexToRewrite].color = getRandomColor();
  EEPROM.put(100 + sizeof(badPixelIndexToRewrite) + (badPixelIndexToRewrite * sizeof(badPixels[badPixelIndexToRewrite])), badPixels[badPixelIndexToRewrite]);
  badPixelIndexToRewrite++;
  EEPROM.put(100, badPixelIndexToRewrite);
}

uint8_t cind;
void loop() {
  if (millis() >= timerLedsUpdate) {
    timerLedsUpdate = millis() + timerLedsUpdateDelay;
    updateVr();
    updateLeds();
    updateBadPixels();
  }

  if (bt.available()) {
    String cmd = bt.readString();
    Serial.println(cmd);
    if (cmd == "BTPS\n") {
      bt.println("OK");
      Serial.println("CONNECTED");
    }
    else {
      if (cmd.length() < 1) {
        bt.println("ERR: empty");
        return;
      }
      char cmdChar = cmd[0];
      switch (cmdChar)
      {
        case 't':
          if (cmd.length() < 9) {
            bt.println("ERR: t col p text");
            return;
          }
          textColor = PostProcessColor(GetColor(cmd.substring(2, 5)));

          preTextAnim = cmd.charAt(7);

          char t;
          cind = 0;
          do {
            t = cmd.charAt(cind + 8);
            text[cind] = t;
            cind++;
          } while (t != '\n' && cind < 250);
          textLength = cind - 1;
          currentAnim = 100;
          bt.println("OKT");
          break;
        case 'b':
          if (cmd.length() < 7) {
            bt.println("ERR: b xx yy");
            return;
          }

          updateBadPixelInEeprom(getCoordByChars(cmd.charAt(2), cmd.charAt(3)), getCoordByChars(cmd.charAt(5), cmd.charAt(6)));

          currentAnim = 100;
          bt.println("OKT");
          break;
        default:
          break;
      }
    }
  }
}