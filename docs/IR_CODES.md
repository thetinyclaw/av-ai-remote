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

## In Progress: Denon AVR M5 Controller

**Started:** 2026-05-10.

The M5 firmware now has a second tool page: **DENON AVR TEST REMOTE**. Hold **B+C** to switch between the LED strip tool and Denon page.

Initial Denon implementation uses Arduino-IRremote's built-in `sendDenon(address, command, repeats)` sender:

```cpp
IrSender.sendDenon(0x02, command, 1);
```

Candidate map loaded for testing:

| # | Label | Address | Command | Confidence |
|---|-------|---------|---------|------------|
| 00 | Power On | `0x02` | `0xE1` | medium-high: decoded from Denon discrete power Pronto pattern |
| 01 | Power Off | `0x02` | `0xE2` | medium-high: decoded from Denon discrete power Pronto pattern |
| 02 | Vol + | `0x02` | `0xF1` | medium: common Denon/Sharp D=2 F=241 reference |
| 03 | Vol - | `0x02` | `0xF2` | medium: common Denon/Sharp D=2 F=242 reference |
| 04 | Mute | `0x02` | `0xF3` | guess adjacent to volume commands |
| 05–11 | Inputs/modes/status | `0x02` | candidate guesses | low: needs testing/sniffing |

Treat Denon labels as provisional until Alec tests them against the AVR or we sniff the original remote.

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

## Denon Macro Mode — Bass Setup V1

**Added:** 2026-05-12.

M5 firmware page: **DENON MACROS**. Hold side button(s) to switch tools until the macro page appears, then tap **A** to run the selected one-shot macro.

Initial macro requested by Alec. Current behavior: **one-shot mode only** for Bass boost and Reset. All one-shot macro commands/repeats are spaced at `100 ms`; faster repeated-button bursts caused AVR menu acceleration/coalescing issues.

### Bass boost sequence

1. HEOS
2. Back
3. Option
4. Down
5. Enter
6. Down
7. Right ×2
8. Back
9. Back

Available as **Bass boost** one-shot. One-shot mode uses `100 ms` between all commands/repeats.

### Treble boost sequence

1. HEOS
2. Back
3. Option
4. Down
5. Enter
6. Down
7. Down
8. Right ×2
9. Back
10. Back

Available as **Treble boost** one-shot. One-shot mode uses `100 ms` between all commands/repeats.

### Music reset sequence

1. HEOS
2. Back
3. Option
4. Enter
5. Left ×48
6. Right ×24
7. Back
8. Down
9. Enter
10. Down
11. Left ×12
12. Right ×6
13. Down
14. Left ×12
15. Right ×6
16. Back
17. Back

Available as **Reset** one-shot. One-shot mode uses `100 ms` between all commands/repeats.

### Subwoofer boost sequence

1. HEOS
2. Back
3. Option
4. Enter
5. Right ×4
6. Back
7. Back

Available as **Subwoofer boost** one-shot. One-shot mode uses `100 ms` between all commands/repeats.

Notes: Macro V1 now starts from **HEOS** instead of Bluetooth. `Back` and `Option` use the validated Flipper/RC1253-style Denon Kaseikyo encoder; `Setup`, `Up`, `Down`, and `Left` use the validated service-table Denon/Sharp mappings; `Enter` and `Right` remain confirmed-good Denon mappings.


## Denon Mapping Correction — 2026-05-12

Alec tested the first Denon table and found these mappings were wrong/not firing: `Mute`, `Option`, `Bluetooth`, `Down`, `Left`, `Back`. He also confirmed two labels were definitely offset: old `Input Tuner` fired **PHONO**, and old `Input V.Aux` fired **CD**.

Root cause: the first table mixed guessed Denon/Sharp command labels with modern Denon Kaseikyo remote commands. The firmware now uses:

- Denon/Sharp table where confirmed by Alec/table cross-check: `Input Phono` = `addr 0x02 cmd 0xC3`, `Input CD` = `addr 0x02 cmd 0xC4`, then Tuner/CBL/SAT/TV Audio/Blu-ray/Aux/Game/Media at the corresponding table offsets.
- Flipper/RC1253-style Denon Kaseikyo encoder for modern remote keys: `Bluetooth`, `Back`, `Option`, `HEOS`, `USB`, `Internet`, transport keys, Quick Selects, etc. Parsed address is `41 54 32 00` / vendor `0x3254`, encoded as genre `0x41`, ID `0`.

If navigation still misses, sniff the original remote and compare against the RC1253 parsed commands before touching the macro sequence itself.
