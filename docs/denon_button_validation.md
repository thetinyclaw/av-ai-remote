# Denon Button Validation

Firmware: `m5-ir-blaster`
Receiver model: `Denon AVR-S960H`
Updated: 2026-05-12 21:29 CDT

## Current findings

- **Setup, Up, Down, and Left are now confirmed correct** from round 4.
- **Mute, Enter, Right, Vol +/- and Power On/Off remain confirmed good**.
- **Bluetooth, Back, and Option are now confirmed good** with round 6 custom Flipper-compatible Denon RC-1253 Kaseikyo packing.
- Remaining high-confidence remote inventory candidates were added from the same RC-1253 map for live testing.



## Web cross-check — 2026-05-12 21:04 CDT

High-signal source found: Flipper-IRDB `Denon_RC1253.ir`, matching the visible remote family. Cross-check result:

- RC-1253 file uses generic Flipper `Kaseikyo`, address bytes `41 54 32 00` for every key.
- `Option` is `command: B5 03 00 00` = 10-bit command `0x03B5`.
- `Back` is `command: 22 00 00 00` = 10-bit command `0x0022`.
- This explains the failure: Arduino-IRremote `sendKaseikyo_Denon(address, uint8_t command)` only sends an 8-bit command and a different address layout. It cannot represent `Option=0x03B5` correctly, and its `Back` byte packing differs from Flipper/RC-1253.
- Firmware round 6 now uses `sendDenonKaseikyoFlipper(...)` for `PROTO_DENON_KASEIKYO`, preserving confirmed-good Sharp/Denon commands untouched.

## Remote inventory gaps from photo — 2026-05-12 20:55 CDT

Visible AVR-S960H remote buttons not yet tracked as first-class Denon validation rows:

- **Still unmapped / not found in RC-1253 map yet:** `ZONE SELECT`, `MAIN`, `ZONE2`, `SLEEP`, `DVD`, possibly `HDMI OUT` depending on exact lower-label read.
- **Mapped + validated:** `POWER` toggle, `USB`, `INTERNET RADIO`, `HEOS`, `8K` (`Aux2` candidate), play/pause, `TUNE -`, `TUNE +`, Quick Select `1`-`4`.
- **Mapped but pending/partial:** `CH/PAGE +/-` presumed good, `ECO`, `INFO`, Sound Mode `MOVIE`, `MUSIC`, `GAME`, `PURE`; `Aux1` is redundant with existing Aux.

Already tracked or represented elsewhere: Power On/Off, Vol +/-, Mute, Bluetooth, Back, Option, Setup, arrows, Enter, PHONO, CD, TUNER, CBL/SAT, TV AUDIO, Blu-ray, AUX, GAME input, MEDIA PLAYER, Stereo/Direct/Status.

## Summary
- **❌ Failed:** 0
- **✅ Passed:** 35
- **✅ Presumed passed:** 2 (`CH/Page +/-`, awaiting a clean test context)
- **♻️ Passed duplicate:** 1 (`Input Aux1 Dup`, redundant with existing Aux)
- **🔁 Needs retest:** 6 (`Eco`, `Info`, Movie/Music/Game/Pure modes)
- **⬜ Unknown:** 3 legacy/non-RC1253 entries plus zone/sleep/DVD/HDMI-out still unmapped

## Active tests / candidate fixes

- **Round 6 passed:** Bluetooth `0x02EF`, Back `0x0022`, Option `0x03B5` using Flipper-compatible RC-1253 Kaseikyo address `0x00325441`.
- **Round 7 passed:** Power Toggle, USB, HEOS, Internet, Input 8K (formerly Aux2 candidate), Play/Pause, Prev/Tune-, Next/Tune+, Quick Select 1-4. CH/Page +/- presumed good but not directly testable yet. Aux1 is redundant with existing Aux.
- **Still needs retest:** Eco, Info, Movie/Music/Game/Pure sound modes.

## 🔁 Needs retest — RC-1253 inventory candidates

All below use `PROTO_DENON_KASEIKYO` with custom Flipper-compatible RC-1253 address `0x00325441` / source bytes `address: 41 54 32 00`.

