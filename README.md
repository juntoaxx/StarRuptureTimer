# StarRuptureTimer

**Author:** Juntoaxx  
**Game:** Star Rupture  
**Version:** 1.0.0  
**License:** Public Domain — free to use, modify, and redistribute. No ownership claimed.

---

## What It Does

StarRuptureTimer adds a lightweight HUD overlay to Star Rupture that shows the current **Arcadia rupture phase** and a live **countdown timer** — so you always know exactly how long until the next phase change without having to watch the sky.

### Phases Tracked

| Phase | Default Colour | Description |
|---|---|---|
| Arcadia Stable | Teal | Safe period between waves |
| Rupture Incoming | Amber | Wave is approaching |
| Arcadia Burning | Orange-Red | Active rupture wave |
| Arcadia Cooling | Blue | Wave receding |
| Stabilizing | Teal-Grey | Regrowth after the wave |

The overlay displays the phase name, a MM:SS countdown, and a progress bar — all styled to match the game's sci-fi HUD aesthetic.

---

## Requirements

**This mod requires the AlienMod Loader.**  
You must install it before this mod will work.

- **AlienMod Loader** by AlienXAXS  
  → https://www.nexusmods.com/starrupture/mods/89  
  *(or search "AlienMod Loader" on the Star Rupture Nexus page)*

Without the mod loader installed and running, `StarRuptureTimer.dll` will not be loaded by the game.

---

## Installation

1. Install the **AlienMod Loader** if you haven't already (see link above).
2. Download `StarRuptureTimer.dll` from the Files tab on this page.
3. Place `StarRuptureTimer.dll` in:
   ```
   steamapps/common/StarRupture/StarRupture/Binaries/Win64/Plugins/
   ```
4. Launch the game. On first run the mod loader generates a config file at:
   ```
   steamapps/common/StarRupture/StarRupture/Binaries/Win64/Plugins/StarRuptureTimer/StarRuptureTimer.ini
   ```
   Edit this file to customise the overlay (see Configuration below).

---

## Configuration

All settings are in `StarRuptureTimer.ini`.  
**Changes take effect immediately** — you do not need to restart the game.

### [General]

| Key | Default | Description |
|---|---|---|
| `Enabled` | `true` | Set to `false` to hide the overlay without unloading the plugin |

### [Display]

| Key | Default | Description |
|---|---|---|
| `Alpha` | `80` | Overlay opacity. `0` = invisible, `100` = fully solid |
| `Scale` | `50` | Panel width. `0` = 200 px (narrowest), `100` = 300 px (widest) |
| `FontSize` | `1` | Text size. `1` = normal, `10` = double size |
| `PosX` | `-1` | Panel left edge in pixels. `-1` = auto (lower-left corner) |
| `PosY` | `-1` | Panel top edge in pixels. `-1` = auto (lower-left corner) |
| `Rotation` | `0` | Tilt angle. `-90` = counter-clockwise, `0` = flat, `90` = clockwise |

**Positioning examples for 1920x1080:**
```ini
; Lower-left (default)
PosX=20
PosY=1004

; Lower-right
PosX=1650
PosY=1004

; Top-left
PosX=20
PosY=20
```

### [Colors.*] — Phase Colours

Each phase has its own colour section: `Colors.Stable`, `Colors.Incoming`, `Colors.Burning`, `Colors.Cooling`, `Colors.Stabilizing`.

**Option A — Use a preset colour (easiest):**  
Set `Preset` to a number from the list below. The `R`, `G`, `B` values are ignored.

```ini
[Colors.Stable]
Preset=14   ; Teal
```

**Option B — Use a fully custom colour:**  
Set `Preset=0` and enter your own `R`, `G`, `B` values (each 0-255).

```ini
[Colors.Stable]
Preset=0
R=30
G=195
B=210
```

### Colour Preset List

| # | Name | | # | Name | | # | Name |
|---|---|---|---|---|---|---|---|
| 1 | White | | 12 | Green | | 23 | Magenta |
| 2 | LightGrey | | 13 | DarkGreen | | 24 | HotPink |
| 3 | Grey | | 14 | Teal | | 25 | Pink |
| 4 | DarkGrey | | 15 | Cyan | | 26 | Coral |
| 5 | Black | | 16 | SkyBlue | | 27 | Salmon |
| 6 | Red | | 17 | Blue | | 28 | Gold |
| 7 | DarkRed | | 18 | DarkBlue | | 29 | Bronze |
| 8 | Orange | | 19 | Navy | | 30 | Turquoise |
| 9 | Amber | | 20 | Indigo | | 31 | Mint |
| 10 | Yellow | | 21 | Purple | | 32 | Lavender |
| 11 | Lime | | 22 | Violet | | | |

---

## Testing the Overlay (F8)

Press **F8** in-game to cycle through all five phases for visual testing.  
This lets you check colours, size, and position without waiting for a real rupture event.  
F8 only changes the display — it has no effect on the actual game.

---

## Compatibility

- Tested on the local/listen-server version of Star Rupture (Early Access).
- Works on dedicated servers using the ACrWaveTimerActor fallback.
- Should remain compatible across game updates as long as the core game structures are unchanged.

---

## Credits & Sources

- **AlienXAXS** — AlienMod Loader and the StarRupture Plugin SDK  
  https://github.com/AlienXAXS/StarRupture-Plugin-SDK

- **Nhimself** — Phase detection research and reference implementation  
  https://github.com/Nhimself/starrupture_timermod  
  *(Phase polling logic was studied and adapted from this project)*

- **Dumper-7** — Unreal Engine 5 SDK generation tool used to expose game internals  
  https://github.com/Encryqed/Dumper-7

We do not claim ownership of any game data, SDK structures, or techniques sourced from the above projects.  
All credit for the underlying game belongs to the Star Rupture development team.

---

## Disclaimer — AI / Vibe Coding

> **⚠ This mod was built entirely through vibe coding using [Claude](https://claude.ai) (Anthropic AI).**
>
> That means the code was written iteratively through conversation with an AI assistant, not by a formally trained software engineer reviewing every line with deep expertise.  
> While it has been tested and is working on the developer's machine, **there is no guarantee it is free of bugs, memory issues, or unexpected behaviour.**
>
> **Use this mod entirely at your own risk.**  
> The author takes no responsibility for crashes, corrupted saves, bans, or any other issues that may arise from its use.  
> Always keep backups and stay within the game's terms of service.

## License

This mod is released to the community with no restrictions.  
You are free to **use, modify, fork, and redistribute** it however you like.  
No attribution required, though it is always appreciated.

If you improve it, share it — that is the spirit it was built in.

---

## Source Code

The full source code is available on GitHub:  
https://github.com/juntoaxx/StarRuptureTimer

---

## Bugs / Feedback

Found a bug or want to suggest something?  
Drop a comment on the Nexus page or open an issue at:  
https://github.com/juntoaxx/StarRuptureTimer/issues
