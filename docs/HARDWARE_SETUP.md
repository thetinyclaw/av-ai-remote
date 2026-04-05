# Hardware Setup Guide

## M5StickC PLUS2 Pinout (Relevant)

| Function | GPIO | Notes |
|----------|------|-------|
| IR Emitter | 19 | Shared with Red LED |
| Button A | 37 | User button |
| Button B | 39 | User button |
| Button C | 35 | Wake/Power button |
| Display MOSI | 15 | TFT Display |
| Display CLK | 13 | TFT Display |
| Display DC | 14 | TFT Display |
| Display RST | 12 | TFT Display |
| Display CS | 5 | TFT Display |

---

## Assembly

1. **Unbox M5StickC PLUS2**
   - Check USB connection via Type-C cable
   - Install driver if needed (CH9102_VCP)

2. **IR Range**
   - Internal emitter: ~5 meters typical
   - Best results: Line-of-sight, perpendicular to device receiver

3. **Power**
   - Charge via USB-C (~2 hours full charge)
   - 200mAh battery → ~4-6 hours runtime (depends on display usage)

---

## Arduino IDE Setup

1. **Install Arduino IDE** (1.8.x or 2.x)

2. **Add ESP32 Board Manager**
   - Preferences → Additional Boards Manager URLs
   - Add: `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
   - Boards Manager → Search "ESP32" → Install

3. **Select Board: "M5StickCPlus2"**
   - Tools → Board → ESP32 → M5StickCPlus2

4. **Install Libraries**
   - Sketch → Include Library → Manage Libraries
   - Search & Install:
     - `M5StickCPlus2` (by M5Stack)
     - `Arduino-IRremote` (by Z. Antal)

5. **Configure Upload**
   - Tools → Upload Speed: `1500000`
   - Tools → Port: `/dev/cu.usbserial-*` (macOS)

---

## First Upload

```bash
# In Arduino IDE:
1. Open av-remote.ino
2. Tools → Port → Select device
3. Sketch → Upload (Ctrl+U / Cmd+U)
4. Wait for "Uploading..." → "Done uploading"
5. Open Serial Monitor (115200 baud) to verify
```

Expected output:
```
AV AI Remote initialized!
Button A: Up | Button B: Down | Button C: Select/Back
```

---

## Troubleshooting

| Issue | Fix |
|-------|-----|
| Port not recognized | Reinstall CH9102 driver |
| "Failed to write to target RAM" | Long-press power button 6s to reset |
| Sketch upload timeout | Reduce upload speed to 921600 |
| Display blank | Check display pins in library config |
