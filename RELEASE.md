# The Impossible Game — PS Vita Port v1.3.1

Hotfix for v1.3.0: some saves unlocked trophies the player never earned on the first launch.

## 🐛 Fixed in v1.3.1

- **Phantom trophies on first launch**: saves written by older builds could hold garbage values (e.g. `38`, `20`, `15`) in medal slots. The retroactive unlock treated any non-zero value as earned, so it unlocked e.g. *SIN BANDERAS 3*, *PRACTICA 4* and *SIN BANDERAS 4* right after boot. The game only ever stores `1` in a medal slot, so any other value is now reset to `0` when the save loads (and logged as `Save medal[i]=N corrupt`).
- Trophies that v1.3.0 already unlocked by mistake stay unlocked. The Vita can't re-lock a trophy; you can only delete the game's trophy data.

---

# v1.3.0

Adds **native PS Vita trophies**: every in-game medal now also unlocks a system trophy, with a Platinum for collecting them all.

---

## 🏆 New in v1.3.0: Trophies

- **13 trophies** (1 Platinum, 5 Gold, 1 Silver, 6 Bronze), one per native medal:

| Trophy | Type | How to get it |
|---|---|---|
| LO IMPOSIBLE | Platinum | Unlock every other trophy |
| PERSEVERANCIA | Silver | 500 attempts without quitting |
| SALTARIN | Bronze | 999 jumps in total |
| PRACTICA 1-5 | Bronze | Finish Level 1 / Xbox Level / Level 2 / Level 3 / Level 4 in Practice Mode |
| SIN BANDERAS 1-5 | Gold | Finish that level with no flags |

- **Retroactive unlocks**: medals already in your save unlock their trophies on the next launch.
- Trophy icons are the game's own medal art.
- ⚠️ The **base game** only has 6 medals (Level 1 + Xbox Level), so the Platinum needs the **Level Pack**.
- Requires [**NoTrpDrm**](https://github.com/Rinnegatamante/NoTrpDrm/releases) under `*main` in `ur0:tai/config.txt` (**reboot** after adding it). Install the **full VPK** — trophies live in `sce_sys/` and are not installed by copying only `eboot.bin`.

---

## 🚨 Critical Fixes (from v1.2.1)

### 1. Fix for White Worlds / Missing Textures
- **Stack Alignment Fix in `ImpossibleGame_initGame`**: In previous builds, passing `fps` as a `jfloat` under modern hardfp toolchains caused the compiler to place it in VFP register `s0`, skipping a stack slot. `libimpossible.so` (compiled under softfp AAPCS) expected `fps` at `[sp, 4]`, which shifted the texture ID arguments: the main texture atlas (`gametextures.png`) received the second texture ID and the second received garbage, leaving game textures unbound and rendering obstacles and background as blank white quads. Passing `fps` as a 32-bit bitcasted word preserves exact stack layout.
- **Fixed-Function GLES1 Float Bridge**: Calls to `glColor4f`, `glClearColor`, `glTexEnvf`, `glPointSize`, `glTranslatef`, and `glRotatef` from `libimpossible.so` now pass through softfp-to-hardfp integer bridge wrappers, ensuring color and transform values are correctly delivered to VitaGL.

### 2. Fix for Total Audio Silence
- **ABI Mismatch in Math Functions**: `stb_vorbis` was passing float arguments in integer registers (`r0-r3`), while VitaSDK GCC 15 `libm.a` (`sin`, `cos`, `pow`, `floor`) expected them in VFP registers (`s0-s15`). This resulted in math functions reading uninitialized registers and returning NaNs/0, causing audio decoding to output total silence.
- Rebuilding the project natively with `-mfloat-abi=hard` and linking against hardfp runtime libraries completely restores BGM and SFX audio playback.

### 3. ABI Bridge Wrappers (`source/dynlib.c`)
- Implemented dedicated softfp-to-hardfp bridge wrappers for all 10 external float functions imported by `libimpossible.so`:
  - Math: `sinf`, `cosf`, `truncf`, `round`
  - GLES1: `glClearColor`, `glColor4f`, `glTexEnvf`, `glPointSize`, `glTranslatef`, `glRotatef`
- Removed `-Wl,--no-warn-mismatch` entirely from CMake; all project components now compile and link cleanly with `-mfloat-abi=hard`.

---

## ✨ Features & Universal Compatibility (from v1.2.0)

- **Universal APK Auto-Detection**:
  - Dynamically resolves JNI exports with automatic fallback between the Level Pack prefix (`Java_com_flukedude_impossiblegamelevelpack_*`) and the Base Game prefix (`Java_com_flukedude_impossiblegame_*`).
  - Drop in either `05133-the-impossible-1.5.2.apk` (2 levels) or `05133-Impossible-Level-Pack.apk` (5 levels) — no code changes or separate builds needed.
- **Adaptive Menus & UI**:
  - **Base Game Mode (2 levels)**: Automatically displays the original 2-button level selector (Fire Aura & Original Xbox Level) and restricts Stats & Medals to 3 subpages, eliminating missing texture errors.
  - **Level Pack Mode (5 levels)**: Automatically enables the full 5-level grid and 6 subpages for Medals and Stats.
- **Bounded Save Synchronization**:
  - Syncs up to 6 medals / 2 levels when running the base game, and 12 medals / 5 levels with the Level Pack.
- **Toolchain Modernization**:
  - Full compatibility with modern VitaSDK (GCC 15.2.0 / C23 standards).
  - Resolved `ATOMIC_VAR_INIT` deprecation in `logger.c`.
  - Corrected return types in FalsoJNI (`GetPrimitiveArrayCritical`, `jda_sizeof`).
- **Persistent Diagnostics Logging**:
  - File logger (`ux0:data/theimpossiblegame/logs/log_XXX.log`) enabled unconditionally for runtime diagnostics.

---

## 📋 Requirements

- PS Vita / PS TV running Custom Firmware (**HENkaku** or **Enso**), firmware 3.60/3.65+.
- [**kubridge**](https://github.com/TheOfficialFloW/kubridge/releases) installed as a kernel plugin (`ur0:tai/config.txt`).
- [**NoTrpDrm**](https://github.com/Rinnegatamante/NoTrpDrm/releases) under `*main` in `ur0:tai/config.txt` (only needed for trophies).
- [**libshacccg.suprx**](https://github.com/Rinnegatamante/ShaRKBR33D/releases/latest) installed in `ur0:data/`.
- A legally obtained copy of:
  - **The Impossible Game** for Android (`05133-the-impossible-1.5.2.apk`), **OR**
  - **The Impossible Game: Level Pack** (`05133-Impossible-Level-Pack.apk`).

---

## 📦 Installation Instructions

1. Install `theimpossiblegame.vpk` on your PS Vita using VitaShell.
2. Open your legally obtained `.apk` using any zip tool (7-Zip, WinRAR, etc.):
   - Copy `lib/armeabi-v7a/libimpossible.so` to `ux0:data/theimpossiblegame/libimpossible.so`.
   - Copy the `assets/` folder to `ux0:data/theimpossiblegame/assets/`.
   - Copy the `res/` folder to `ux0:data/theimpossiblegame/res/`.
3. *(Optional)* If you own the **Level Pack** (`05133-Impossible-Level-Pack.apk`), simply copy and overwrite `libimpossible.so` and merge the `assets/` and `res/` folders into `ux0:data/theimpossiblegame/`. The game will automatically detect all 5 levels.
