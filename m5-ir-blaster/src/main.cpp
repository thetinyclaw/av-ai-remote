#include <Arduino.h>
#include <M5Unified.h>

#define IR_SEND_PIN 19
#include <IRremote.hpp>

enum Protocol : uint8_t {
  PROTO_LED_EF24,
  PROTO_HISENSE_NEC,
  PROTO_DENON,
  PROTO_DENON_KASEIKYO,
  PROTO_DENON_UNMAPPED,
  PROTO_DENON_MACRO,
};

struct Command {
  const char* name;
  uint32_t raw;
  uint32_t address;
  uint16_t command;
  Protocol protocol;
};

struct ToolPage {
  const char* title;
  const char* hint;
  const Command* commands;
  size_t count;
};

struct MacroStep {
  const Command* command;
  uint8_t repeats;
  uint16_t delayAfterMs;
};

struct Macro {
  const char* name;
  const MacroStep* steps;
  size_t count;
  bool stepByStep;
};

const uint16_t MACRO_STEP_DELAY_MS = 500;
const uint16_t MACRO_ONE_SHOT_DELAY_MS = 100;
const bool MACRO_WAIT_FOR_A_EACH_STEP = true;

// Alec's LED strip receiver: confirmed EF24 family.
// Resolved as a 24-key RGB remote grid, numbered left-to-right/top-to-bottom.
static const Command LED_COMMANDS[] = {
  {"Bright+",        0x00F700FF, 0, 0, PROTO_LED_EF24}, // 1x1
  {"Bright-",        0x00F7807F, 0, 0, PROTO_LED_EF24}, // 1x2
  {"Off",            0x00F740BF, 0, 0, PROTO_LED_EF24}, // 1x3
  {"On",             0x00F7C03F, 0, 0, PROTO_LED_EF24}, // 1x4
  {"Red",            0x00F720DF, 0, 0, PROTO_LED_EF24}, // 2x1
  {"Green",          0x00F7A05F, 0, 0, PROTO_LED_EF24}, // 2x2
  {"Blue",           0x00F7609F, 0, 0, PROTO_LED_EF24}, // 2x3
  {"White",          0x00F7E01F, 0, 0, PROTO_LED_EF24}, // 2x4
  {"Tomato",         0x00F710EF, 0, 0, PROTO_LED_EF24}, // 3x1
  {"LightGreen",     0x00F7906F, 0, 0, PROTO_LED_EF24}, // 3x2
  {"SkyBlue",        0x00F750AF, 0, 0, PROTO_LED_EF24}, // 3x3
  {"Flash",          0x00F7D02F, 0, 0, PROTO_LED_EF24}, // 3x4
  {"OrangeRed",      0x00F730CF, 0, 0, PROTO_LED_EF24}, // 4x1
  {"Cyan",           0x00F7B04F, 0, 0, PROTO_LED_EF24}, // 4x2
  {"RebeccaPurple",  0x00F7708F, 0, 0, PROTO_LED_EF24}, // 4x3
  {"Strobe",         0x00F7F00F, 0, 0, PROTO_LED_EF24}, // 4x4
  {"Orange",         0x00F708F7, 0, 0, PROTO_LED_EF24}, // 5x1
  {"Turquoise",      0x00F78877, 0, 0, PROTO_LED_EF24}, // 5x2
  {"Purple",         0x00F748B7, 0, 0, PROTO_LED_EF24}, // 5x3
  {"Fade",           0x00F7C837, 0, 0, PROTO_LED_EF24}, // 5x4
  {"Yellow",         0x00F728D7, 0, 0, PROTO_LED_EF24}, // 6x1
  {"DarkCyan",       0x00F7A857, 0, 0, PROTO_LED_EF24}, // 6x2
  {"Plum",           0x00F76897, 0, 0, PROTO_LED_EF24}, // 6x3
  {"Smooth",         0x00F7E817, 0, 0, PROTO_LED_EF24}, // 6x4
};

// Denon AVR candidate controls.
// These use Arduino-IRremote's Denon/Sharp protocol sender: 5-bit address + 8-bit command.
// Address 0x02 is the common Denon receiver device family from published/decoded examples.
// Start with high-value controls; we'll test and rename/fix commands like the LED map.
static const uint32_t DENON_KASEIKYO_ADDR = 0x00325441; // Flipper/Denon RC1253 parsed address: genre 0x41 + vendor 0x3254, ID 0.

