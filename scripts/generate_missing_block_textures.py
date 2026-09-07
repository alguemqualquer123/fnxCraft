#!/usr/bin/env python3
"""Generate the missing block textures reported in build/logs/game.log.

The block loader (src/gameLayer/blocksLoader.cpp) looks for
    resources/assets/blocks/<name>.png
for every entry in `texturesNames[]`.  A batch of camelCase names
(ironTrapdoor.png ... placeholder68.png) had no files on disk, producing
hundreds of "error openning:" log lines at startup.

This script fills those gaps:
  1. Where a clear snake_case equivalent already exists in the same folder
     (e.g. blue_ice.png for blueIce.png) it is copied.
  2. Otherwise a simple 16x16 procedural texture is generated.
  3. The 69 placeholderN.png entries get distinct hue-wheel colors.

Usage:
    python3 scripts/generate_missing_block_textures.py          # generate
    python3 scripts/generate_missing_block_textures.py --verify # only verify
"""
import colorsys
import os
import random
import sys

from PIL import Image, ImageDraw, ImageOps

BASE = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OUT = os.path.join(BASE, "resources", "assets", "blocks")


# --------------------------------------------------------------------------
# helpers
# --------------------------------------------------------------------------

def clamp(v, lo=0, hi=255):
    return max(lo, min(hi, int(v)))


def load_source(candidates):
    """Return the first existing candidate texture (RGBA) or None."""
    for name in candidates:
        p = os.path.join(OUT, name + ".png")
        if os.path.isfile(p):
            return Image.open(p).convert("RGBA")
    return None


def tint(img, color, strength=1.0):
    """Blend img toward img * color (per channel, in [0,255])."""
    out = img.copy()
    px = out.load()
    r, g, b = color
    for y in range(out.height):
        for x in range(out.width):
            pr, pg, pb, pa = px[x, y]
            nr = pr * (r / 255.0)
            ng = pg * (g / 255.0)
            nb = pb * (b / 255.0)
            px[x, y] = (
                clamp(pr * (1 - strength) + nr * strength),
                clamp(pg * (1 - strength) + ng * strength),
                clamp(pb * (1 - strength) + nb * strength),
                pa,
            )
    return out


def desaturate(img, strength=1.0):
    gray = ImageOps.grayscale(img).convert("RGBA")
    return Image.blend(img, gray, strength)


def noise_texture(base, w=16, h=16, jitter=10, seed=0):
    rng = random.Random(seed)
    im = Image.new("RGBA", (w, h), (0, 0, 0, 255))
    px = im.load()
    for y in range(h):
        for x in range(w):
            d = rng.randint(-jitter, jitter)
            px[x, y] = (clamp(base[0] + d), clamp(base[1] + d), clamp(base[2] + d), 255)
    return im


