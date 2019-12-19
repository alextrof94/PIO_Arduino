#ifndef __MY_LCD
#define __MY_LCD

void lcdClear(int mode);
void lcdPrint(char* str, int x = 0, int y = 0);
void lcdInit();

#endif