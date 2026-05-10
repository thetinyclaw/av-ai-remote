# IR Code Reference

## Confirmed: Alec's LED Strip IR Protocol — EF24

**Confirmed:** 2026-05-10 via M5StickC IR blaster testing.

Alec's LED strip receiver responds to the **EF24** command family, not the common 44-key `00FFxxxx` RGB-strip table.

**Format:** NEC-style 32-bit frames  
**Observed family:** `EF24 00–23` emitted as `0x00F7xxxx` candidate codes  
**Firmware:** `/Users/tinyclaw/GitHub/av-ai-remote/m5-ir-blaster`  
**M5 mapping:** Button A = fire/action/blast; Button B = previous command; Button C = next command.

Use the EF24 list as the known-good protocol for this strip.

---

## Protocol: Samsung AA59 Remote (Extended NEC)

**Format:** 32-bit Samsung protocol (extended NEC variant)
**Address:** `0xE0E0` (Samsung TV universal address)
**Command:** 8-bit codes (see below)

**Transmission:**
```cpp
IrSender.sendNEC(0xE0E0, 0xBF, 0);  // Power toggle (0xBF = power command)
```

---

## Samsung TV Commands (AA59 Remote)

**Source:** 2016 Samsung TV remote (widely compatible with modern Samsung models)

| Function | Code | Hex | Notes |
|----------|------|-----|-------|
| **Power** | 0xBF | E0E040BF | Toggle standby |
| **Volume Up** | 0x1F | E0E0E01F | |
| **Volume Down** | 0x2F | E0E0D02F | |
| **Channel Up** | 0xB7 | E0E048B7 | |
| **Channel Down** | 0xF7 | E0E008F7 | |
| **Mute** | 0x0F | E0E0F00F | Toggle mute |
| **Menu** | 0xA7 | E0E058A7 | Open menu |
| **Source/Input** | 0x7F | E0E0807F | Switch input |
| **Info** | 0x07 | E0E0F807 | Show info |
| **Guide** | 0x0D | E0E0F20D | EPG guide |
| **Return/Back** | 0xE5 | E0E01AE5 | Go back |
| **Enter/OK** | 0xE9 | E0E016E9 | Select |
| **Up Arrow** | 0xF9 | E0E006F9 | |
| **Down Arrow** | 0x79 | E0E08679 | |
| **Left Arrow** | 0x59 | E0E0A659 | |
| **Right Arrow** | 0xB9 | E0E046B9 | |

---

## Other Device Codes (Reference)

### LG TV (0x04FB)
| Function | Code |
|----------|------|
| Power | 0x40 |
| Volume Up | 0x18 |
| Volume Down | 0x19 |

### Sony TV (0xA73C)
| Function | Code |
|----------|------|
| Power | 0xA9 |
| Volume Up | 0x14 |
| Volume Down | 0x15 |

---

## Finding IR Codes

**Option 1:** Google `[Device Model] IR NEC codes`

**Option 2:** Use an IR receiver to sniff existing remote:
- M5Stack IR Unit + Arduino sketch
- Decode the transmitted codes

**Option 3:** Device datasheet/manual (often includes IR protocol spec)

---

## Protocol Support

Arduino-IRremote also supports:
- Onkyo (extended NEC)
- Sony (SIRC)
- Panasonic
- JVC
- Denon (depends on variant)

Check library docs for syntax.
