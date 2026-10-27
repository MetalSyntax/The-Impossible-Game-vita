# The Impossible Game — PS Vita Port v1.2.0

Adds automatic detection and full backward compatibility for **both the Base Game and the Level Pack**, fixing the launch crash when using `05133-the-impossible-1.5.2.apk`.

## ✨ What's New in v1.2.0

- **Universal APK Compatibility (Fix for "cant resolve JNI symbols")**:
  - The loader now resolves JNI exports dynamically with automatic fallback between the Level Pack prefix (`Java_com_flukedude_impossiblegamelevelpack_*`) and the Base Game prefix (`Java_com_flukedude_impossiblegame_*`).
  - You can now install either `05133-the-impossible-1.5.2.apk` (2 levels) or `05133-Impossible-Level-Pack.apk` (5 levels) with **zero code modifications**.
- **Dynamic UI Adaptation**:
  - **Base Game Mode (2 levels)**: The level select menu automatically shows the original 2-button layout (Fire Aura & Xbox Level) and limits the Stats and Medals pages to 3 subpages, completely avoiding texture missing errors.
  - **Level Pack Mode (5 levels)**: Scales up to all 5 levels and 6 subpages for medals and stats.
- **Save Sync Guarding**:
  - Safely bounds save synchronization to 6 medals / 2 levels when running the base game, and 12 medals / 5 levels with the Level Pack.
- **Toolchain Updates**:
  - Fixed C23 / GCC 15 toolchain issues (`ATOMIC_VAR_INIT` and FalsoJNI return types).
  - Added `-Wl,--no-warn-mismatch` in build configuration.

## 📋 Requirements

- PS Vita / PS TV running Custom Firmware (HENkaku or Enso), firmware 3.60/3.65+.
- [`kubridge`](https://github.com/TheOfficialFloW/kubridge/releases) installed as a kernel plugin.
- [`libshacccg.suprx`](https://github.com/Rinnegatamante/ShaRKBR33D/releases/latest) installed in `ur0:data/`.
- A legally obtained copy of:
  - **The Impossible Game** for Android (`05133-the-impossible-1.5.2.apk`), OR
  - **The Impossible Game: Level Pack** (`05133-Impossible-Level-Pack.apk`).

## 📦 Installation

1. Install `theimpossiblegame.vpk` with VitaShell.
2. Unzip your legally obtained `.apk` using 7-Zip, WinRAR, or any standard unzip tool (no command-line tools needed):
   - Copy `lib/armeabi-v7a/libimpossible.so` to `ux0:data/theimpossiblegame/libimpossible.so`.
   - Copy the `assets/` folder to `ux0:data/theimpossiblegame/assets/`.
   - Copy the `res/` folder to `ux0:data/theimpossiblegame/res/`.
3. *(Optional)* If you also have the **Level Pack** (`05133-Impossible-Level-Pack.apk`), unzip it and merge its contents (overwriting `libimpossible.so` and merging `res/` and `assets/`). The game will automatically detect the Level Pack and unlock all 5 levels!
