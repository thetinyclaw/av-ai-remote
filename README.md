# AV AI Remote 📡

Universal infrared blaster for AV devices using M5StickC PLUS2.

**Current Status:** Path 1 — Quick Prototype (Basic IR Blaster)

## Hardware

- **M5StickC PLUS2** (ESP32-PICO-V3-02)
- **IR Emitter** on GPIO19
- **Display** 1.14" TFT (135×240)
- **Battery** 200mAh @ 3.7V

## Dependencies

- [Arduino-IRremote](https://github.com/Arduino-IRremote/Arduino-IRremote) — IR transmission
- [M5StickCPlus2 Library](https://github.com/m5stack/M5StickCPlus2) — device drivers

## Roadmap

### Phase 1: Quick Prototype ✏️
- [ ] Basic IR blaster sketch with hardcoded NEC codes
- [ ] Menu UI on display (Button navigation)
- [ ] Test with common AV devices (TV, soundbar, etc.)

### Phase 2: Device Code Storage (Future)
- [ ] SPIFFS-based code database
- [ ] REST/Serial API for remote triggering

### Phase 3: Distributed AI Control (Long-term)
- [ ] BLE/Wi-Fi integration with Mac mini OpenClaw
- [ ] AI-driven device discovery & command dispatch

## Quick Start

1. Clone repo
2. Install Arduino IDE + ESP32 board manager
3. Add M5StickCPlus2 library via Arduino IDE Library Manager
4. Add Arduino-IRremote library (same)
5. Open sketch → Upload to device

## Files

- `src/av-remote.ino` — Main firmware
- `codes/` — IR code database (TBD)
- `docs/` — Hardware notes, pinouts, protocol reference

## License

MIT