static const Command DENON_COMMANDS[] = {
  {"Power On",    0, 0x02, 0xE1, PROTO_DENON},
  {"Power Off",   0, 0x02, 0xE2, PROTO_DENON},
  {"Vol +",       0, 0x02, 0xF1, PROTO_DENON}, // reverted: worked before Kaseikyo remap.
  {"Vol -",       0, 0x02, 0xF2, PROTO_DENON}, // reverted: worked before Kaseikyo remap.
  {"Mute",        0, 0x02, 0xF0, PROTO_DENON}, // candidate patch: RCSHP0230048 MUTING.
  {"Bluetooth",   0, DENON_KASEIKYO_ADDR, 0x02EF, PROTO_DENON_KASEIKYO}, // RC1253 Flipper Denon-K: address 41 54 32 00, command EF 02.
  {"Back",        0, DENON_KASEIKYO_ADDR, 0x0022, PROTO_DENON_KASEIKYO}, // RC1253 Flipper Denon-K: address 41 54 32 00, command 22 00.
  {"Option",      0, DENON_KASEIKYO_ADDR, 0x03B5, PROTO_DENON_KASEIKYO}, // RC1253 Flipper Denon-K: address 41 54 32 00, command B5 03.
  {"Setup",       0, 0x0C, 0xA0, PROTO_DENON}, // round 4: RCSHP0C20032 SETUP MENU.
  {"Up",          0, 0x0C, 0xA3, PROTO_DENON}, // round 4: RCSHP0C20035 CURSOR UP.
  {"Enter",       0, 0x02, 0xE0, PROTO_DENON}, // confirmed good.
  {"Right",       0, 0x02, 0xDD, PROTO_DENON}, // confirmed good.
  {"Down",        0, 0x0C, 0xA4, PROTO_DENON}, // round 4: RCSHP0C20036 CURSOR DOWN; 0x02/DF was Info.
  {"Left",        0, 0x0C, 0x7F, PROTO_DENON}, // round 4: RCSHP0C10063 CURSOR LEFT; 0x02/DE was info-like.
  {"Input Phono", 0, 0x02, 0xC3, PROTO_DENON}, // Alec confirmed old “Input Tuner” fired PHONO.
  {"Input CD",    0, 0x02, 0xC4, PROTO_DENON}, // Alec confirmed old “Input V.Aux” fired CD.
  {"Input Tuner", 0, 0x02, 0xC5, PROTO_DENON},
  {"Input CBL/SAT", 0, 0x02, 0xC8, PROTO_DENON},
  {"Input TV Audio", 0, 0x02, 0xC9, PROTO_DENON},
  {"Input Blu-ray", 0, 0x02, 0xCA, PROTO_DENON},
  {"Input Aux",   0, 0x02, 0xCC, PROTO_DENON},
  {"Input Game",  0, 0x02, 0xCD, PROTO_DENON},
  {"Input Media", 0, 0x02, 0xCE, PROTO_DENON},
  {"Stereo",      0, 0x02, 0xD1, PROTO_DENON},
  {"Direct",      0, 0x02, 0xD2, PROTO_DENON},
  {"Status",      0, 0x02, 0xD3, PROTO_DENON},

  // RC-1253 full remote inventory candidates from Flipper-IRDB.
  // These use the custom Flipper-compatible Denon Kaseikyo encoder: address 41 54 32 00 + 10-bit command.
  {"Power Toggle", 0, DENON_KASEIKYO_ADDR, 0x0005, PROTO_DENON_KASEIKYO},
  {"Input USB",    0, DENON_KASEIKYO_ADDR, 0x02CE, PROTO_DENON_KASEIKYO},
  {"Input HEOS",   0, DENON_KASEIKYO_ADDR, 0x02C7, PROTO_DENON_KASEIKYO},
  {"Input Internet", 0, DENON_KASEIKYO_ADDR, 0x02E5, PROTO_DENON_KASEIKYO},
  {"Input Aux1 Dup", 0, DENON_KASEIKYO_ADDR, 0x02D9, PROTO_DENON_KASEIKYO},
  {"Input 8K",     0, DENON_KASEIKYO_ADDR, 0x02DA, PROTO_DENON_KASEIKYO},
  {"Eco",          0, DENON_KASEIKYO_ADDR, 0x0368, PROTO_DENON_KASEIKYO},
  {"Info",         0, DENON_KASEIKYO_ADDR, 0x0270, PROTO_DENON_KASEIKYO},
  {"CH/Page +",    0, DENON_KASEIKYO_ADDR, 0x03B1, PROTO_DENON_KASEIKYO},
  {"CH/Page -",    0, DENON_KASEIKYO_ADDR, 0x03B2, PROTO_DENON_KASEIKYO},
  {"Play/Pause",   0, DENON_KASEIKYO_ADDR, 0x03E0, PROTO_DENON_KASEIKYO},
  {"Prev/Tune -",  0, DENON_KASEIKYO_ADDR, 0x03E3, PROTO_DENON_KASEIKYO},
  {"Next/Tune +",  0, DENON_KASEIKYO_ADDR, 0x03E4, PROTO_DENON_KASEIKYO},
  {"Quick Sel 1",  0, DENON_KASEIKYO_ADDR, 0x0124, PROTO_DENON_KASEIKYO},
  {"Quick Sel 2",  0, DENON_KASEIKYO_ADDR, 0x0125, PROTO_DENON_KASEIKYO},
  {"Quick Sel 3",  0, DENON_KASEIKYO_ADDR, 0x0126, PROTO_DENON_KASEIKYO},
  {"Quick Sel 4",  0, DENON_KASEIKYO_ADDR, 0x0127, PROTO_DENON_KASEIKYO},
  {"Movie Mode",   0, DENON_KASEIKYO_ADDR, 0x0149, PROTO_DENON_KASEIKYO},
  {"Music Mode",   0, DENON_KASEIKYO_ADDR, 0x014A, PROTO_DENON_KASEIKYO},
  {"Game Mode",    0, DENON_KASEIKYO_ADDR, 0x014B, PROTO_DENON_KASEIKYO},
  {"Pure Mode",    0, DENON_KASEIKYO_ADDR, 0x0135, PROTO_DENON_KASEIKYO},
};

