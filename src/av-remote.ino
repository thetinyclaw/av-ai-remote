/*
 * AV AI Remote — IR Blaster Sketch for M5StickC PLUS2
 * 
 * Path 1: Quick Prototype
 * - Hardcoded NEC IR codes for common AV devices
 * - Menu UI on display
 * - Button-driven control
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
// IR CODE DATABASE (NEC Protocol: Address, Command)
// ============================================================================

// Common TV codes (example: LG TV)
const uint16_t TV_ADDR = 0x04FB;
const uint8_t TV_POWER  = 0x40;
const uint8_t TV_VOL_UP = 0x18;
const uint8_t TV_VOL_DN = 0x19;
const uint8_t TV_CH_UP  = 0x00;
const uint8_t TV_CH_DN  = 0x01;
const uint8_t TV_MUTE   = 0x65;

// Soundbar codes (example: generic)
const uint16_t SB_ADDR = 0x0000;
const uint8_t SB_POWER = 0x00;
const uint8_t SB_VOL_UP = 0x01;
const uint8_t SB_VOL_DN = 0x02;

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
  StickCP2.Display.drawLine(0, 25, 135, 25, TFT_GREY);
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
    drawText("TV", 67, 50);
    drawText("Soundbar", 67, 80, TFT_GREEN);
  } else {
    drawText("TV", 67, 50, TFT_GREEN);
    drawHighlight(1, 80);
    drawText("Soundbar", 67, 80);
  }

  drawText("[C] Select", 67, 110, TFT_YELLOW);
}

void renderTVMenu() {
  clearDisplay();
  drawHeader("TV CONTROL");

  const char* items[] = {"Power", "Vol +", "Vol -", "Ch +", "Ch -", "Mute", "Back"};
  uint8_t numItems = 7;

  for (uint8_t i = 0; i < numItems; i++) {
    uint16_t y = 40 + (i * 20);

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

  const char* items[] = {"Power", "Vol +", "Vol -", "Back"};
  uint8_t numItems = 4;

  for (uint8_t i = 0; i < numItems; i++) {
    uint16_t y = 50 + (i * 25);

    if (i == selectedItem) {
      drawHighlight(i, y);
      drawText(items[i], 67, y);
    } else {
      drawText(items[i], 67, y, TFT_GREEN);
    }
  }
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
    case 0: sendIRCode(TV_ADDR, TV_POWER); break;
    case 1: sendIRCode(TV_ADDR, TV_VOL_UP); break;
    case 2: sendIRCode(TV_ADDR, TV_VOL_DN); break;
    case 3: sendIRCode(TV_ADDR, TV_CH_UP); break;
    case 4: sendIRCode(TV_ADDR, TV_CH_DN); break;
    case 5: sendIRCode(TV_ADDR, TV_MUTE); break;
    case 6: currentMenu = MENU_MAIN; selectedItem = 0; break;  // Back
  }
}

void handleSoundbarCommand(uint8_t item) {
  switch (item) {
    case 0: sendIRCode(SB_ADDR, SB_POWER); break;
    case 1: sendIRCode(SB_ADDR, SB_VOL_UP); break;
    case 2: sendIRCode(SB_ADDR, SB_VOL_DN); break;
    case 3: currentMenu = MENU_MAIN; selectedItem = 0; break;  // Back
  }
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
    else if (currentMenu == MENU_TV) maxItems = 7;
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

  Serial.println("AV AI Remote initialized!");
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
