# Control Modes

Two firmware variants for different button configurations, each in its own Arduino project folder.

---

## Mode 1: Three-Button Menu

**Folder:** `av-remote/`  
**File:** `av-remote.ino`

**Use if:** Your M5StickC PLUS2 has accessible buttons A, B, C

**Controls:**
- **Button A (GPIO37):** Navigate up
- **Button B (GPIO39):** Navigate down  
- **Button C (GPIO35):** Select / Execute / Back

**UI:** Full hierarchical menu with command list visible

**Pros:**
- Most intuitive navigation
- Full visual menu layout
- Best for power users

**Cons:**
- Requires 3 buttons accessible

---

## Mode 2: Single-Button Click/Double-Click

**Folder:** `av-remote-single-button/`  
**File:** `av-remote-single-button.ino`

**Use if:** Only one button is practically accessible (Button C)

**Controls:**
- **Single Click:** Cycle to next command
- **Double Click** (within 300ms): Send IR code for current command
- **Long Press** (1+ second): Go back (cycle previous)

**UI:** Large command name, carousel-style with prev/next hints

**Pros:**
- Single-button simplicity
- Minimal wrist movement
- Wearable-friendly

**Cons:**
- Requires double-clicking for execution (slightly slower)

---

## How to Upload (Arduino IDE)

### Mode 1 (Three-Button)

1. Open Arduino IDE
2. File → Open → Navigate to `av-remote/av-remote.ino`
3. Select your board: Tools → Board → ESP32 → M5StickCPlus2
4. Select port: Tools → Port → `/dev/cu.usbserial-*` (macOS) or `COM*` (Windows)
5. Upload: Sketch → Upload (or Ctrl+U / Cmd+U)

### Mode 2 (Single-Button)

1. Open Arduino IDE
2. File → Open → Navigate to `av-remote-single-button/av-remote-single-button.ino`
3. Same steps as above (board, port, upload)

---

## Hardware Reality Check

**M5StickC PLUS2 buttons:**
- Button A (GPIO37) — YES, accessible via side
- Button B (GPIO39) — YES, accessible via side  
- Button C (GPIO35) — YES, wake/power button (always accessible)

If you can reach all 3 buttons comfortably, use **Mode 1 (three-button)**.  
If only Button C is practical (wearable scenario), use **Mode 2 (single-button)**.

---

## Troubleshooting

**"Sketch file was not found"**  
→ Make sure you're opening the `.ino` file from within its folder (e.g., `av-remote/av-remote.ino`)

**"Board not found"**  
→ Install M5StickCPlus2 library via Sketch → Include Library → Manage Libraries

**"Port not recognized"**  
→ Reinstall CH9102 driver (see docs/HARDWARE_SETUP.md)