enum DenonCommandIndex : uint8_t {
  DENON_BLUETOOTH = 5,
  DENON_BACK = 6,
  DENON_OPTION = 7,
  DENON_SETUP = 8,
  DENON_UP = 9,
  DENON_ENTER = 10,
  DENON_RIGHT = 11,
  DENON_DOWN = 12,
  DENON_LEFT = 13,
  DENON_HEOS = 28,
};

#define DENON_STEP(idx, reps) {&DENON_COMMANDS[idx], reps, MACRO_STEP_DELAY_MS}

static const MacroStep DENON_BASS_BOOST_STEPS[] = {
  DENON_STEP(DENON_HEOS, 1),
  DENON_STEP(DENON_BACK, 1),
  DENON_STEP(DENON_OPTION, 1),
  DENON_STEP(DENON_DOWN, 1),
  DENON_STEP(DENON_ENTER, 1),
  DENON_STEP(DENON_DOWN, 1),
  DENON_STEP(DENON_RIGHT, 2),
  DENON_STEP(DENON_BACK, 1),
  DENON_STEP(DENON_BACK, 1),
};

static const MacroStep DENON_MUSIC_RESET_STEPS[] = {
  DENON_STEP(DENON_HEOS, 1),
  DENON_STEP(DENON_BACK, 1),
  DENON_STEP(DENON_OPTION, 1),
  DENON_STEP(DENON_ENTER, 1),
  DENON_STEP(DENON_LEFT, 48),
  DENON_STEP(DENON_RIGHT, 24),
  DENON_STEP(DENON_BACK, 1),
  DENON_STEP(DENON_DOWN, 1),
  DENON_STEP(DENON_ENTER, 1),
  DENON_STEP(DENON_DOWN, 1),
  DENON_STEP(DENON_LEFT, 12),
  DENON_STEP(DENON_RIGHT, 6),
  DENON_STEP(DENON_DOWN, 1),
  DENON_STEP(DENON_LEFT, 12),
  DENON_STEP(DENON_RIGHT, 6),
  DENON_STEP(DENON_BACK, 1),
  DENON_STEP(DENON_BACK, 1),
};

