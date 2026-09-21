# The Impossible Game — PS Vita Port v1.1.0

Adds full support for the official **Level Pack** add-on (3 extra levels), on top of the
playable base port from v1.0.0. Runs `libimpossible.so` directly on ARM via a *soloader* +
*FalsoJNI*, with OpenGL ES 1.1 (fixed-function pipeline) rendering through vitaGL.

## ✨ What's New

- **5 levels playable**: the 2 base levels (Fire Aura, Original Xbox Level) plus the 3 levels
  from the official Level Pack (Chaoz Fantasy, Heaven, Phazd) — confirmed working on real
  hardware with audio, input, and save data.
- **Level Pack `.so` support**: the loader now resolves the Level Pack's
  `libimpossible.so` (same 11 JNI exports, `impossiblegamelevelpack` symbol prefix instead of
  `impossiblegame`) — no source changes needed to swap between the base and Level Pack `.so`.
- **Save data expanded**: 6 → 12 medals, 2 → 5 levels of practice/no-flag progress tracking.
- **Menu scaled to 5 levels**: level select grew from 2 to 5 buttons (3+2 grid with a back
  button), Stats and Medals screens grew from 3 to 6 subpages each.
- **Per-level music**: added `BGM_SOUNDTRACK2/3/4` for Chaoz Fantasy/Heaven/Phazd; the 2 base
  levels keep using the original `soundtrack.ogg`.

## ⚠️ Known Issues / Pending Work

- None currently open. The Level Pack's `statspage4/5/6.png` assets only ship at their original
  mdpi resolution (no landscape crop like the base game's stats pages) — a minor, cosmetic
  sharpness difference only, not a functional issue.
- Updating an existing install resets `save.dat` (the save layout grew to fit 5 levels / 12
  medals) — expected, not a bug.

## 📋 Requirements

- PS Vita / PS TV running Custom Firmware (HENkaku or Enso), firmware 3.60/3.65+.
- [`kubridge`](https://github.com/TheOfficialFloW/kubridge/releases) installed as a kernel plugin.
- [`libshacccg.suprx`](https://github.com/Rinnegatamante/ShaRKBR33D/releases/latest) installed in `ur0:data/`.
- A legally obtained copy of **The Impossible Game** for Android
  (`05133-the-impossible-1.5.2.apk`, package `com.flukedude.impossiblegame`).
- *(Optional, for the 3 extra levels)* A legally obtained copy of the **Level Pack** add-on
  (`05133-Impossible-Level-Pack.apk`, package `com.flukedude.impossiblegamelevelpack`).

## 📦 Installation

1. Install `theimpossiblegame.vpk` with VitaShell.
2. Copy the contents of `ux0_data/theimpossiblegame/` to `ux0:data/theimpossiblegame/` on
   the console (see `README.md` for the full file layout).
3. **For the 3 extra levels**: unzip `05133-Impossible-Level-Pack.apk` on your PC and merge its
   `.so`, `soundtrack2/3/4.ogg`, and level-select/stats/medals PNGs into
   `ux0:data/theimpossiblegame/` — see the
   [Level Pack Installation](README.md#-level-pack-installation-5-levels-optional) section in
   `README.md` for the exact file list and paths. Without this step, the game still runs fine
   with just the 2 base levels.

---

See `README.md` for the full port description, and `port_progress.md` for the bug-by-bug
diagnosis log (every fix confirmed on real hardware, no guessing).
