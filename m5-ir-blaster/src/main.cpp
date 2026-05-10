#include <Arduino.h>
#include <M5Unified.h>

#define IR_SEND_PIN 19
#include <IRremote.hpp>

struct Command {
  const char* name;
  uint32_t raw;
};

// Common cheap RGB LED strip remote codes.
// These are the ubiquitous NEC-style 24/44-key IR controller codes using 0xFFxx.. frames.
// If Alec's strip uses a different receiver, we can add a learn/sniff mode later.
static const Command COMMANDS[] = {
  {"Power",       0x00FF02FD},
  {"Bright +",    0x00FF3AC5},
  {"Bright -",    0x00FFBA45},
  {"Play/Pause",  0x00FF827D},
  {"Red",         0x00FF1AE5},
  {"Green",       0x00FF9A65},
  {"Blue",        0x00FFA25D},
  {"White",       0x00FF22DD},
  {"Orange",      0x00FF2AD5},
  {"Lime",        0x00FFAA55},
  {"Cyan",        0x00FF926D},
  {"Warm White",  0x00FF12ED},
  {"Yellow",      0x00FF0AF5},
  {"Aqua",        0x00FF8A75},
  {"Purple",      0x00FFB24D},
  {"Pink",        0x00FF32CD},
  {"Jump3",       0x00FFB04F},
  {"Jump7",       0x00FF30CF},
  {"Fade3",       0x00FF708F},
  {"Fade7",       0x00FFF00F},
  {"Flash",       0x00FFD02F},
  {"Auto",        0x00FFE01F},
};
static const size_t COMMAND_COUNT = sizeof(COMMANDS) / sizeof(COMMANDS[0]);

size_t selected = 0;
uint32_t lastDraw = 0;
uint32_t lastSend = 0;

void drawUi(const char* status = nullptr) {
  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.setRotation(1);
  M5.Display.setTextDatum(top_center);

  M5.Display.setTextColor(TFT_MAGENTA, TFT_BLACK);
  M5.Display.setTextSize(1);
  M5.Display.drawString("LED STRIP IR REMOTE", M5.Display.width() / 2, 4);
  M5.Display.drawLine(0, 20, M5.Display.width(), 20, TFT_DARKGREY);

  const Command& c = COMMANDS[selected];
  M5.Display.setTextColor(TFT_CYAN, TFT_BLACK);
  M5.Display.setTextSize(2);
  M5.Display.drawString(c.name, M5.Display.width() / 2, 34);

  char buf[64];
  snprintf(buf, sizeof(buf), "NEC 0x%08lX", (unsigned long)c.raw);
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
  Serial.println("M5 LED Strip IR Blaster ready");
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