static const MacroStep DENON_TREBLE_BOOST_STEPS[] = {
  DENON_STEP(DENON_HEOS, 1),
  DENON_STEP(DENON_BACK, 1),
  DENON_STEP(DENON_OPTION, 1),
  DENON_STEP(DENON_DOWN, 1),
  DENON_STEP(DENON_ENTER, 1),
  DENON_STEP(DENON_DOWN, 1),
  DENON_STEP(DENON_DOWN, 1),
  DENON_STEP(DENON_RIGHT, 2),
  DENON_STEP(DENON_BACK, 1),
  DENON_STEP(DENON_BACK, 1),
};

static const MacroStep DENON_SUBWOOFER_BOOST_STEPS[] = {
  DENON_STEP(DENON_HEOS, 1),
  DENON_STEP(DENON_BACK, 1),
  DENON_STEP(DENON_OPTION, 1),
  DENON_STEP(DENON_ENTER, 1),
  DENON_STEP(DENON_RIGHT, 4),
  DENON_STEP(DENON_BACK, 1),
  DENON_STEP(DENON_BACK, 1),
};

static const Macro DENON_MACROS[] = {
  {"Bass boost", DENON_BASS_BOOST_STEPS, sizeof(DENON_BASS_BOOST_STEPS) / sizeof(DENON_BASS_BOOST_STEPS[0]), false},
  {"Treble boost", DENON_TREBLE_BOOST_STEPS, sizeof(DENON_TREBLE_BOOST_STEPS) / sizeof(DENON_TREBLE_BOOST_STEPS[0]), false},
  {"Reset", DENON_MUSIC_RESET_STEPS, sizeof(DENON_MUSIC_RESET_STEPS) / sizeof(DENON_MUSIC_RESET_STEPS[0]), false},
  {"Subwoofer boost", DENON_SUBWOOFER_BOOST_STEPS, sizeof(DENON_SUBWOOFER_BOOST_STEPS) / sizeof(DENON_SUBWOOFER_BOOST_STEPS[0]), false},
};

static const Command DENON_MACRO_COMMANDS[] = {
  {"Bass boost", 0, 0, 0, PROTO_DENON_MACRO},
  {"Treble boost", 1, 0, 0, PROTO_DENON_MACRO},
  {"Reset", 2, 0, 0, PROTO_DENON_MACRO},
  {"Subwoofer boost", 3, 0, 0, PROTO_DENON_MACRO},
};

// Hisense TV NEC candidates from decoded B7500-style remote tables.
// Stored as 32-bit padded raw NEC values; tri-blast sends common IRremote orderings.
static const Command HISENSE_COMMANDS[] = {
  {"Power",       0x00FDB04F, 0, 0, PROTO_HISENSE_NEC},
  {"Input",       0x00FD48B7, 0, 0, PROTO_HISENSE_NEC},
  {"Home",        0x00FD04FB, 0, 0, PROTO_HISENSE_NEC},
  {"Menu",        0x00FD28D7, 0, 0, PROTO_HISENSE_NEC},
  {"Up",          0x00FD6897, 0, 0, PROTO_HISENSE_NEC},
  {"Down",        0x00FDE817, 0, 0, PROTO_HISENSE_NEC},
  {"Left",        0x00FD9867, 0, 0, PROTO_HISENSE_NEC},
  {"Right",       0x00FD18E7, 0, 0, PROTO_HISENSE_NEC},
  {"OK",          0x00FDA857, 0, 0, PROTO_HISENSE_NEC},
  {"Back",        0x00FD12ED, 0, 0, PROTO_HISENSE_NEC},
  {"Exit",        0x00FD3AC5, 0, 0, PROTO_HISENSE_NEC},
  {"Vol +",       0x00FD22DD, 0, 0, PROTO_HISENSE_NEC},
  {"Vol -",       0x00FDC23D, 0, 0, PROTO_HISENSE_NEC},
  {"Mute",        0x00FD708F, 0, 0, PROTO_HISENSE_NEC},
  {"Ch +",        0x00FD52AD, 0, 0, PROTO_HISENSE_NEC},
  {"Ch -",        0x00FDD22D, 0, 0, PROTO_HISENSE_NEC},
  {"Netflix",     0x00FDE21D, 0, 0, PROTO_HISENSE_NEC},
  {"YouTube",     0x00FD55AA, 0, 0, PROTO_HISENSE_NEC},
};

