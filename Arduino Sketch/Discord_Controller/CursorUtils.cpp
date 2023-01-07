#include "Arduino.h"
#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>
#include "CursorUtils.h"

CursorUtils::CursorUtils(MCUFRIEND_kbv* tft) {
  _tft = tft;
}

CursorUtils::save_cursor_pos() {
  _saved_cursor[0] = _tft->getCursorX();
  _saved_cursor[1] = _tft->getCursorY();
}

CursorUtils::revert_cursor_pos() {
  _tft->setCursor(_saved_cursor[0], _saved_cursor[1]);
}

CursorUtils::revert_cursor() {
  revert_cursor_pos();
  _tft->setTextSize(1);
  _tft->setTextColor(WHITE);
}

CursorUtils::fillRect_safe(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
  save_cursor_pos();
  _tft->fillRect(x, y, w, h, color);
  revert_cursor();
}

CursorUtils::dotLoop(bool* condition, int delayAmount, bool boot = false) {
  int dot_index = 0;

  while (!*condition) {
      if (dot_index < 3) {
        revert_cursor_pos();
        _tft->print(dots[dot_index]);
        dot_index++;
      } else {
        revert_cursor_pos();
        _tft->setTextColor(BLACK);
        _tft->print(dots[2]);
        _tft->setTextColor(WHITE);
        dot_index = 0;
      }

      if (Serial.available()) {
        *condition = true;
      } else {
        if (boot) Serial.print("<#P0>");
        delay(delayAmount);
      }
    }
}