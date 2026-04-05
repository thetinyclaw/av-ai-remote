/*
 * AV AI Remote — IR Blaster Sketch for M5StickC PLUS2
 * 
 * Path 1: Quick Prototype - THREE BUTTON MODE
 * - Hardcoded NEC IR codes for Samsung TV
 * - Menu UI on display
 * - Button-driven control (A=up, B=down, C=select)
 * 
 * Hardware: M5StickC PLUS2 (ESP32-PICO-V3-02)
 * IR Pin: GPIO19
 * 
 * Libraries:
 *   - M5StickCPlus2 (M5Stack)
 *   - Arduino-IRremote (Arduino-IRremote)
 */

#define DISABLE_CODE_FOR_RECEIVER  // Disable receiver code to save memory
#define SEND_PWM_BY_TIMER
#define IR_TX_PIN 19

#include "M5StickCPlus2.h"
#include <IRremote.hpp>

// ============================================================================
// IR CODE DATABASE (Samsung AA59 Remote - 32-bit format)
// Protocol: Samsung (extended NEC variant)
// Address: 0xE0E0 (Samsung TV address)
// Command: See below
// ============================================================================

const uint16_t SAMSUNG_ADDR = 0xE0E0;

// Samsung TV Commands (8-bit)
const uint8_t SAM_POWER   = 0xBF;  // Standby toggle
const uint8_t SAM_VOL_UP  = 0x1F;  // Volume up
const uint8_t SAM_VOL_DN  = 0x2F;  // Volume down
const uint8_t SAM_CH_UP   = 0xB7;  // Channel up
const uint8_t SAM_CH_DN   = 0xF7;  // Channel down
const uint8_t SAM_MUTE    = 0x0F;  // Mute toggle
const uint8_t SAM_MENU    = 0xA7;  // Menu
const uint8_t SAM_ENTER   = 0xE9;  // Enter/OK
const uint8_t SAM_UP      = 0xF9;  // Up arrow
const uint8_t SAM_DOWN    = 0x79;  // Down arrow
const uint8_t SAM_LEFT    = 0x59;  // Left arrow
const uint8_t SAM_RIGHT   = 0xB9;  // Right arrow
const uint8_t SAM_RETURN  = 0xE5;  // Return/Back
const uint8_t SAM_SOURCE  = 0x7F;  // Source/Input
const uint8_t SAM_INFO    = 0x07;  // Info
const uint8_t SAM_GUIDE   = 0x0D;  // Guide

// ============================================================================
// UI STATE
// ============================================================================

enum MenuState {
  MENU_MAIN,
  MENU_TV,
  MENU_SOUNDBAR
};

MenuState currentMenu = MENU_MAIN;
uint8_t selectedItem = 0;

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
  StickCP2.Display.drawLine(0, 25, 135, 25, TFT_DARKGREY);
}

void drawText(const char* text, uint16_t x, uint16_t y, uint16_t color = TFT_GREEN) {
  StickCP2.Display.setTextColor(color);
  StickCP2.Display.drawString(text, x, y);
}

void drawHighlight(uint8_t item, uint16_t y) {
  StickCP2.Display.fillRect(5, y - 10, 125, 20, TFT_DARKGREEN);
  StickCP2.Display.setTextColor(TFT_WHITE);
}

// ============================================================================
// MENU RENDERING
// ============================================================================

void renderMainMenu() {
  clearDisplay();
  drawHeader("AV REMOTE");

  if (selectedItem == 0) {
    drawHighlight(0, 50);
    drawText("Samsung TV", 67, 50);
    drawText("TV Control", 67, 80, TFT_GREEN);
  } else {
    drawText("Samsung TV", 67, 50, TFT_GREEN);
    drawHighlight(1, 80);
    drawText("TV Control", 67, 80);
  }

  drawText("[C] Select", 67, 110, TFT_YELLOW);
}

