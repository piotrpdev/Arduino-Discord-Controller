#ifndef DiscordButton_h
#define DiscordButton_h

#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

#define BTN_W 76

#include "Arduino.h"
#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>

class DiscordButton {
public:
  DiscordButton(MCUFRIEND_kbv* tft, int16_t x, int16_t y, int16_t rectY, char * label, char type);
  Adafruit_GFX_Button* getBtn();
  init();
  toggle();
  setState(bool state);
  on();
  off();
  enable();
  disable();
  checkIfPressed();
private:
  MCUFRIEND_kbv* _tft;
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