static const ToolPage PAGES[] = {
  {"LED STRIP 24-KEY V2", "WLED pos: row x col", LED_COMMANDS, sizeof(LED_COMMANDS) / sizeof(LED_COMMANDS[0])},
  {"DENON AVR TEST REMOTE", "Denon addr 02 candidate map", DENON_COMMANDS, sizeof(DENON_COMMANDS) / sizeof(DENON_COMMANDS[0])},
  {"DENON MACROS", "A runs selected macro", DENON_MACRO_COMMANDS, sizeof(DENON_MACRO_COMMANDS) / sizeof(DENON_MACRO_COMMANDS[0])},
  {"HISENSE TV TEST REMOTE", "Hisense NEC candidates", HISENSE_COMMANDS, sizeof(HISENSE_COMMANDS) / sizeof(HISENSE_COMMANDS[0])},
};
static const size_t PAGE_COUNT = sizeof(PAGES) / sizeof(PAGES[0]);

size_t page = 0;
size_t selected = 0;
uint32_t lastDraw = 0;
uint32_t lastSend = 0;
uint32_t lastPulse = 0;
uint32_t comboAt = 0;
bool comboHandled = false;
const uint32_t HOLD_PULSE_MS = 650;
const uint32_t FAST_HOLD_PULSE_MS = HOLD_PULSE_MS / 15;

const ToolPage& currentPage() {
  return PAGES[page];
}

const Command& currentCommand() {
  const ToolPage& p = currentPage();
  return p.commands[selected];
}

void ledPos(size_t idx, char* out, size_t len) {
  snprintf(out, len, "%ux%u", (unsigned)(idx / 4 + 1), (unsigned)(idx % 4 + 1));
}

void drawUi(const char* status = nullptr) {
  const ToolPage& p = currentPage();
  const Command& c = currentCommand();

  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.setRotation(3);
  M5.Display.setTextDatum(top_center);

  M5.Display.setTextColor(TFT_MAGENTA, TFT_BLACK);
  M5.Display.setTextSize(1);
  M5.Display.drawString(p.title, M5.Display.width() / 2, 4);
  M5.Display.drawLine(0, 20, M5.Display.width(), 20, TFT_DARKGREY);

  char buf[80];
  if (c.protocol == PROTO_LED_EF24) {
    char pos[8];
    ledPos(selected, pos, sizeof(pos));
    snprintf(buf, sizeof(buf), "%s  %s", pos, c.name);
  } else if (c.protocol == PROTO_DENON_MACRO) {
    snprintf(buf, sizeof(buf), "MACRO %s", c.name);
  } else {
    snprintf(buf, sizeof(buf), "%02u  %s", (unsigned)selected, c.name);
  }
  M5.Display.setTextColor(TFT_CYAN, TFT_BLACK);
  M5.Display.setTextSize(2);
  M5.Display.drawString(buf, M5.Display.width() / 2, 34);

  M5.Display.setTextColor(TFT_GREEN, TFT_BLACK);
  M5.Display.setTextSize(1);
  if (c.protocol == PROTO_LED_EF24 || c.protocol == PROTO_HISENSE_NEC) {
    snprintf(buf, sizeof(buf), "NEC 0x%08lX", (unsigned long)c.raw);
  } else if (c.protocol == PROTO_DENON_MACRO) {
    const Macro& macro = DENON_MACROS[c.raw];
    snprintf(buf, sizeof(buf), "%u steps / %s", (unsigned)macro.count, macro.stepByStep ? "A-gated" : "one-shot");
  } else if (c.protocol == PROTO_DENON_KASEIKYO) {
    snprintf(buf, sizeof(buf), "KDENON addr=0x%03lX cmd=0x%02X", (unsigned long)c.address, c.command);
  } else if (c.protocol == PROTO_DENON_UNMAPPED) {
    snprintf(buf, sizeof(buf), "UNMAPPED - no blast");
  } else {
    snprintf(buf, sizeof(buf), "DENON addr=0x%02lX cmd=0x%02X", (unsigned long)c.address, c.command);
  }
  M5.Display.drawString(buf, M5.Display.width() / 2, 72);

  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  if (c.protocol == PROTO_LED_EF24) {
    char pos[8];
    ledPos(selected, pos, sizeof(pos));
    snprintf(buf, sizeof(buf), "EF24 pos %s  %u/%u", pos, (unsigned)(selected + 1), (unsigned)p.count);
  } else if (c.protocol == PROTO_HISENSE_NEC) {
    snprintf(buf, sizeof(buf), "HISENSE #%02u  page %u/%u  %u/%u", (unsigned)selected, (unsigned)(page + 1), (unsigned)PAGE_COUNT, (unsigned)(selected + 1), (unsigned)p.count);
  } else if (c.protocol == PROTO_DENON_MACRO) {
    snprintf(buf, sizeof(buf), "MACRO #%02u  page %u/%u", (unsigned)selected, (unsigned)(page + 1), (unsigned)PAGE_COUNT);
  } else {
    snprintf(buf, sizeof(buf), "DENON #%02u  page %u/%u  %u/%u", (unsigned)selected, (unsigned)(page + 1), (unsigned)PAGE_COUNT, (unsigned)(selected + 1), (unsigned)p.count);
  }
  M5.Display.drawString(buf, M5.Display.width() / 2, 92);

  M5.Display.setTextColor(TFT_YELLOW, TFT_BLACK);
  M5.Display.drawString("A tap/hold: blast/repeat", M5.Display.width() / 2, 112);
  M5.Display.drawString("Side: choose / hold tool", M5.Display.width() / 2, 126);
  M5.Display.drawString(p.hint, M5.Display.width() / 2, 140);

  if (status) {
    M5.Display.setTextColor(TFT_RED, TFT_BLACK);
    M5.Display.drawString(status, M5.Display.width() / 2, 154);
  }
}


