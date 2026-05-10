#include <Arduino.h>
#include <M5Unified.h>

#define IR_SEND_PIN 19
#include <IRremote.hpp>

struct Command {
  const char* name;
  uint16_t address;
  uint8_t command;
};

// Samsung-style NEC/extended NEC commands. Good first target for validating TX.
static const Command COMMANDS[] = {
  {"Samsung Power", 0xE0E0, 0xBF},
  {"Samsung Vol+",  0xE0E0, 0x1F},
  {"Samsung Vol-",  0xE0E0, 0x2F},
  {"Samsung Mute",  0xE0E0, 0x0F},
  {"Samsung Source",0xE0E0, 0x7F},
  {"Samsung Up",    0xE0E0, 0xF9},
  {"Samsung Down",  0xE0E0, 0x79},
  {"Samsung Left",  0xE0E0, 0x59},
  {"Samsung Right", 0xE0E0, 0xB9},
  {"Samsung OK",    0xE0E0, 0xE9},
  {"Samsung Back",  0xE0E0, 0xE5},
};
static const size_t COMMAND_COUNT = sizeof(COMMANDS) / sizeof(COMMANDS[0]);

size_t selected = 0;
uint32_t lastDraw = 0;
uint32_t lastSend = 0;

void drawUi(const char* status = nullptr) {
  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.setRotation(1);
  M5.Display.setTextDatum(top_center);

  M5.Display.setTextColor(TFT_ORANGE, TFT_BLACK);
  M5.Display.setTextSize(1);
  M5.Display.drawString("M5 IR BLASTER", M5.Display.width() / 2, 4);
  M5.Display.drawLine(0, 20, M5.Display.width(), 20, TFT_DARKGREY);

  const Command& c = COMMANDS[selected];
  M5.Display.setTextColor(TFT_CYAN, TFT_BLACK);
  M5.Display.setTextSize(2);
  M5.Display.drawString(c.name, M5.Display.width() / 2, 34);

  char buf[64];
  snprintf(buf, sizeof(buf), "addr 0x%04X cmd 0x%02X", c.address, c.command);
  M5.Display.setTextColor(TFT_GREEN, TFT_BLACK);
  M5.Display.setTextSize(1);
  M5.Display.drawString(buf, M5.Display.width() / 2, 72);

  snprintf(buf, sizeof(buf), "%u/%u", (unsigned)(selected + 1), (unsigned)COMMAND_COUNT);
  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Display.drawString(buf, M5.Display.width() / 2, 92);

  M5.Display.setTextColor(TFT_YELLOW, TFT_BLACK);
  M5.Display.drawString("A/B: choose   C: blast", M5.Display.width() / 2, 112);
  M5.Display.drawString("Serial: n p s 0-9", M5.Display.width() / 2, 126);

  if (status) {
    M5.Display.setTextColor(TFT_RED, TFT_BLACK);
    M5.Display.drawString(status, M5.Display.width() / 2, 144);
  }
}

void sendSelected() {
  const Command& c = COMMANDS[selected];
  Serial.printf("BLAST %s addr=0x%04X cmd=0x%02X pin=%d\n", c.name, c.address, c.command, IR_SEND_PIN);
  drawUi("BLAST!");

  // Send three times for reliability; many remotes repeat while held.
  for (int i = 0; i < 3; i++) {
    IrSender.sendNEC(c.address, c.command, 0);
    delay(90);
  }
  lastSend = millis();
}

void nextCommand() {
  selected = (selected + 1) % COMMAND_COUNT;
  drawUi();
  Serial.printf("SELECT %u: %s\n", (unsigned)selected, COMMANDS[selected].name);
}

void prevCommand() {
  selected = (selected + COMMAND_COUNT - 1) % COMMAND_COUNT;
  drawUi();
  Serial.printf("SELECT %u: %s\n", (unsigned)selected, COMMANDS[selected].name);
}

void handleSerial() {
  while (Serial.available()) {
    char ch = Serial.read();
    if (ch == 'n' || ch == '+') nextCommand();
    else if (ch == 'p' || ch == '-') prevCommand();
    else if (ch == 's' || ch == 'b' || ch == '\n') sendSelected();
    else if (ch >= '0' && ch <= '9') {
      size_t idx = (size_t)(ch - '0');
      if (idx < COMMAND_COUNT) {
        selected = idx;
        drawUi();
        Serial.printf("SELECT %u: %s\n", (unsigned)selected, COMMANDS[selected].name);
      }
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(300);

  auto cfg = M5.config();
  M5.begin(cfg);
  M5.Display.setBrightness(180);

  IrSender.begin();

  Serial.println();
  Serial.println("M5 IR Blaster ready");
  Serial.printf("IR TX pin: GPIO%d\n", IR_SEND_PIN);
  Serial.println("Buttons: A/B choose, C blasts. Serial: n next, p prev, s send, 0-9 select.");
  drawUi("READY");
}

void loop() {
  M5.update();
  handleSerial();

  if (M5.BtnA.wasPressed()) nextCommand();
  if (M5.BtnB.wasPressed()) prevCommand();
  if (M5.BtnC.wasPressed()) sendSelected();

  if (lastSend && millis() - lastSend > 600) {
    lastSend = 0;
    drawUi();
  }

  if (millis() - lastDraw > 5000) {
    lastDraw = millis();
    Serial.printf("alive selected=%u %s\n", (unsigned)selected, COMMANDS[selected].name);
  }
  delay(10);
}
