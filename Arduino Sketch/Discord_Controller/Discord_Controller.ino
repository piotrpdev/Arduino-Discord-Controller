#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>
MCUFRIEND_kbv tft;
#include <TouchScreen.h>
#include "CursorUtils.h"
#include "Serial.h"
#include "DiscordButton.h"
#include "DiscordStats.h"

#define MINPRESSURE 200
#define MAXPRESSURE 1000

// ALL Touch panels and wiring is DIFFERENT
// copy-paste results from TouchScreen_Calibr_native.ino
const int XP = 8, XM = A2, YP = A3, YM = 9; //ID=0x9341
const int TS_LEFT = 74, TS_RT = 904, TS_TOP = 105, TS_BOT = 924;

TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

CursorUtils c = CursorUtils(&tft);

char buffer[10];

DiscordButton mic_btn(&tft, &c, 280, 2 + BTN_W * 0.5, 2 + 0, "MIC", COMMAND_MIC);
DiscordButton head_btn(&tft, &c, 280, 6 + BTN_W * 1.5, 6 + BTN_W * 1, "HEAD", COMMAND_HEAD);
DiscordButton connected_btn(&tft, &c, 280, 10 + BTN_W * 2.5, 10 + BTN_W * 2, "CONN", COMMAND_CONNECTED);

DiscordStats stats(&tft, &c);
bool timer_enabled = false;

int discord_cursor[2] = {0, 0};

int pixel_x, pixel_y;     //Touch_getXY() updates global vars

int press_cooldown = 200;
unsigned long last_pressed = 0;

bool Touch_getXY(void)
{
    TSPoint p = ts.getPoint();
    pinMode(YP, OUTPUT);      //restore shared pins
    pinMode(XM, OUTPUT);
    digitalWrite(YP, HIGH);   //because TFT control pins
    digitalWrite(XM, HIGH);

    bool pressed = (p.z > MINPRESSURE && p.z < MAXPRESSURE) && millis() - last_pressed > press_cooldown;

    if (pressed) {
        last_pressed = millis();
        pixel_x = map(p.y, TS_RT, TS_LEFT, 0, tft.width()); //.kbv makes sense to me
        pixel_y = map(p.x, TS_BOT, TS_TOP, 0, tft.height());
    }

    return pressed;
}

void setup()
{
    uint16_t ID = tft.readID();

    tft.begin(ID);
    tft.setRotation(3);
    tft.fillScreen(BLACK);

    tft.println("TFT initialized");
    tft.println("\nWaiting for Serial...");
    Serial.begin(9600);
    while (!Serial);
    tft.println("Serial connected");

    tft.println("\nInitializing buttons...");

    mic_btn.init();
    head_btn.init();
    connected_btn.init();

    stats.init();

    tft.println("Buttons initialized");
}

Adafruit_GFX_Button *buttons[] = {mic_btn.getBtn(), head_btn.getBtn(), connected_btn.getBtn(), NULL};

bool update_button(Adafruit_GFX_Button *b, bool down)
{
    b->press(down && b->contains(pixel_x, pixel_y));
    if (b->justReleased())
        b->drawButton(false);
    if (b->justPressed())
        b->drawButton(true);
    return down;
}

bool update_button_list(Adafruit_GFX_Button **pb)
{
    c.save_cursor_pos();
    bool down = Touch_getXY();
    for (int i = 0 ; pb[i] != NULL; i++) {
        update_button(pb[i], down);
    }
    c.revert_cursor();
    return down;
}

bool py_connected = false;

void wait_for_py() {
  tft.print("\nWaiting for py script");
  c.save_cursor_pos();

  c.dotLoop(&py_connected, 500, true);
}

bool discord_connected = false;

void wait_for_discord() {
  if (discord_cursor[1] == 0) {
    discord_cursor[0] = tft.getCursorX();
    discord_cursor[1] = tft.getCursorY();
  } 
  tft.print("\nWaiting for Discord");
  c.save_cursor_pos();

  c.dotLoop(&discord_connected, 500);
}

bool _state;

void loop()
{
    if (!py_connected) {
      wait_for_py();
    } else if (!discord_connected) {
      wait_for_discord();
    }

    if (timer_enabled) {
      stats.updateTimer();
    }

    if (Serial.available()) {
      if (Serial.read() == PROTOCOL_START) {
        int charsRead = Serial.readBytesUntil(PROTOCOL_END, buffer, sizeof(buffer) - 1);
        
        if (buffer[0] == COMMAND_START) {
          switch (buffer[1]) {
            case COMMAND_PYTHON:
              if (buffer[2] - '0') {
                tft.println("\nPy script connected");
              } else {
                tft.println("\nPy script disconnected");
                py_connected = false;
                discord_connected = false;
                timer_enabled = false;

                mic_btn.disable();
                head_btn.disable();
                connected_btn.disable();
              }
              break;
            case COMMAND_DISCORD:
              if (buffer[2] - '0') {
                tft.println("\nDiscord connected");
                mic_btn.enable();
                head_btn.enable();
                connected_btn.enable();
              } else {
                if (!(discord_cursor[1] == 0)) {
                  c.fillRect_safe(discord_cursor[0], discord_cursor[1] + 3, 156, 76, BLACK);
                  tft.setCursor(discord_cursor[0], discord_cursor[1]);
                }
                tft.println("\nDiscord disconnected");
                discord_connected = false;
                timer_enabled = false;

                mic_btn.disable();
                head_btn.disable();
                connected_btn.disable();
              }
              break;
            case COMMAND_SYN:
              mic_btn.setState(buffer[2] - '0');
              head_btn.setState(buffer[3] - '0');
              connected_btn.setState(buffer[4] - '0');

              _state = buffer[4] - '0';

              if (_state) {
                timer_enabled = true;
                stats._epoch = millis();
              } else {
                timer_enabled = false;
              }
              break;
            case COMMAND_VOICE_STATE_UPDATES:
              mic_btn.setState(buffer[2] - '0');
              head_btn.setState(buffer[3] - '0');
              connected_btn.setState(buffer[4] - '0');
              break;
            case COMMAND_AUDIO_TOGGLE:
              mic_btn.setState(buffer[2] - '0');
              head_btn.setState(buffer[3] - '0');
              break;
            case COMMAND_CONNECTED:
              _state = buffer[2] - '0';
              if (_state) {
                timer_enabled = true;
                stats._epoch = millis();
              } else {
                timer_enabled = false;
              }

              connected_btn.setState(_state);
              break;
            default:
              break;
          }
        }
      }
    }

    update_button_list(buttons);
    
    mic_btn.checkIfPressed();
    head_btn.checkIfPressed();
    connected_btn.checkIfPressed();
}