void drawMacroStepUi(const Macro& macro, size_t stepIndex) {
  const MacroStep& step = macro.steps[stepIndex];
  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.setRotation(3);
  M5.Display.setTextDatum(top_center);

  M5.Display.setTextColor(TFT_MAGENTA, TFT_BLACK);
  M5.Display.setTextSize(1);
  M5.Display.drawString("DENON MACRO STEP", M5.Display.width() / 2, 4);
  M5.Display.drawLine(0, 20, M5.Display.width(), 20, TFT_DARKGREY);

  char buf[96];
  M5.Display.setTextColor(TFT_YELLOW, TFT_BLACK);
  M5.Display.setTextSize(2);
  snprintf(buf, sizeof(buf), "STEP %u/%u", (unsigned)(stepIndex + 1), (unsigned)macro.count);
  M5.Display.drawString(buf, M5.Display.width() / 2, 30);

  M5.Display.setTextColor(TFT_CYAN, TFT_BLACK);
  M5.Display.setTextSize(2);
  if (step.repeats > 1) {
    snprintf(buf, sizeof(buf), "%s x%u", step.command->name, (unsigned)step.repeats);
  } else {
    snprintf(buf, sizeof(buf), "%s", step.command->name);
  }
  M5.Display.drawString(buf, M5.Display.width() / 2, 62);

  M5.Display.setTextColor(TFT_GREEN, TFT_BLACK);
  M5.Display.setTextSize(1);
  if (step.repeats > 1) {
    snprintf(buf, sizeof(buf), "A fires %u blasts, %ums apart", (unsigned)step.repeats, (unsigned)step.delayAfterMs);
  } else {
    snprintf(buf, sizeof(buf), "A fires: %s", step.command->name);
  }
  M5.Display.drawString(buf, M5.Display.width() / 2, 96);

  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  if (macro.stepByStep) {
    M5.Display.drawString("Press A for this step", M5.Display.width() / 2, 120);
    M5.Display.drawString("Then next step appears", M5.Display.width() / 2, 138);
  } else {
    M5.Display.drawString("One-shot: auto running", M5.Display.width() / 2, 120);
    M5.Display.drawString("Do not point away", M5.Display.width() / 2, 138);
  }
}

bool isFastRepeatCommand() {
  const Command& c = currentCommand();
  return c.protocol == PROTO_LED_EF24 && (c.raw == 0x00F700FF || c.raw == 0x00F7807F);
}

uint32_t holdPulseInterval() {
  return isFastRepeatCommand() ? FAST_HOLD_PULSE_MS : HOLD_PULSE_MS;
}