| Button | Command | Source bytes |
|---|---:|---|
| Power Toggle | `0x0005` | `05 00 00 00` |
| Input USB | `0x02CE` | `CE 02 00 00` |
| Input HEOS | `0x02C7` | `C7 02 00 00` |
| Input Internet | `0x02E5` | `E5 02 00 00` |
| Input Aux1 Dup | `0x02D9` | `D9 02 00 00` |
| Input 8K | `0x02DA` | `DA 02 00 00` |
| Eco | `0x0368` | `68 03 00 00` |
| Info | `0x0270` | `70 02 00 00` |
| CH/Page + | `0x03B1` | `B1 03 00 00` |
| CH/Page - | `0x03B2` | `B2 03 00 00` |
| Play/Pause | `0x03E0` | `E0 03 00 00` |
| Prev/Tune - | `0x03E3` | `E3 03 00 00` |
| Next/Tune + | `0x03E4` | `E4 03 00 00` |
| Quick Select 1 | `0x0124` | `24 01 00 00` |
| Quick Select 2 | `0x0125` | `25 01 00 00` |
| Quick Select 3 | `0x0126` | `26 01 00 00` |
| Quick Select 4 | `0x0127` | `27 01 00 00` |
| Movie Mode | `0x0149` | `49 01 00 00` |
| Music Mode | `0x014A` | `4A 01 00 00` |
| Game Mode | `0x014B` | `4B 01 00 00` |
| Pure Mode | `0x0135` | `35 01 00 00` |


### Round 7 live validation — 2026-05-12 21:27 CDT

- **Passed:** Power Toggle, USB, HEOS, Internet, Play/Pause, Prev/Tune-, Next/Tune+, Quick Select 1-4.
- **Relabeled:** `Input Aux2` candidate is the physical **8K** button.
- **Duplicate:** `Input Aux1` is redundant with existing Aux.
- **Presumed:** CH/Page +/- likely works but Alec does not currently have a clean way to test it.

## ✅ Passed

### 5. Bluetooth
- **Current expected IR:** `PROTO_DENON_KASEIKYO Flipper addr=0x00325441 cmd=0x02EF`
- **Firmware protocol:** `PROTO_DENON_KASEIKYO` using custom Flipper-compatible Denon RC-1253 Kaseikyo encoder
- **Address:** `0x00325441`
- **Command:** `0x02EF`
- **Observed:** correct per Alec after round 6

### 6. Back
- **Current expected IR:** `PROTO_DENON_KASEIKYO Flipper addr=0x00325441 cmd=0x0022`
- **Firmware protocol:** `PROTO_DENON_KASEIKYO` using custom Flipper-compatible Denon RC-1253 Kaseikyo encoder
- **Address:** `0x00325441`
- **Command:** `0x0022`
- **Observed:** correct per Alec after round 6

### 7. Option
- **Current expected IR:** `PROTO_DENON_KASEIKYO Flipper addr=0x00325441 cmd=0x03B5`
- **Firmware protocol:** `PROTO_DENON_KASEIKYO` using custom Flipper-compatible Denon RC-1253 Kaseikyo encoder
- **Address:** `0x00325441`
- **Command:** `0x03B5`
- **Observed:** correct per Alec after round 6


### 0. Power On
- **Current expected IR:** `PROTO_DENON addr=0x02 cmd=0xE1`
- **Firmware protocol:** `PROTO_DENON`
- **Address:** `0x02`
- **Command:** `0xE1`
- **Observed:** has been and is still good per Alec

### 1. Power Off
- **Current expected IR:** `PROTO_DENON addr=0x02 cmd=0xE2`
- **Firmware protocol:** `PROTO_DENON`
- **Address:** `0x02`
- **Command:** `0xE2`
- **Observed:** has been and is still good per Alec

### 2. Vol +
- **Current expected IR:** `PROTO_DENON addr=0x02 cmd=0xF1`
- **Firmware protocol:** `PROTO_DENON`
- **Address:** `0x02`
- **Command:** `0xF1`
- **Observed:** good after revert

### 3. Vol -
- **Current expected IR:** `PROTO_DENON addr=0x02 cmd=0xF2`
- **Firmware protocol:** `PROTO_DENON`
- **Address:** `0x02`
- **Command:** `0xF2`
- **Observed:** good after revert

### 4. Mute
- **Current expected IR:** `PROTO_DENON addr=0x02 cmd=0xF0`
- **Firmware protocol:** `PROTO_DENON`
- **Address:** `0x02`
- **Command:** `0xF0`
- **Observed:** good at addr=0x02 cmd=0xF0

### 8. Setup
- **Current expected IR:** `PROTO_DENON addr=0x0C cmd=0xA0`
- **Firmware protocol:** `PROTO_DENON`
- **Address:** `0x0C`
- **Command:** `0xA0`
- **Observed:** correct in round 4

### 9. Up
- **Current expected IR:** `PROTO_DENON addr=0x0C cmd=0xA3`
- **Firmware protocol:** `PROTO_DENON`
- **Address:** `0x0C`
- **Command:** `0xA3`
- **Observed:** correct in round 4

### 10. Enter
- **Current expected IR:** `PROTO_DENON addr=0x02 cmd=0xE0`
- **Firmware protocol:** `PROTO_DENON`
- **Address:** `0x02`
- **Command:** `0xE0`
- **Observed:** good after revert

