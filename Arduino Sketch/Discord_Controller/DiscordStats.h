#ifndef DiscordStats_h
#define DiscordStats_h

#include "Arduino.h"
#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>
#include "CursorUtils.h"

class DiscordStats {
public:
  DiscordStats(MCUFRIEND_kbv* tft, CursorUtils* c);
  bool second_passed();
  printTime();
  updateTimer();
  init();
  clear();
  unsigned long _epoch;
private:
  MCUFRIEND_kbv* _tft;
  CursorUtils* _c;
  unsigned long _last_updated;
};

#endif