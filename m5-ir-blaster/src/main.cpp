#include <Arduino.h>
#include <M5Unified.h>

#define IR_SEND_PIN 19
#include <IRremote.hpp>

struct Command {
  const char* name;
  uint32_t raw;
};

// Alec's LED strip receiver: confirmed EF24 family.
// Resolved as a 24-key RGB remote grid, numbered left-to-right/top-to-bottom.
static const Command COMMANDS[] = {
  {"Bright +",    0x00F700FF},
  {"Bright -",    0x00F7807F},
  {"Off",         0x00F740BF},
  {"On",          0x00F7C03F},
  {"Red",         0x00F720DF},
  {"Green",       0x00F7A05F},
  {"Blue",        0x00F7609F},
  {"White",       0x00F7E01F},
  {"Orange",      0x00F710EF},
  {"Mint",        0x00F7906F},
  {"Royal Blue",  0x00F750AF},
  {"Warm White",  0x00F7D02F},
  {"Yellow",      0x00F730CF},
  {"Cyan",        0x00F7B04F},
  {"Purple",      0x00F7708F},
  {"Strobe",      0x00F7F00F},
  {"Amber",       0x00F708F7},
  {"Yellow",      0x00F78877},
  {"Cyan",        0x00F748B7},
  {"Purple",      0x00F7C837},
  {"Amber",       0x00F728D7},
  {"Aqua",        0x00F7A857},
  {"Magenta",     0x00F76897},
  {"Smooth",      0x00F7E817},
};
static const size_t COMMAND_COUNT = sizeof(COMMANDS) / sizeof(COMMANDS[0]);

size_t selected = 0;
uint32_t lastDraw = 0;
uint32_t lastSend = 0;

void drawUi(const char* status = nullptr) {
  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.setRotation(3);
  M5.Display.setTextDatum(top_center);

  M5.Display.setTextColor(TFT_MAGENTA, TFT_BLACK);
  M5.Display.setTextSize(1);
  M5.Display.drawString("LED STRIP 24-KEY GRID", M5.Display.width() / 2, 4);
  M5.Display.drawLine(0, 20, M5.Display.width(), 20, TFT_DARKGREY);

  const Command& c = COMMANDS[selected];
  char buf[64];
  snprintf(buf, sizeof(buf), "%02u  %s", (unsigned)selected, c.name);
  M5.Display.setTextColor(TFT_CYAN, TFT_BLACK);
  M5.Display.setTextSize(2);
  M5.Display.drawString(buf, M5.Display.width() / 2, 34);

  snprintf(buf, sizeof(buf), "NEC 0x%08lX", (unsigned long)c.raw);
  M5.Display.setTextColor(TFT_GREEN, TFT_BLACK);
  M5.Display.setTextSize(1);
  M5.Display.drawString(buf, M5.Display.width() / 2, 72);

  snprintf(buf, sizeof(buf), "EF24 #%02u  R%uC%u  %u/%u", (unsigned)selected, (unsigned)(selected / 4 + 1), (unsigned)(selected % 4 + 1), (unsigned)(selected + 1), (unsigned)COMMAND_COUNT);
  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Display.drawString(buf, M5.Display.width() / 2, 92);

  M5.Display.setTextColor(TFT_YELLOW, TFT_BLACK);
  M5.Display.drawString("B/C: choose   A: blast", M5.Display.width() / 2, 112);
  M5.Display.drawString("Serial: n p s 0-9", M5.Display.width() / 2, 126);
  M5.Display.drawString("24-key grid: L->R, top->bottom", M5.Display.width() / 2, 140);

  if (status) {
    M5.Display.setTextColor(TFT_RED, TFT_BLACK);
    M5.Display.drawString(status, M5.Display.width() / 2, 154);
  }
}

void sendSelected() {
  const Command& c = COMMANDS[selected];
  uint8_t address = (uint8_t)((c.raw >> 24) & 0xFF);
  uint8_t command = (uint8_t)((c.raw >> 8) & 0xFF);
  Serial.printf("BLAST LED %s raw=0x%08lX addr=0x%02X cmd=0x%02X pin=%d\n",
                c.name, (unsigned long)c.raw, address, command, IR_SEND_PIN);
  drawUi("TRI-BLAST!");

  // Cheap LED receivers are inconsistent about NEC byte/bit ordering in published code tables.
  // Send all common interpretations: modern address/command, old MSB 32-bit, and raw LSB.
  for (int i = 0; i < 2; i++) {
    IrSender.sendNEC(address, command, 0);
    delay(75);
    IrSender.sendNECMSB(c.raw, 32, false);
    delay(75);
    IrSender.sendNECRaw(c.raw, 0);
    delay(75);
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
  Serial.println("M5 LED Strip 24-key grid ready - EF24 confirmed");
  Serial.printf("IR TX pin: GPIO%d\n", IR_SEND_PIN);
  Serial.println("Buttons: B/C choose, A blasts. Serial: n next, p prev, s send, 0-9 select.");
  drawUi("READY");
}

void loop() {
  M5.update();
  handleSerial();

  if (M5.BtnA.wasPressed()) sendSelected();
  if (M5.BtnB.wasPressed()) prevCommand();
  if (M5.BtnC.wasPressed()) nextCommand();

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
