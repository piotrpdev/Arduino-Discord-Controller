#ifndef DiscordButton_h
#define DiscordButton_h

#define BTN_W 76

#include "Arduino.h"
#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>
#include "CursorUtils.h"

class DiscordButton {
public:
  DiscordButton(MCUFRIEND_kbv* tft, CursorUtils* _c, int16_t x, int16_t y, int16_t rectY, char * label, char type);
  Adafruit_GFX_Button* getBtn();
  init();
  toggle();
  bool getState();
  setState(bool state);
  on();
  off();
  enable();
  disable();
  checkIfPressed();
private:
  MCUFRIEND_kbv* _tft;
  CursorUtils* _c;
  int16_t _x;
  int16_t _y;
  char * _label;
  int16_t _rectY;
  bool _state;
  char _type;
  bool _disabled;
  issueCommand(char type, bool state);
  Adafruit_GFX_Button btn;
};

#endif