### 11. Right
- **Current expected IR:** `PROTO_DENON addr=0x02 cmd=0xDD`
- **Firmware protocol:** `PROTO_DENON`
- **Address:** `0x02`
- **Command:** `0xDD`
- **Observed:** good after revert

### 12. Down
- **Current expected IR:** `PROTO_DENON addr=0x0C cmd=0xA4`
- **Firmware protocol:** `PROTO_DENON`
- **Address:** `0x0C`
- **Command:** `0xA4`
- **Observed:** correct in round 4

### 13. Left
- **Current expected IR:** `PROTO_DENON addr=0x0C cmd=0x7F`
- **Firmware protocol:** `PROTO_DENON`
- **Address:** `0x0C`
- **Command:** `0x7F`
- **Observed:** correct in round 4

### 14. Input Phono
- **Current expected IR:** `PROTO_DENON addr=0x02 cmd=0xC3`
- **Firmware protocol:** `PROTO_DENON`
- **Address:** `0x02`
- **Command:** `0xC3`
- **Observed:** works correctly; old Input Tuner fired PHONO

### 15. Input CD
- **Current expected IR:** `PROTO_DENON addr=0x02 cmd=0xC4`
- **Firmware protocol:** `PROTO_DENON`
- **Address:** `0x02`
- **Command:** `0xC4`
- **Observed:** works correctly; old Input V.Aux fired CD

### 16. Input Tuner
- **Current expected IR:** `PROTO_DENON addr=0x02 cmd=0xC5`
- **Firmware protocol:** `PROTO_DENON`
- **Address:** `0x02`
- **Command:** `0xC5`
- **Observed:** items 12–20 from earlier build worked correctly per Alec 2026-05-12 19:44; index shifted after adding Setup/Up

### 17. Input CBL/SAT
- **Current expected IR:** `PROTO_DENON addr=0x02 cmd=0xC8`
- **Firmware protocol:** `PROTO_DENON`
- **Address:** `0x02`
- **Command:** `0xC8`
- **Observed:** items 12–20 from earlier build worked correctly per Alec 2026-05-12 19:44; index shifted after adding Setup/Up

### 18. Input TV Audio
- **Current expected IR:** `PROTO_DENON addr=0x02 cmd=0xC9`
- **Firmware protocol:** `PROTO_DENON`
- **Address:** `0x02`
- **Command:** `0xC9`
- **Observed:** items 12–20 from earlier build worked correctly per Alec 2026-05-12 19:44; index shifted after adding Setup/Up

### 19. Input Blu-ray
- **Current expected IR:** `PROTO_DENON addr=0x02 cmd=0xCA`
- **Firmware protocol:** `PROTO_DENON`
- **Address:** `0x02`
- **Command:** `0xCA`
- **Observed:** items 12–20 from earlier build worked correctly per Alec 2026-05-12 19:44; index shifted after adding Setup/Up

### 20. Input Aux
- **Current expected IR:** `PROTO_DENON addr=0x02 cmd=0xCC`
- **Firmware protocol:** `PROTO_DENON`
- **Address:** `0x02`
- **Command:** `0xCC`
- **Observed:** items 12–20 from earlier build worked correctly per Alec 2026-05-12 19:44; index shifted after adding Setup/Up

### 21. Input Game
- **Current expected IR:** `PROTO_DENON addr=0x02 cmd=0xCD`
- **Firmware protocol:** `PROTO_DENON`
- **Address:** `0x02`
- **Command:** `0xCD`
- **Observed:** items 12–20 from earlier build worked correctly per Alec 2026-05-12 19:44; index shifted after adding Setup/Up

### 22. Input Media
- **Current expected IR:** `PROTO_DENON addr=0x02 cmd=0xCE`
- **Firmware protocol:** `PROTO_DENON`
- **Address:** `0x02`
- **Command:** `0xCE`
- **Observed:** items 12–20 from earlier build worked correctly per Alec 2026-05-12 19:44; index shifted after adding Setup/Up

## ⬜ Unknown

### 23. Stereo
- **Current expected IR:** `PROTO_DENON addr=0x02 cmd=0xD1`
- **Firmware protocol:** `PROTO_DENON`
- **Address:** `0x02`
- **Command:** `0xD1`

### 24. Direct
- **Current expected IR:** `PROTO_DENON addr=0x02 cmd=0xD2`
- **Firmware protocol:** `PROTO_DENON`
- **Address:** `0x02`
- **Command:** `0xD2`

### 25. Status
- **Current expected IR:** `PROTO_DENON addr=0x02 cmd=0xD3`
- **Firmware protocol:** `PROTO_DENON`
- **Address:** `0x02`
- **Command:** `0xD3`