def checker_overlay(im, factor=0.12, cell=4):
    """Darken/lighten 4x4 checker cells so the texture reads as a placeholder."""
    px = im.load()
    for y in range(im.height):
        for x in range(im.width):
            r, g, b, a = px[x, y]
            f = factor if (((x // cell) + (y // cell)) % 2) else -factor
            px[x, y] = (clamp(r * (1 + f)), clamp(g * (1 + f)), clamp(b * (1 + f)), a)
    return im


def draw_grid(im, color, step=8, width=1):
    d = ImageDraw.Draw(im)
    for i in range(0, im.width, step):
        d.rectangle([i, 0, i + width - 1, im.height - 1], fill=color)
        d.rectangle([0, i, im.width - 1, i + width - 1], fill=color)
    return im


# --------------------------------------------------------------------------
# procedural pattern generators
# --------------------------------------------------------------------------

def gen_flame(seed=7):
    im = Image.new("RGBA", (16, 16), (0, 0, 0, 0))
    px = im.load()
    rng = random.Random(seed)
    for y in range(16):
        t = y / 15.0  # 0 top, 1 bottom
        half = 15 * (0.12 + 0.38 * t) * rng.uniform(0.75, 1.15)
        cx = 8 + rng.randint(-1, 1)
        for x in range(max(0, int(cx - half)), min(16, int(cx + half) + 1)):
            r = clamp(120 + 135 * t)
            g = clamp(30 + 170 * (t ** 0.7))
            b = 30
            px[x, y] = (r, g, b, 255)
    return im


def gen_solar_panel(seed=11):
    im = noise_texture((28, 38, 78), jitter=6, seed=seed)
    draw_grid(im, (70, 95, 160), step=5, width=1)
    d = ImageDraw.Draw(im)
    for i in range(-16, 16, 6):  # subtle diagonal sheen
        d.line([i, 15, i + 16, 0], fill=(90, 120, 190), width=1)
    d.rectangle([0, 0, 15, 0], fill=(55, 70, 110))
    return im


def gen_wind_turbine(seed=13):
    im = noise_texture((120, 150, 170), jitter=8, seed=seed)
    d = ImageDraw.Draw(im)
    d.rectangle([7, 6, 8, 15], fill=(220, 220, 225))   # pole
    d.ellipse([6, 4, 9, 7], fill=(240, 240, 245))      # hub
    d.line([8, 5, 8, 0], fill=(240, 240, 245), width=1)        # blade up
    d.line([8, 5, 14, 9], fill=(235, 235, 240), width=1)       # blade right-down
    d.line([8, 5, 2, 9], fill=(235, 235, 240), width=1)        # blade left-down
    return im


def gen_water_pipe(seed=17):
    im = noise_texture((110, 112, 120), jitter=8, seed=seed)
    px = im.load()
    for y in range(16):
        for x in range(4, 12):
            if 6 <= x <= 9:
                px[x, y] = (165, 168, 175, 255)
        px[4, y] = (70, 72, 80, 255)
        px[11, y] = (70, 72, 80, 255)
    return im


def gen_conveyor_belt(seed=19):
    im = noise_texture((45, 45, 50), jitter=7, seed=seed)
    d = ImageDraw.Draw(im)
    d.rectangle([0, 6, 15, 9], fill=(80, 80, 88))
    for x in (2, 7, 12):
        d.rectangle([x, 7, x + 1, 8], fill=(220, 180, 40))
    return im


def gen_rope_elevator(seed=23):
    im = noise_texture((150, 122, 84), jitter=8, seed=seed)
    px = im.load()
    for y in range(16):
        for x0 in (2, 7, 12):
            for x in (x0, x0 + 1):
                px[x, y] = (192, 160, 110, 255)
            px[x0 - 1, y] = (110, 88, 58, 255)
            px[x0 + 2, y] = (110, 88, 58, 255)
    return im


def gen_placeholder(i, total=69):
    hue = i / float(total)
    r, g, b = colorsys.hsv_to_rgb(hue, 0.65, 0.90)
    base = (int(r * 255), int(g * 255), int(b * 255))
    im = noise_texture(base, jitter=8, seed=1000 + i)
    checker_overlay(im, factor=0.10, cell=4)
    return im


# --------------------------------------------------------------------------
# texture definitions
# --------------------------------------------------------------------------
# name -> (copy candidates, tint color or None, desaturate strength, procedural fn)

SPECS = {
    "ironTrapdoor":     (["iron_trapdoor"], None, 0, None),
    "crystalDoor":      (["amethyst_block"], None, 0, None),
    "vitralWindow":     (["magenta_stained_glass", "vitral1", "white_stained_glass"], None, 0, None),
    "ceramicTile":      (["white_glazed_terracotta", "light_gray_glazed_terracotta", "white_concrete"], None, 0, None),
    "ironBars":         (["iron_bars"], None, 0, None),
    "barbedWire":       (["iron_bars"], (150, 150, 160), 0, None),
    "powderSnow":       (["powder_snow", "snow"], None, 0, None),
    "driedMud":         (["packed_mud", "mud"], None, 0, None),
    "crackedMud":       (["mud", "dirt"], None, 0, None),
    "polishedBasalt":   (["polished_basalt_side", "basalt_side"], None, 0, None),
    "basaltPillar":     (["basalt_side", "polished_basalt_side"], None, 0, None),
    "endStone":         (["end_stone"], None, 0, None),
    "endStoneBricks":   (["end_stone_bricks"], None, 0, None),
    "purpurBlock":      (["purpur_block"], None, 0, None),
    "purpurPillar":     (["purpur_pillar", "purpur_block"], None, 0, None),
    "giantMushroom":    (["mushroom_stem", "red_mushroom_block"], None, 0, None),
    "floweringCactus":  (["cactus_side"], None, 0, None),
    "tallGrassBlock":   (["grass"], None, 0, None),
    "fernBlock":        (["grass", "dead_bush"], None, 0, None),
    "hangingRoots":     (["dead_bush"], None, 0, None),
    "giantLilyPad":     (["lily_pad"], None, 0, None),
    "coralBlock":       (["brain_coral_block", "fire_coral_block"], None, 0, None),
    "deadCoralBlock":   (["brain_coral_block", "fire_coral_block"], None, 1.0, None),
    "drySponge":        (["sponge"], None, 0, None),
    "wetSponge":        (["wet_sponge"], None, 0, None),
    "soulSand":         (["soul_sand"], None, 0, None),
    "soulSoil":         (["soul_soil"], None, 0, None),
    "blueIce":          (["blue_ice"], None, 0, None),
    "packedIce":        (["packed_ice", "ice"], None, 0, None),
    "blastFurnace":     (["blast_furnace_front", "blast_furnace_side"], None, 0, None),
    "smokerBlock":      (["smoker_front", "smoker_side"], None, 0, None),
    "stonecutterBlock": (["stonecutter_side", "stonecutter_saw", "stonecutter_bottom"], None, 0, None),
    "cartographyTable": (["cartography_table_side1", "cartography_table_side2", "cartography_table_top"], None, 0, None),
    "drumBlock":        (["note_block"], None, 0, None),
    "noteBlock":        (["note_block"], None, 0, None),
    "safeBlock":        (["iron_block"], (170, 170, 180), 0, None),
    "crystalPressurePlate": (["amethyst_block"], None, 0, None),
    "daylightSensor":   (["daylight_detector_top"], None, 0, None),
    "motionSensor":     (["daylight_detector_top"], (200, 140, 140), 0, None),
    "ghostBlock":       (["white_stained_glass"], None, 0, None),
    "cristalBruto":     (["amethyst_block"], None, 0, None),
    "cristalLapidado":  (["amethyst_block"], None, 0, None),
    "cristalLapidado_stairs": (["amethyst_block"], None, 0, None),
    "cristalLapidado_slab":   (["amethyst_block"], None, 0, None),
    "cristalLapidado_wall":   (["amethyst_block"], None, 0, None),
    "obsidianaChorona": (["crying_obsidian", "obsidian"], None, 0, None),
    "patinatedCopper":  (["exposed_copper"], None, 0, None),
    "patinatedCopperAged": (["weathered_copper"], None, 0, None),
    "patinatedCopperOxidized": (["oxidized_copper"], None, 0, None),
    "tinOre":           (["iron_ore", "stone"], (205, 205, 215), 0, None),
    "tinBlock":         (["iron_block"], (205, 205, 215), 0, None),
    "mithrilOre":       (["iron_ore", "stone"], (140, 220, 210), 0, None),
    "mithrilBlock":     (["iron_block"], (140, 220, 210), 0, None),
    "bambooBlock":      (["bamboo_stalk"], None, 0, None),
    "bambooPlanks":     (["bamboo_planks", "jungle_planks"], None, 0,
                         lambda: noise_texture((190, 170, 100), seed=31)),
    "bambooFence":      (["bamboo_stalk"], None, 0, None),
    "strawBlock":       (["hay"], None, 0, None),
    "strawSlab":        (["hay"], None, 0, None),
    "strawStairs":      (["hay"], None, 0, None),
    "burntClayBricks":  (["bricks"], (150, 110, 95), 0, None),
    "temperedGlass":    (["glass"], None, 0, None),
    "coloredVitral":    (["magenta_stained_glass"], None, 0, None),
    "slateBlock":       (["deepslate"], None, 0, None),
    "slateSlab":        (["deepslate"], None, 0, None),
    "mahoganyLog":      (["acacia_log"], None, 0, None),
    "mahoganyLeaves":   (["acacia_leaves"], None, 0, None),
    "quicksand":        (["sand"], (215, 190, 130), 0, None),
    "jellyBlock":       (["slime_block"], (90, 220, 170), 0, None),
    "mushroomTrampoline": (["red_mushroom_block"], None, 0, None),
    "strawMattress":    (["hay"], (220, 200, 140), 0, None),
    "enchantmentTable": (["enchanting_table_top", "enchanting_table_side"], None, 0, None),
    "goblinAnvil":      (["anvil"], (140, 160, 120), 0, None),
    "clothLoom":        (["loom_top", "loom_side"], None, 0, None),
    "composterBlock":   (["composter_bottom", "composter_side", "composter_compost"], None, 0, None),
    "oakBarrel":        (["barrel_side"], None, 0, None),
    "paperLantern":     (["lantern"], (240, 230, 200), 0, None),
    "lightPost":        (["iron_bars"], None, 0, None),
    "solarPanel":       ([], None, 0, gen_solar_panel),
    "windTurbine":      ([], None, 0, gen_wind_turbine),
    "waterPipe":        ([], None, 0, gen_water_pipe),
    "conveyorBelt":     ([], None, 0, gen_conveyor_belt),
    "ropeElevator":     ([], None, 0, gen_rope_elevator),
    "drawbridge":       (["oak_planks"], (170, 170, 180), 0, None),
    "wetTorch":         (["torch"], (130, 170, 230), 0, None),
    "torchUnlit":       (["torch"], (110, 110, 110), 0, None),
    "fire":             ([], None, 0, gen_flame),
}

PLACEHOLDER_COUNT = 69  # placeholder0 ... placeholder68


def build(name, spec):
    candidates, tint_color, desat, procedural = spec
    img = load_source(candidates)
    if img is not None:
        if desat:
            img = desaturate(img, desat)
        if tint_color:
            img = tint(img, tint_color)
        source = "copied"
    elif procedural is not None:
        img = procedural()
        source = "generated"
    else:
        img = noise_texture((160, 60, 160), seed=hash(name) & 0xFFFF)
        source = "fallback-generated"
    img.save(os.path.join(OUT, name + ".png"), "PNG")
    return source


def main():
    os.makedirs(OUT, exist_ok=True)

    names = list(SPECS.keys()) + ["placeholder%d" % i for i in range(PLACEHOLDER_COUNT)]

    if "--verify" in sys.argv:
        missing = [n for n in names if not os.path.isfile(os.path.join(OUT, n + ".png"))]
        if missing:
            print("MISSING %d:" % len(missing))
            for n in missing:
                print("  " + n)
            sys.exit(1)
        print("OK: all %d textures present." % len(names))
        return

    stats = {"copied": 0, "generated": 0, "fallback-generated": 0}
    for name in names:
        spec = SPECS.get(name)
        if spec is None:
            idx = int(name.replace("placeholder", ""))
            spec = ([], None, 0, lambda i=idx: gen_placeholder(i))
        source = build(name, spec)
        stats[source] += 1
        print("%-28s %s" % (name, source))

    print("\nDone. copied=%d generated=%d fallback=%d"
          % (stats["copied"], stats["generated"], stats["fallback-generated"]))


if __name__ == "__main__":
    main()