void sendDenonKaseikyoFlipper(uint32_t address, uint16_t data, int_fast8_t repeats) {
  const uint16_t vendor = 0x3254;
  const uint8_t vendorParity = 0x0;
  const uint8_t genre1 = (address >> 4) & 0x0F;
  const uint8_t genre2 = address & 0x0F;
  const uint8_t id = (address >> 24) & 0x03;

  uint8_t bytes[6];
  bytes[0] = vendor & 0xFF;
  bytes[1] = vendor >> 8;
  bytes[2] = (genre1 << 4) | vendorParity;
  bytes[3] = ((data & 0x0F) << 4) | genre2;
  bytes[4] = (id << 6) | ((data >> 4) & 0x3F);
  bytes[5] = bytes[2] ^ bytes[3] ^ bytes[4];

  IRDecodedRawDataType raw = 0;
  for (uint8_t i = 0; i < 6; i++) raw |= ((IRDecodedRawDataType)bytes[i]) << (8 * i);

  IrSender.sendPulseDistanceWidth(37, 3456, 1728, 432, 1296, 432, 432, raw, 48, PROTOCOL_IS_LSB_FIRST | PROTOCOL_IS_PULSE_DISTANCE, 130, repeats);
}

void sendCommand(const Command& c, bool fastPulse = false, bool redraw = true) {
  Serial.printf("BLAST page=%u name=%s proto=%u raw=0x%08lX addr=0x%02X cmd=0x%02X pin=%d fast=%u\n",
                (unsigned)page, c.name, (unsigned)c.protocol, (unsigned long)c.raw, (unsigned)c.address, (unsigned)c.command, IR_SEND_PIN, fastPulse ? 1 : 0);

  if (fastPulse && c.protocol == PROTO_LED_EF24 && (c.raw == 0x00F700FF || c.raw == 0x00F7807F)) {
    IrSender.sendNECMSB(c.raw, 32, false);
  } else if (c.protocol == PROTO_HISENSE_NEC) {
    if (redraw) drawUi("HISENSE BLAST!");
    IrSender.sendNECMSB(c.raw, 32, false);
  } else if (c.protocol == PROTO_LED_EF24) {
    if (redraw) drawUi("LED TRI-BLAST!");
    uint8_t address = (uint8_t)((c.raw >> 24) & 0xFF);
    uint8_t command = (uint8_t)((c.raw >> 8) & 0xFF);

    for (int i = 0; i < 2; i++) {
      IrSender.sendNEC(address, command, 0);
      delay(75);
      IrSender.sendNECMSB(c.raw, 32, false);
      delay(75);
      IrSender.sendNECRaw(c.raw, 0);
      delay(75);
    }
  } else if (c.protocol == PROTO_DENON_KASEIKYO) {
    if (redraw) drawUi("K-DENON BLAST!");
    sendDenonKaseikyoFlipper(c.address, c.command, 1);
  } else if (c.protocol == PROTO_DENON_UNMAPPED) {
    if (redraw) drawUi("UNMAPPED");
    Serial.printf("SKIP unmapped Denon command: %s\n", c.name);
  } else {
    if (redraw) drawUi("DENON BLAST!");
    IrSender.sendDenon(c.address, c.command, 1);
  }
  lastSend = millis();
}

void runMacro(size_t macroIndex) {
  if (macroIndex >= (sizeof(DENON_MACROS) / sizeof(DENON_MACROS[0]))) return;
  const Macro& macro = DENON_MACROS[macroIndex];
  Serial.printf("MACRO start idx=%u name=%s steps=%u mode=%s delay=%ums\n", (unsigned)macroIndex, macro.name, (unsigned)macro.count, macro.stepByStep ? "A-gated" : "one-shot", (unsigned)MACRO_STEP_DELAY_MS);

  for (size_t i = 0; i < macro.count; i++) {
    const MacroStep& step = macro.steps[i];
    drawMacroStepUi(macro, i);

    if (macro.stepByStep) {
      Serial.printf("MACRO waiting step=%u/%u name=%s repeats=%u\n", (unsigned)(i + 1), (unsigned)macro.count, step.command->name, step.repeats);
      while (true) {
        M5.update();
        if (M5.BtnA.wasPressed()) break;
        delay(10);
      }
    } else {
      Serial.printf("MACRO auto step=%u/%u name=%s repeats=%u\n", (unsigned)(i + 1), (unsigned)macro.count, step.command->name, step.repeats);
      delay(250);
    }

    Serial.printf("MACRO fire step=%u/%u name=%s repeats=%u\n", (unsigned)(i + 1), (unsigned)macro.count, step.command->name, step.repeats);
    for (uint8_t r = 0; r < step.repeats; r++) {
      sendCommand(*step.command, false, false);
      if (r + 1 < step.repeats) delay(macro.stepByStep ? step.delayAfterMs : MACRO_ONE_SHOT_DELAY_MS);
    }
    drawUi(macro.stepByStep ? "STEP FIRED" : "AUTO STEP");
    delay(macro.stepByStep ? 250 : MACRO_ONE_SHOT_DELAY_MS);
  }

  Serial.printf("MACRO done idx=%u name=%s\n", (unsigned)macroIndex, macro.name);
  drawUi("MACRO DONE");
}

