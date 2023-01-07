#include "DiscordButton.h"
#include "Serial.h"

#include "Arduino.h"
#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>
#include "CursorUtils.h"

DiscordButton::DiscordButton(MCUFRIEND_kbv* tft, CursorUtils* c, int16_t x, int16_t y, int16_t rectY, char * label, char type) {
  _tft = tft;
  _c = c;
  _x = x;
  _y = y;
  _label = label;
  _rectY = rectY;
  _type = type;
}

Adafruit_GFX_Button* DiscordButton::getBtn() {
  return &btn;
}

DiscordButton::init() {
  _c->save_cursor_pos();
  btn.initButton(_tft, _x, _y, BTN_W, BTN_W, WHITE, CYAN, BLACK, _label, 2);
  btn.drawButton(false);
  _c->revert_cursor();
  disable();
}

DiscordButton::enable() {
  _disabled = false;
}

DiscordButton::disable() {
  _disabled = true;
  _c->fillRect_safe(160, _rectY, BTN_W, BTN_W, YELLOW);
}

bool DiscordButton::getState() {
  return _state;
}

DiscordButton::setState(bool state) {
  if (_disabled) _disabled = false;
  _state = state;

  if (state) {
    _c->fillRect_safe(160, _rectY, BTN_W, BTN_W, GREEN);
  } else {
    _c->fillRect_safe(160, _rectY, BTN_W, BTN_W, RED);
  }
}

DiscordButton::issueCommand(char type, bool state) {
  Serial.print("<" + String(COMMAND_START) + String(type) + String(state) + ">");
}

DiscordButton::checkIfPressed() {
  if (btn.justPressed() && !_disabled) {
    if (_type == 'C' && !_state) return; // User cant just connect
    toggle();

    issueCommand(_type, _state);
  }
}

DiscordButton::toggle() {
  setState(!_state);
}

DiscordButton::on() {
  setState(1);
}

DiscordButton::off() {
  setState(0);
}