void renderTVMenu() {
  clearDisplay();
  drawHeader("SAMSUNG TV");

  const char* items[] = {
    "Power",    // 0
    "Vol +",    // 1
    "Vol -",    // 2
    "Ch +",     // 3
    "Ch -",     // 4
    "Mute",     // 5
    "Menu",     // 6
    "Source",   // 7
    "Info",     // 8
    "Back"      // 9
  };
  uint8_t numItems = 10;

  for (uint8_t i = 0; i < numItems; i++) {
    uint16_t y = 35 + (i * 18);
    if (y > 235) continue;  // Skip off-screen items

    if (i == selectedItem) {
      drawHighlight(i, y);
      drawText(items[i], 67, y);
    } else {
      drawText(items[i], 67, y, TFT_GREEN);
    }
  }
}

void renderSoundbarMenu() {
  clearDisplay();
  drawHeader("SOUNDBAR");
  drawText("Coming soon...", 67, 120, TFT_YELLOW);
}

// ============================================================================
// IR TRANSMISSION
// ============================================================================

void sendIRCode(uint16_t addr, uint8_t cmd) {
  Serial.printf("Sending NEC: addr=0x%04X, cmd=0x%02X\n", addr, cmd);
  IrSender.sendNEC(addr, cmd, 0);  // 0 repeats = single send
  delay(100);  // Safety delay before next send
}

void handleTVCommand(uint8_t item) {
  switch (item) {
    case 0: sendIRCode(SAMSUNG_ADDR, SAM_POWER); break;
    case 1: sendIRCode(SAMSUNG_ADDR, SAM_VOL_UP); break;
    case 2: sendIRCode(SAMSUNG_ADDR, SAM_VOL_DN); break;
    case 3: sendIRCode(SAMSUNG_ADDR, SAM_CH_UP); break;
    case 4: sendIRCode(SAMSUNG_ADDR, SAM_CH_DN); break;
    case 5: sendIRCode(SAMSUNG_ADDR, SAM_MUTE); break;
    case 6: sendIRCode(SAMSUNG_ADDR, SAM_MENU); break;
    case 7: sendIRCode(SAMSUNG_ADDR, SAM_SOURCE); break;
    case 8: sendIRCode(SAMSUNG_ADDR, SAM_INFO); break;
    case 9: currentMenu = MENU_MAIN; selectedItem = 0; break;  // Back
  }
}

void handleSoundbarCommand(uint8_t item) {
  // TODO: Add soundbar codes
}

// ============================================================================
// BUTTON HANDLING
// ============================================================================

void handleButtonPress() {
  if (StickCP2.BtnA.wasPressed()) {
    // Button A: Navigate up
    if (selectedItem > 0) selectedItem--;
  }

  if (StickCP2.BtnB.wasPressed()) {
    // Button B: Navigate down
    uint8_t maxItems = 0;
    if (currentMenu == MENU_MAIN) maxItems = 2;
    else if (currentMenu == MENU_TV) maxItems = 10;
    else if (currentMenu == MENU_SOUNDBAR) maxItems = 4;

    if (selectedItem < maxItems - 1) selectedItem++;
  }

  if (StickCP2.BtnC.wasPressed()) {
    // Button C: Select / Go back
    if (currentMenu == MENU_MAIN) {
      if (selectedItem == 0) {
        currentMenu = MENU_TV;
        selectedItem = 0;
      } else if (selectedItem == 1) {
        currentMenu = MENU_SOUNDBAR;
        selectedItem = 0;
      }
    } else if (currentMenu == MENU_TV) {
      handleTVCommand(selectedItem);
    } else if (currentMenu == MENU_SOUNDBAR) {
      handleSoundbarCommand(selectedItem);
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

  Serial.println("AV AI Remote (THREE BUTTON MODE) initialized!");
  Serial.println("Button A: Up | Button B: Down | Button C: Select/Back");
}

void loop() {
  StickCP2.update();
  handleButtonPress();

  // Render current menu
  switch (currentMenu) {
    case MENU_MAIN:
      renderMainMenu();
      break;
    case MENU_TV:
      renderTVMenu();
      break;
    case MENU_SOUNDBAR:
      renderSoundbarMenu();
      break;
  }

  delay(50);  // Debounce & refresh rate
}
