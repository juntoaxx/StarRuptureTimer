# StarRuptureTimer

**Author:** Juntoaxx  
**Game:** Star Rupture  
**Version:** 1.3.0  
**License:** Public Domain - free to use, modify, and redistribute.

---

## Overview

StarRuptureTimer adds a lightweight HUD overlay that shows:
1. Current Arcadia rupture phase
2. Countdown to the next phase change

Timer behavior is server-authoritative:
1. Timer is shown only when trusted server values are available.
2. While waiting for sync, the timer shows `XX:XX` in red.
3. Before sync, the phase label uses the Stable color preset so color changes are visible immediately.

---

## Current Features (v1.3)

1. Live rupture phase + countdown HUD.
2. F8 live sync poll.
3. F8 dismisses active alarm for the current phase cycle.
4. Repeating system-beep alarm (no WAV file dependency).
5. Preset-only phase colors (`Preset=1-32`).
6. Debug logging toggle (`DebugMode`) with log file next to the DLL.

---

## Requirements

This mod requires AlienMod Loader.

- AlienMod Loader: https://www.nexusmods.com/starrupture/mods/89

Without the loader, `StarRuptureTimer.dll` will not be loaded.

---

## Installation

1. Install AlienMod Loader.
2. Copy `StarRuptureTimer.dll` to:
   ```
   steamapps/common/StarRupture/StarRupture/Binaries/Win64/Plugins/
   ```
3. Launch the game.
4. The config file is generated at:
   ```
   steamapps/common/StarRupture/StarRupture/Binaries/Win64/Plugins/config/StarRuptureTimer.ini
   ```

---

## Configuration

All settings are in `StarRuptureTimer.ini`. Changes apply live.

### [General]

| Key | Default | Description |
|---|---|---|
| `Enabled` | `true` | Enables or disables the overlay |
| `DebugMode` | `false` | Enables debug logging to `starrupturetimer.log` |

### [Display]

| Key | Default | Description |
|---|---|---|
| `Alpha` | `80` | Overlay opacity (`0-100`) |
| `Scale` | `50` | Panel width scale (`0-100`) |
| `FontSize` | `1` | Text size (`1-10`) |
| `PosX` | `-1` | X position (`-1` = auto lower-left) |
| `PosY` | `-1` | Y position (`-1` = auto lower-left) |
| `Rotation` | `0` | Panel tilt (`-90` to `90`) |

### [Alarm]

| Key | Default | Description |
|---|---|---|
| `Enabled` | `false` | Enables phase alarm |
| `Phase` | `0` | Trigger phase (`0=Stable,1=Incoming,2=Burning,3=Cooling,4=Stabilizing`) |
| `TriggerMinutes` | `5` | Minutes before phase end |
| `TriggerSeconds` | `0` | Additional seconds before phase end |
| `BeepIntervalMs` | `1000` | Delay between beeps (`100-5000`) |

Alarm notes:
1. Alarm repeats until dismissed with F8.
2. Dismiss applies only to the current phase cycle.

### [Colors.*] Preset Colors

Sections:
1. `Colors.Stable`
2. `Colors.Incoming`
3. `Colors.Burning`
4. `Colors.Cooling`
5. `Colors.Stabilizing`

Each section uses one key:
- `Preset` (`1-32`)

Example:
```ini
[Colors.Stable]
Preset=14
```

Preset list:

| # | Name | # | Name | # | Name |
|---|---|---|---|---|---|
| 1 | White | 12 | Green | 23 | Magenta |
| 2 | LightGrey | 13 | DarkGreen | 24 | HotPink |
| 3 | Grey | 14 | Teal | 25 | Pink |
| 4 | DarkGrey | 15 | Cyan | 26 | Coral |
| 5 | Black | 16 | SkyBlue | 27 | Salmon |
| 6 | Red | 17 | Blue | 28 | Gold |
| 7 | DarkRed | 18 | DarkBlue | 29 | Bronze |
| 8 | Orange | 19 | Navy | 30 | Turquoise |
| 9 | Amber | 20 | Indigo | 31 | Mint |
| 10 | Yellow | 21 | Purple | 32 | Lavender |
| 11 | Lime | 22 | Violet |  |  |

---

## F8 Behavior

Press F8 in-game to:
1. Run an immediate live sync poll.
2. Dismiss the active alarm (if one is currently sounding).

## Enable Or Disable Alarm In-Game (F2)

You can toggle the alarm without editing the INI manually:
1. Press F2 in-game to open the mod loader menu.
2. Open the Config tab.
3. Select StarRuptureTimer.dll from the plugin list.
4. Find Alarm > Enabled.
5. Set it to true to enable the alarm, or false to disable it.
6. Close the menu. Changes apply immediately.

---

## Debug Logging

When `DebugMode=true`, logs are written to `starrupturetimer.log` next to the loaded DLL.

---

## Source

GitHub: https://github.com/juntoaxx/StarRuptureTimer

Issues: https://github.com/juntoaxx/StarRuptureTimer/issues
