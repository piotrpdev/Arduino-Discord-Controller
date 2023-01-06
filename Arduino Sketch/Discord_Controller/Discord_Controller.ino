#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>
MCUFRIEND_kbv tft;
#include <TouchScreen.h>
#include "DiscordSerial.h"
#include "DiscordButton.h"

#define MINPRESSURE 200
#define MAXPRESSURE 1000

// ALL Touch panels and wiring is DIFFERENT
// copy-paste results from TouchScreen_Calibr_native.ino
const int XP = 8, XM = A2, YP = A3, YM = 9; //ID=0x9341
const int TS_LEFT = 74, TS_RT = 904, TS_TOP = 105, TS_BOT = 924;

TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

int press_cooldown = 200;
unsigned long last_pressed = 0;

char buffer[10];

bool py_connected = false;
int saved_cursor[2];
int not_connected_count = 0;
char* not_connected_dots[3] = { ".", "..", "..." }; 

DiscordButton mic_btn(&tft, 280, 2 + BTN_W * 0.5, 2 + 0, "MIC", COMMAND_MIC);
DiscordButton head_btn(&tft, 280, 6 + BTN_W * 1.5, 6 + BTN_W * 1, "HEAD", COMMAND_HEAD);
DiscordButton connected_btn(&tft, 280, 10 + BTN_W * 2.5, 10 + BTN_W * 2, "CONN", COMMAND_CONNECTED);

int pixel_x, pixel_y;     //Touch_getXY() updates global vars

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
    int oldCursor[2] = {tft.getCursorX(), tft.getCursorY()};

    mic_btn.init();
    head_btn.init();
    connected_btn.init();

    tft.setTextColor(WHITE);
    tft.setCursor(oldCursor[0], oldCursor[1]);
    tft.setTextSize(1);
    tft.println("Buttons initialized\n");

    tft.setTextColor(WHITE);
    tft.print("Waiting for py script");
    saved_cursor[0] = tft.getCursorX();
    saved_cursor[1] = tft.getCursorY();
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
    bool down = Touch_getXY();
    for (int i = 0 ; pb[i] != NULL; i++) {
        update_button(pb[i], down);
    }
    return down;
}

void loop()
{
    while (!py_connected) {
      if (not_connected_count < 3) {
        tft.setCursor(saved_cursor[0], saved_cursor[1]);
        tft.print(not_connected_dots[not_connected_count]);
        not_connected_count++;
      } else {
        tft.setCursor(saved_cursor[0], saved_cursor[1]);
        tft.setTextColor(BLACK);
        tft.print(not_connected_dots[2]);
        tft.setTextColor(WHITE);
        not_connected_count = 0;
      }

      if (Serial.available()) {
        py_connected = true;
      } else {
        Serial.print("<#P0>");
        delay(500);
      }
    }

    if (Serial.available()) {
      if (Serial.read() == '<') {
        int charsRead = Serial.readBytesUntil('>', buffer, sizeof(buffer) - 1);  // Get bytes up to sentinel
        
        if (buffer[0] == COMMAND_START) {
          switch (buffer[1]) {
            case COMMAND_PYTHON:
              if (buffer[2] - '0') {
                tft.println("\nPy script connected");
                tft.println("\nWaiting for Discord...");
              } else {
                tft.println("\nPy script disconnected");
                tft.print("Waiting for py script");
                saved_cursor[0] = tft.getCursorX();
                saved_cursor[1] = tft.getCursorY();
                py_connected = false;
              }
              break;
            case COMMAND_DISCORD:
              if (buffer[2] - '0') {
                tft.println("Discord connected");
                mic_btn.enable();
                head_btn.enable();
                connected_btn.enable();
              } else {
                tft.println("\nDsicord disconnected");
                tft.println("\nWaiting for Discord...");

                saved_cursor[0] = tft.getCursorX();
                saved_cursor[1] = tft.getCursorY();

                mic_btn.disable();
                head_btn.disable();
                connected_btn.disable();

                tft.setCursor(saved_cursor[0], saved_cursor[1]);
                tft.setTextSize(1);
                tft.setTextColor(WHITE);
              }
              break;
            case COMMAND_SYN:
              saved_cursor[0] = tft.getCursorX();
              saved_cursor[1] = tft.getCursorY();

              mic_btn.setState(buffer[2] - '0');
              head_btn.setState(buffer[3] - '0');
              connected_btn.setState(buffer[4] - '0');

              tft.setCursor(saved_cursor[0], saved_cursor[1]);
              tft.setTextSize(1);
              tft.setTextColor(WHITE);
              break;
            case COMMAND_VOICE_STATE_UPDATES:
              saved_cursor[0] = tft.getCursorX();
              saved_cursor[1] = tft.getCursorY();

              mic_btn.setState(buffer[2] - '0');
              head_btn.setState(buffer[3] - '0');
              connected_btn.setState(buffer[4] - '0');

              tft.setCursor(saved_cursor[0], saved_cursor[1]);
              tft.setTextSize(1);
              tft.setTextColor(WHITE);
              break;
            case COMMAND_AUDIO_TOGGLE:
              saved_cursor[0] = tft.getCursorX();
              saved_cursor[1] = tft.getCursorY();

              mic_btn.setState(buffer[2] - '0');
              head_btn.setState(buffer[3] - '0');

              tft.setCursor(saved_cursor[0], saved_cursor[1]);
              tft.setTextSize(1);
              tft.setTextColor(WHITE);
              break;
            case COMMAND_CONNECTED:
              saved_cursor[0] = tft.getCursorX();
              saved_cursor[1] = tft.getCursorY();

              connected_btn.setState(buffer[2] - '0');

              tft.setCursor(saved_cursor[0], saved_cursor[1]);
              tft.setTextSize(1);
              tft.setTextColor(WHITE);
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

