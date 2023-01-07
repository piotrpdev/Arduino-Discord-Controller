#ifndef CursorUtils_h
#define CursorUtils_h

#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

#include "Arduino.h"
#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>

class CursorUtils {
public:
  CursorUtils(MCUFRIEND_kbv* tft);
  save_cursor_pos();
  revert_cursor_pos();
  revert_cursor();
  dotLoop(bool* condition, int delayAmount, bool boot = false);
  char* dots[3] = { ".", "..", "..." }; 
private:
  MCUFRIEND_kbv* _tft;
  int _saved_cursor[2];
};

#endif