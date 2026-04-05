# Dependency Management

## Submodules

This project uses Git submodules to pin library versions and ensure reproducible builds.

### Arduino-IRremote
**Source:** https://github.com/Arduino-IRremote/Arduino-IRremote  
**Path:** `lib/Arduino-IRremote/`  
**Purpose:** IR transmission on GPIO19

### Initial Clone (with submodules)

```bash
git clone --recurse-submodules https://github.com/thetinyclaw/av-ai-remote.git
```

### Update Submodules

```bash
# Clone without submodules first
git clone https://github.com/thetinyclaw/av-ai-remote.git
cd av-ai-remote

# Initialize and fetch submodules
git submodule update --init --recursive

# Or pull latest versions
git submodule update --remote
```

### Committing Submodule Changes

If you update a submodule to a newer commit:

```bash
cd lib/Arduino-IRremote
git fetch origin
git checkout <new-commit-hash>
cd ../..

git add lib/Arduino-IRremote
git commit -m "Bump Arduino-IRremote to <hash>"
```

---

## Arduino IDE Library Path Setup

If you're building via Arduino IDE (not PlatformIO), you may need to link the submodule into Arduino's library folder:

```bash
ln -s $(pwd)/lib/Arduino-IRremote ~/Arduino/libraries/Arduino-IRremote
```

Or manually copy `lib/Arduino-IRremote/` to your Arduino `libraries/` folder.

---

## M5StickCPlus2 Library

**Status:** Installed via Arduino IDE Library Manager (not submoduled)

If you prefer to submodule it too:
```bash
git submodule add https://github.com/m5stack/M5StickCPlus2.git lib/M5StickCPlus2
```

Currently not submoduled to keep setup simple for first-time users.
