#include "Arduino.h"
#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>
#include "DiscordStats.h"
#include "CursorUtils.h"

DiscordStats::DiscordStats(MCUFRIEND_kbv* tft, CursorUtils* c) {
  _tft = tft;
  _c = c;
  _epoch = millis();
  _last_updated = millis();
}

DiscordStats::init() {
  _c->fillRect_safe(0, 162, 156, 76, WHITE);
}

bool DiscordStats::second_passed() {
  bool passed = false;

  if (millis() - _last_updated > 998) {
    passed = true;
  }

  return passed;
}

DiscordStats::printTime() {
  // https://forum.arduino.cc/t/sketch-to-convert-milliseconds-to-hours-minutes-and-seconds-hh-mm-ss/636386/4
  unsigned long currentMillis = millis() - _epoch;
  unsigned long seconds = currentMillis / 1000;
  unsigned long minutes = seconds / 60;
  unsigned long hours = minutes / 60;
  currentMillis %= 1000;
  seconds %= 60;
  minutes %= 60;

  if (hours < 10)
    _tft->print('0');
  _tft->print(hours);
  _tft->print(':');
  if (minutes < 10)
    _tft->print('0');
  _tft->print(minutes);
  _tft->print(':');
  if (seconds < 10)
    _tft->print('0');
  _tft->print(seconds);
}

DiscordStats::updateTimer() {
  if (second_passed()) {
    _last_updated = millis();
    //clear();
    _c->save_cursor_pos();
    _tft->setTextSize(3);
    _tft->setTextColor(BLACK, WHITE);
    _tft->setCursor(7, 188);
    printTime();
    _c->revert_cursor();
  }
}

DiscordStats::clear() {
  _c->fillRect_safe(0, 162, 156, 76, WHITE);
}