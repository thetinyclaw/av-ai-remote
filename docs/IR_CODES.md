# IR Code Reference

## NEC Protocol Format

Standard NEC uses:
- **Address**: 16-bit (device ID)
- **Command**: 8-bit (action code)

Example: `IrSender.sendNEC(0x04FB, 0x40, 0);`
- Address: `0x04FB` (LG TV)
- Command: `0x40` (Power toggle)
- Repeats: `0` (single transmission)

---

## Common Device Codes

### LG TV (0x04FB)
| Function | Code |
|----------|------|
| Power | 0x40 |
| Volume Up | 0x18 |
| Volume Down | 0x19 |
| Channel Up | 0x00 |
| Channel Down | 0x01 |
| Mute | 0x65 |
| Input | 0x39 |

### Sony TV (0xA73C)
| Function | Code |
|----------|------|
| Power | 0xA9 |
| Volume Up | 0x14 |
| Volume Down | 0x15 |

### Samsung TV (0x0707)
| Function | Code |
|----------|------|
| Power | 0xF2 |
| Volume Up | 0x07 |
| Volume Down | 0x0B |

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
