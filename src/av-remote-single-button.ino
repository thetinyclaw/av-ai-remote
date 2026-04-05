/*
 * AV AI Remote — Single Button Mode (Click / Double-Click)
 * 
 * M5StickC PLUS2 — Single accessible button control
 * - Single Click: Navigate menu item (cycle through commands)
 * - Double Click: Execute IR command
 * - Long Press: Go back / Exit menu
 * 
 * Hardware: M5StickC PLUS2 (ESP32-PICO-V3-02)
 * IR Pin: GPIO19
 * Button: GPIO35 (Button C)
 */

#define DISABLE_CODE_FOR_RECEIVER
#define SEND_PWM_BY_TIMER
#define IR_TX_PIN 19

#include "M5StickCPlus2.h"
#include <IRremote.hpp>

// ============================================================================
// IR CODE DATABASE (Samsung AA59 Remote)
// ============================================================================

const uint16_t SAMSUNG_ADDR = 0xE0E0;

const uint8_t SAM_POWER   = 0xBF;
const uint8_t SAM_VOL_UP  = 0x1F;
const uint8_t SAM_VOL_DN  = 0x2F;
const uint8_t SAM_CH_UP   = 0xB7;
const uint8_t SAM_CH_DN   = 0xF7;
const uint8_t SAM_MUTE    = 0x0F;
const uint8_t SAM_MENU    = 0xA7;
const uint8_t SAM_ENTER   = 0xE9;
const uint8_t SAM_UP      = 0xF9;
const uint8_t SAM_DOWN    = 0x79;
const uint8_t SAM_LEFT    = 0x59;
const uint8_t SAM_RIGHT   = 0xB9;
const uint8_t SAM_RETURN  = 0xE5;
const uint8_t SAM_SOURCE  = 0x7F;
const uint8_t SAM_INFO    = 0x07;
const uint8_t SAM_GUIDE   = 0x0D;

// TV Command list (in cycle order)
struct TVCommand {
  const char* label;
  uint8_t code;
};

const TVCommand tvCommands[] = {
  {"Power", SAM_POWER},
  {"Vol +", SAM_VOL_UP},
  {"Vol -", SAM_VOL_DN},
  {"Ch +", SAM_CH_UP},
  {"Ch -", SAM_CH_DN},
  {"Mute", SAM_MUTE},
  {"Menu", SAM_MENU},
  {"Source", SAM_SOURCE},
  {"Info", SAM_INFO}
};
const uint8_t NUM_COMMANDS = sizeof(tvCommands) / sizeof(tvCommands[0]);

// ============================================================================
// UI STATE
// ============================================================================

uint8_t currentCommand = 0;  // Index into tvCommands[]
unsigned long lastClickTime = 0;
const unsigned long DOUBLE_CLICK_TIMEOUT = 300;  // ms

// ============================================================================
// DISPLAY HELPERS
// ============================================================================

void displayInit() {
  StickCP2.Display.setRotation(1);
  StickCP2.Display.setTextColor(TFT_GREEN);
  StickCP2.Display.setTextDatum(middle_center);
  StickCP2.Display.setTextSize(1);
}

void clearDisplay() {
  StickCP2.Display.fillScreen(TFT_BLACK);
}

void drawHeader(const char* title) {
  StickCP2.Display.setTextColor(TFT_WHITE);
  StickCP2.Display.drawString(title, 67, 15);
  StickCP2.Display.drawLine(0, 25, 135, 25, TFT_GREY);
}

void renderCommandMenu() {
  clearDisplay();
  drawHeader("TV COMMAND");

  // Current command (large, center)
  StickCP2.Display.setTextColor(TFT_CYAN);
  StickCP2.Display.setTextFont(&fonts::Orbitron_Light_24);
  StickCP2.Display.drawString(tvCommands[currentCommand].label, 67, 80);
  StickCP2.Display.setTextFont(nullptr);

  // Previous command (small, top)
  if (currentCommand > 0) {
    StickCP2.Display.setTextColor(TFT_DARKGREY);
    StickCP2.Display.setTextSize(1);
    StickCP2.Display.drawString(tvCommands[currentCommand - 1].label, 67, 50);
  }

  // Next command (small, bottom)
  if (currentCommand < NUM_COMMANDS - 1) {
    StickCP2.Display.setTextColor(TFT_DARKGREY);
    StickCP2.Display.setTextSize(1);
    StickCP2.Display.drawString(tvCommands[currentCommand + 1].label, 67, 110);
  }

  // Instructions
  StickCP2.Display.setTextColor(TFT_YELLOW);
  StickCP2.Display.setTextSize(1);
  StickCP2.Display.drawString("Click: Next | Double: Send", 67, 200);
  StickCP2.Display.drawString("Hold: Back", 67, 215);

  // Indicator
  StickCP2.Display.setTextColor(TFT_GREEN);
  char indicator[16];
  snprintf(indicator, sizeof(indicator), "%d/%d", currentCommand + 1, NUM_COMMANDS);
  StickCP2.Display.drawString(indicator, 67, 135);
}

// ============================================================================
// IR TRANSMISSION
// ============================================================================

void sendIRCode(uint16_t addr, uint8_t cmd) {
  Serial.printf("Sending: %s (0x%04X, 0x%02X)\n", 
                tvCommands[currentCommand].label, addr, cmd);
  
  // Flash LED feedback
  StickCP2.Display.fillCircle(130, 20, 5, TFT_RED);
  delay(100);
  StickCP2.Display.fillCircle(130, 20, 5, TFT_BLACK);
  
  IrSender.sendNEC(addr, cmd, 0);
  delay(100);
}

// ============================================================================
// BUTTON HANDLING (Single Button, Click/Double-Click)
// ============================================================================

void handleButtonPress() {
  // Check Button C (power/user button)
  if (StickCP2.BtnC.wasPressed()) {
    unsigned long now = millis();
    unsigned long timeSinceLastClick = now - lastClickTime;
    
    Serial.printf("Button pressed. Time since last: %lu ms\n", timeSinceLastClick);

    if (timeSinceLastClick < DOUBLE_CLICK_TIMEOUT) {
      // DOUBLE CLICK → Execute command
      Serial.println("Double click detected!");
      sendIRCode(SAMSUNG_ADDR, tvCommands[currentCommand].code);
    } else {
      // SINGLE CLICK → Navigate next
      Serial.println("Single click detected!");
      currentCommand = (currentCommand + 1) % NUM_COMMANDS;
    }

    lastClickTime = now;
  }

  // Long press detection (held for 1+ second)
  if (StickCP2.BtnC.isPressed() && StickCP2.BtnC.pressedFor(1000)) {
    // Go back to home (cycle backwards)
    Serial.println("Long press detected!");
    if (currentCommand > 0) {
      currentCommand--;
    }
  }
}

// ============================================================================
// SETUP & LOOP
// ============================================================================

void setup() {
  Serial.begin(115200);
  delay(500);

  auto cfg = M5.config();
  StickCP2.begin(cfg);
  displayInit();

  // Initialize IR sender
  IrSender.begin(DISABLE_LED_FEEDBACK);
  IrSender.setSendPin(IR_TX_PIN);

  Serial.println("AV AI Remote (Single Button Mode) initialized!");
  Serial.println("Click: Next | Double-Click: Send IR | Hold: Back");
}

void loop() {
  StickCP2.update();
  handleButtonPress();

  // Render current state
  renderCommandMenu();

  delay(50);
}
