#!/usr/bin/env python3
"""
Generate trophy icons + extras/trophy/trophies.json for The Impossible Game,
then pack with the psvita-trophies skill:
    python3 extras/scripts/generate_trophies.py
    python3 ~/.claude/skills/psvita-trophies/scripts/make_trp.py
Trophies 1..12 map 1:1 to the game's 12 native medals (medal i -> trophy i+1);
0 is the Platinum. Icons are cropped from the game's own medal art.
"""

import json
import os
from PIL import Image

DRAWABLE = "ux0_data/theimpossiblegame/res/drawable"
ICONS = "extras/trophy/icons"
LEVELS = ["el Nivel 1", "el Nivel Xbox", "el Nivel 2", "el Nivel 3", "el Nivel 4"]

trophies = [
    {"ttype": "P", "name": "LO IMPOSIBLE", "detail": "Consigue todas las medallas del juego."},
    {"ttype": "S", "name": "PERSEVERANCIA", "detail": "500 intentos sin abandonar."},
    {"ttype": "B", "name": "SALTARIN", "detail": "999 saltos en total."},
]
for i, lvl in enumerate(LEVELS):
    trophies.append({"ttype": "B", "name": f"PRACTICA {i + 1}", "detail": f"Termina {lvl} en modo practica."})
    trophies.append({"ttype": "G", "name": f"SIN BANDERAS {i + 1}", "detail": f"Termina {lvl} sin banderas."})

os.makedirs(ICONS, exist_ok=True)
# Each medalspageNl.png holds 2 medals (left/right): crop the round medal, drop the ribbon.
for page in range(6):
    sheet = Image.open(f"{DRAWABLE}/medalspage{page + 1}l.png").convert("RGBA")
    for side in range(2):
        x0 = 0 if side == 0 else sheet.width * 300 // 554
        col = sheet.crop((x0, sheet.height // 2, x0 + 90, sheet.height))
        medal = col.crop(col.getbbox())
        medal = medal.crop((0, medal.height - medal.width, medal.width, medal.height))
        n = page * 2 + side + 1
        path = f"{ICONS}/TROP{n:03d}.PNG"
        medal.resize((208, 208), Image.Resampling.LANCZOS).save(path)
        trophies[n]["art"] = path

cfg = {
    "comm_id": "TIGV00001",
    "title": "The Impossible Game",
    "detail": "The Impossible Game PlayStation Vita Port",
    "icon": "extras/livearea/icon0.png",
    "banner": "The_Impossible_Game_logo.jpg",
    "trophies": trophies,
}
with open("extras/trophy/trophies.json", "w") as f:
    json.dump(cfg, f, indent=2)
print("Wrote extras/trophy/trophies.json + icons")
