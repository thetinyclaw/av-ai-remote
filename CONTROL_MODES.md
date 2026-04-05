# Control Modes

Two firmware variants for different button configurations.

---

## Mode 1: Three-Button Menu (av-remote.ino)

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

## Mode 2: Single-Button Click/Double-Click (av-remote-single-button.ino)

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

## Quick Start

### Mode 1 (Three-Button)
```cpp
// Use default sketch
// src/av-remote.ino
```

1. Upload via Arduino IDE
2. Button A/B = navigate, Button C = select
3. Done

### Mode 2 (Single-Button)
```cpp
// Use single-button variant
// src/av-remote-single-button.ino
```

1. Rename/replace with `av-remote.ino`
2. Upload via Arduino IDE
3. Single click to cycle, double-click to send

---

## Switching Modes

```bash
# Mode 1 (three-button)
cp src/av-remote.ino src/av-remote.ino.bak
# Use original

# Mode 2 (single-button)
cp src/av-remote-single-button.ino src/av-remote.ino
# Now upload
```

---

## Hardware Reality Check

**M5StickC PLUS2 buttons:**
- Button A (GPIO37) — YES, accessible via side
- Button B (GPIO39) — YES, accessible via side  
- Button C (GPIO35) — YES, wake/power button (always accessible)

If you can reach all 3 buttons comfortably, use **Mode 1 (three-button)**.  
If only Button C is practical (wearable scenario), use **Mode 2 (single-button)**.
