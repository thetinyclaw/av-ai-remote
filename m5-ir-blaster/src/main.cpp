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
  {"Power",       0xFF02FD},
  {"Bright +",    0xFF3AC5},
  {"Bright -",    0xFFBA45},
  {"Play/Pause",  0xFF827D},
  {"Red",         0xFF1AE5},
  {"Green",       0xFF9A65},
  {"Blue",        0xFFA25D},
  {"White",       0xFF22DD},
  {"Orange",      0xFF2AD5},
  {"Lime",        0xFFAA55},
  {"Cyan",        0xFF926D},
  {"Warm White",  0xFF12ED},
  {"Yellow",      0xFF0AF5},
  {"Aqua",        0xFF8A75},
  {"Purple",      0xFFB24D},
  {"Pink",        0xFF32CD},
  {"Jump3",       0xFFB04F},
  {"Jump7",       0xFF30CF},
  {"Fade3",       0xFF708F},
  {"Fade7",       0xFFF00F},
  {"Flash",       0xFFD02F},
  {"Auto",        0xFFE01F},
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
  snprintf(buf, sizeof(buf), "NEC raw 0x%06lX", (unsigned long)c.raw);
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
  Serial.printf("BLAST LED %s raw=0x%06lX pin=%d\n", c.name, (unsigned long)c.raw, IR_SEND_PIN);
  drawUi("BLAST!");

  // Send repeated full frames so finicky LED controllers have a better chance to catch it.
  for (int i = 0; i < 5; i++) {
    IrSender.sendNECRaw(c.raw, 0);
    delay(70);
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