void sendSelected(bool fastPulse = false) {
  const Command& c = currentCommand();
  if (c.protocol == PROTO_DENON_MACRO) {
    runMacro(c.raw);
  } else {
    sendCommand(c, fastPulse, true);
  }
}


void nextCommand() {
  const ToolPage& p = currentPage();
  selected = (selected + 1) % p.count;
  drawUi();
  Serial.printf("SELECT page=%u idx=%u: %s\n", (unsigned)page, (unsigned)selected, currentCommand().name);
}

void prevCommand() {
  const ToolPage& p = currentPage();
  selected = (selected + p.count - 1) % p.count;
  drawUi();
  Serial.printf("SELECT page=%u idx=%u: %s\n", (unsigned)page, (unsigned)selected, currentCommand().name);
}

void switchPage() {
  page = (page + 1) % PAGE_COUNT;
  selected = 0;
  drawUi("SWITCHED TOOL");
  Serial.printf("PAGE %u: %s\n", (unsigned)page, currentPage().title);
}

void handleSerial() {
  while (Serial.available()) {
    char ch = Serial.read();
    if (ch == 'n' || ch == '+') nextCommand();
    else if (ch == 'p' || ch == '-') prevCommand();
    else if (ch == 't') switchPage();
    else if (ch == 's' || ch == 'b' || ch == '\n') sendSelected();
    else if (ch >= '0' && ch <= '9') {
      size_t idx = (size_t)(ch - '0');
      if (idx < currentPage().count) {
        selected = idx;
        drawUi();
        Serial.printf("SELECT page=%u idx=%u: %s\n", (unsigned)page, (unsigned)selected, currentCommand().name);
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
  Serial.println("M5 multi-tool IR blaster ready - LED EF24 + Denon AVR + Hisense TV");
  Serial.printf("IR TX pin: GPIO%d\n", IR_SEND_PIN);
  Serial.println("Buttons: A tap blast, hold A repeat pulse, Side(B/C) choose, hold Side(B/C) switch tool. Serial: n next, p prev, t tool, s send, 0-9 select.");
  drawUi("READY");
}

void loop() {
  M5.update();
  handleSerial();

  bool sidePressed = M5.BtnB.isPressed() || M5.BtnC.isPressed();
  bool sideReleased = M5.BtnB.wasReleased() || M5.BtnC.wasReleased();
  if (sidePressed) {
    if (!comboAt) comboAt = millis();
    if (!comboHandled && millis() - comboAt > 700) {
      comboHandled = true;
      switchPage();
    }
  } else {
    if (M5.BtnA.wasPressed()) {
      sendSelected();
      lastPulse = millis();
    } else if (M5.BtnA.isPressed() && millis() - lastPulse > holdPulseInterval()) {
      sendSelected(true);
      lastPulse = millis();
    }
    if (M5.BtnA.wasReleased()) lastPulse = 0;
    if (!comboHandled && sideReleased) nextCommand();
    comboAt = 0;
    comboHandled = false;
  }

  if (lastSend && millis() - lastSend > 600) {
    lastSend = 0;
    drawUi();
  }

  if (millis() - lastDraw > 5000) {
    lastDraw = millis();
    Serial.printf("alive page=%u selected=%u %s\n", (unsigned)page, (unsigned)selected, currentCommand().name);
  }
  delay(10);
}
