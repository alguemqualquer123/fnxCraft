#!/usr/bin/env python3
"""
PBR Texture Agent — fnxCraft
============================
Gera os mapas PBR (_n normal, _s material, _b altura/parallax) para TODAS as
texturas de bloco registradas em `texturesNames[]` (src/gameLayer/blocksLoader.cpp)
que ainda não os possuem. Nunca sobrescreve mapas existentes.

Convenções (validadas empiricamente contra dirt_n / anvil_s e defaultShader.frag):
  _n : normal map tangent-space, "flat" = RGB(127,127,255).
       R = 0.5 - dH/dx, G = 0.5 - dH/dy (espaço de imagem), B = z.
       Alpha herdado do albedo (folhas/plantas), igual ao oak_leaves_n.
  _s : R = smoothness (shader: roughness = (1-R)^2)
       G = metallic   (shader: metallic  = pow(G/255, 0.5))
       B = emissive   (shader: B > 0.085 ativa scroll UV de fogo
                       -> somente fire/lava/magma recebem B alto)
  _b : altura para parallax occlusion mapping; BRANCO = alto
       (depth = 1 - r no shader). 255 = plano (sem deslocamento).

Uso:
  python3 scripts/agents/pbr_texture_agent.py                # gera faltantes
  python3 scripts/agents/pbr_texture_agent.py --verify       # só relatório
  python3 scripts/agents/pbr_texture_agent.py --force dirt   # regenera 1 textura
"""
import argparse
import hashlib
import os
import re
import sys

import numpy as np
from PIL import Image

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
BLOCKS_DIR = os.path.join(ROOT, "resources", "assets", "blocks")
LOADER_CPP = os.path.join(ROOT, "src", "gameLayer", "blocksLoader.cpp")
SIZE = 128  # resolução padrão dos assets existentes


# --------------------------------------------------------------------------
# Parsing do loader (fonte da verdade)
# --------------------------------------------------------------------------

def parse_texture_names():
    src = open(LOADER_CPP, encoding="utf-8", errors="replace").read()
    m = re.search(r"const char\s*\*\s*texturesNames\[\]\s*=\s*\{(.*?)\};", src, re.S)
    if not m:
        raise SystemExit("ERRO: texturesNames[] não encontrado em blocksLoader.cpp")
    return re.findall(r'"([^"]*)"', m.group(1))


# --------------------------------------------------------------------------
# Ruído procedural tileable
# --------------------------------------------------------------------------

def _seed(name):
    return int(hashlib.md5(name.encode()).hexdigest()[:8], 16)


def _smooth(t):
    return t * t * (3.0 - 2.0 * t)


def value_noise(size, cells, seed):
    """Value noise tileável em [0,1]."""
    rng = np.random.default_rng(seed)
    grid = rng.random((cells, cells)).astype(np.float32)
    ys, xs = np.mgrid[0:size, 0:size].astype(np.float32)
    fx = xs / size * cells
    fy = ys / size * cells
    x0 = np.floor(fx).astype(np.int64)
    y0 = np.floor(fy).astype(np.int64)
    tx = _smooth(fx - x0)
    ty = _smooth(fy - y0)
    x0w = x0 % cells
    y0w = y0 % cells
    x1w = (x0 + 1) % cells
    y1w = (y0 + 1) % cells
    a = grid[y0w, x0w]
    b = grid[y0w, x1w]
    c = grid[y1w, x0w]
    d = grid[y1w, x1w]
    top = a * (1 - tx) + b * tx
    bot = c * (1 - tx) + d * tx
    return top * (1 - ty) + bot * ty


def fbm(size, seed, octaves=4, base_cells=4, persistence=0.55):
    out = np.zeros((size, size), np.float32)
    amp, total, cells = 1.0, 0.0, base_cells
    for o in range(octaves):
        out += amp * value_noise(size, cells, seed + o * 7919)
        total += amp
        amp *= persistence
        cells *= 2
    return out / total


def voronoi(size, cells, seed):
    """Voronoi tileável. Retorna (dist1, borda[0..1], id da célula)."""
    rng = np.random.default_rng(seed)
    pts = rng.random((cells, cells, 2)).astype(np.float32)
    ys, xs = np.mgrid[0:size, 0:size].astype(np.float32)
    fx = xs / size * cells
    fy = ys / size * cells
    ix = np.floor(fx).astype(np.int64)
    iy = np.floor(fy).astype(np.int64)
    d1 = np.full((size, size), 1e9, np.float32)
    d2 = np.full((size, size), 1e9, np.float32)
    cid = np.zeros((size, size), np.int64)
    for dy in (-1, 0, 1):
        for dx in (-1, 0, 1):
            gx = (ix + dx) % cells
            gy = (iy + dy) % cells
            px = gx + pts[gy, gx, 0] + dx
            py = gy + pts[gy, gx, 1] + dy
            d = np.sqrt((fx - px) ** 2 + (fy - py) ** 2)
            better = d < d1
            d2 = np.where(better, d1, np.minimum(d2, d))
            cid = np.where(better, gy * cells + gx, cid)
            d1 = np.where(better, d, d1)
    edge = np.clip((d2 - d1) * cells * 1.2, 0.0, 1.0)
    return d1, edge, cid


# --------------------------------------------------------------------------
# Classificação de material por nome
# --------------------------------------------------------------------------

def classify(name):
    """Retorna parâmetros do material detectado no nome da textura."""
    n = name.lower()
    base = n.split("/")[-1]
    cls = {
        "kind": "rock",        # gerador de altura
        "bump": 1.0,           # força do relevo (normal)
        "smooth": 0.14,        # canal R do _s
        "metallic": 0.0,       # canal G do _s
        "emissive": 0.0,       # canal B do _s
        "flat_b": False,       # _b branco puro (sem parallax)
        "alpha_mask": False,   # _n herda alpha do albedo
        "normal_strength": 2.2,
    }
    def has(*kws):
        return any(k in base for k in kws)

    # --- ordem importa: mais específico primeiro ---
    if has("fire", "lava", "magma"):
        cls.update(kind="ember", smooth=0.30, emissive=1.0, bump=0.7,
                   normal_strength=1.6, flat_b=True)  # sem POM em blocos animados
    elif has("leaves", "leaf", "pine_needles"):
        cls.update(kind="leaf", smooth=0.08, bump=0.8, alpha_mask=True,
                   flat_b=True, normal_strength=2.0)
    elif base.startswith("water") or has("waterfall"):
        cls.update(kind="flat", smooth=0.55, flat_b=True)
    elif has("grass_block", "path_block", "pathblock", "farmland"):
        cls.update(kind="soil", smooth=0.06, bump=0.9, normal_strength=2.4)
    elif has("glass", "vitral", "window", "ice"):
        cls.update(kind="flat", smooth=0.72, flat_b=True)
    elif has("log", "stem", "crate", "barrel", "drum"):
        cls.update(kind="wood_v", smooth=0.12, bump=0.9, normal_strength=2.6)
    elif has("plank", "bamboo", "board"):
        cls.update(kind="wood_h", smooth=0.12, bump=0.9, normal_strength=2.4)
    elif has("brick"):
        cls.update(kind="brick", smooth=0.14, bump=1.0, normal_strength=2.8)
    elif has("tile"):
        cls.update(kind="tile", smooth=0.30, bump=1.0, normal_strength=2.6)
    elif has("ore"):
        cls.update(kind="ore", smooth=0.20, metallic=0.55, bump=1.1,
                   normal_strength=3.0)
    elif has("anvil", "bars", "cauldron", "block", "ingot", "chain",
             "patinatedcopper", "hopper", "rail", "pressureplate"):
        cls.update(kind="metal", smooth=0.42, metallic=0.85, bump=0.6,
                   normal_strength=1.8)
    elif has("crystal", "cristal", "gem", "amethyst", "quartz", "diamond",
             "emerald", "ruby", "sapphire", "jelly"):
        cls.update(kind="crystal", smooth=0.65, bump=1.2, normal_strength=3.2)
    elif has("fabric", "cloth", "wool", "mattress", "carpet", "loom",
             "straw", "sponge"):
        cls.update(kind="fabric", smooth=0.03, bump=0.6, normal_strength=1.6)
    elif has("sand", "gravel", "dirt", "mud", "soil", "soul", "clay",
             "terracotta", "powder", "quicksand"):
        cls.update(kind="soil", smooth=0.06, bump=0.9, normal_strength=2.4)
    elif has("cactus"):
        cls.update(kind="cactus", smooth=0.18, bump=0.9, normal_strength=2.4)
    elif has("grass", "rose", "flower", "bush", "fern", "plant", "crop",
             "sapling", "wheat", "roots", "vine", "sprout", "bud",
             "dead_bush", "lily", "mushroom", "corn"):
        cls.update(kind="plant", smooth=0.10, flat_b=True, alpha_mask=True,
                   bump=0.4, normal_strength=1.2)
    elif has("cobble", "boulder"):
        cls.update(kind="cobble", smooth=0.12, bump=1.2, normal_strength=3.0)
    elif has("lamp", "glow", "torch", "lantern", "beacon"):
        cls.update(kind="rock", smooth=0.30, bump=0.8)
    elif has("stone", "granite", "marble", "basalt", "slate",
             "endstone", "purpur", "deepslate", "dripstone", "tuff",
             "calcite", "lime", "obsidian", "rock", "concrete"):
        cls.update(kind="rock", smooth=0.15, bump=1.0)
    elif has("wood"):
        cls.update(kind="wood_h", smooth=0.12, bump=0.9, normal_strength=2.4)
    elif has("placeholder", "controll"):
        cls.update(kind="placeholder", smooth=0.12, flat_b=True, bump=0.5,
                   normal_strength=1.4)
    else:
        cls.update(kind="rock", smooth=0.13, bump=0.8, normal_strength=2.0)

    # blocos metálicos puros conhecidos (nome exato)
    if base in ("gold_block", "iron_block", "copper_block", "tin_block",
                "mithril_block", "lead_block", "silver_block",
                "mithrilblock", "tinblock", "anvil", "goblinanvil"):
        cls.update(kind="metal", smooth=0.45, metallic=0.9, bump=0.6,
                   normal_strength=1.8)
    return cls


# --------------------------------------------------------------------------
# Geradores de campo de altura (0..1, tileable)
# --------------------------------------------------------------------------

def _groove(dist, half_width, depth):
    """Sulco suave: 0 longe, `depth` no centro do sulco."""
    return depth * np.clip(1.0 - np.abs(dist) / half_width, 0.0, 1.0) ** 1.5


def gen_height(name, cls):
    """Campo de altura H em [0,1] com wrap (tileable)."""
    s = _seed(name)
    base = name.lower().split("/")[-1]
    kind = cls["kind"]
    size = SIZE

    if kind in ("flat", "plant", "placeholder"):
        return np.ones((size, size), np.float32)

    if kind == "rock":
        return np.clip(0.35 + 0.65 * fbm(size, s, 4, 4), 0, 1)

    if kind == "soil":
        fine = fbm(size, s, 3, 12)
        big = fbm(size, s + 31, 2, 4)
        return np.clip(0.55 + (fine - 0.5) * 0.7 + (big - 0.5) * 0.44, 0, 1)

    if kind in ("wood_v", "wood_h"):
        warp = fbm(size, s, 3, 5) * 2.4
        if kind == "wood_v":   # grãos verticais: varia com x
            coord = np.linspace(0, np.pi * 7, size).astype(np.float32)
            coord = np.tile(coord, (size, 1)) + warp * 2.2
        else:                  # grãos horizontais: varia com y
            coord = np.linspace(0, np.pi * 7, size).astype(np.float32)
            coord = np.tile(coord.reshape(size, 1), (1, size)) + warp * 2.2
        grain = 0.5 + 0.5 * np.sin(coord)
        fine = fbm(size, s + 77, 3, 10) * 0.25
        h = 0.45 + 0.40 * grain + fine
        yy, _ = np.mgrid[0:size, 0:size].astype(np.float32)
        if kind == "wood_h":   # junta entre tábuas
            row = size // 2
            d = ((yy - row) % size + size) % size - size / 2
            h -= _groove(d, 2.5, 0.35)
        return np.clip(h, 0, 1)

    if kind == "brick":
        yy, xx = np.mgrid[0:size, 0:size].astype(np.float32)
        rows = 4
        row_h = size / rows
        row = np.floor(yy / row_h)
        offset = (row % 2) * row_h  # meia-tijolo alternado
        xd = ((xx + offset) % row_h) - row_h / 2
        yd = (yy % row_h) - row_h / 2
        mortar = np.minimum(_groove(xd, 2.0, 0.5), _groove(yd, 2.0, 0.5))
        tex = fbm(size, s, 3, 6)
        return np.clip(0.72 + (tex - 0.5) * 0.36 - mortar, 0, 1)

    if kind == "tile":
        yy, xx = np.mgrid[0:size, 0:size].astype(np.float32)
        cell = size / 2
        xd = (xx % cell) - cell / 2
        yd = (yy % cell) - cell / 2
        g = np.minimum(_groove(xd, 2.0, 0.4), _groove(yd, 2.0, 0.4))
        tex = fbm(size, s, 3, 8)
        return np.clip(0.78 + (tex - 0.5) * 0.24 - g, 0, 1)

    if kind == "cobble":
        d1, edge, _ = voronoi(size, 4, s)
        tex = fbm(size, s + 13, 3, 8)
        dome = 1.0 - np.clip(d1 * 1.4, 0, 1) * 0.55
        return np.clip(dome - (1.0 - edge) * 0.45 + (tex - 0.5) * 0.22, 0, 1)

    if kind == "ore":
        rng = np.random.default_rng(s + 5)
        yy, xx = np.mgrid[0:size, 0:size].astype(np.float32)
        h = 0.45 + 0.5 * fbm(size, s, 4, 4)
        for _ in range(int(rng.integers(5, 9))):
            cx, cy = rng.random(2) * size
            r = rng.uniform(6.0, 13.0)
            dx = ((xx - cx + size / 2) % size) - size / 2
            dy = ((yy - cy + size / 2) % size) - size / 2
            d = np.sqrt(dx * dx + dy * dy)
            h = np.maximum(h, np.clip(1.05 - (d / r) ** 2 * 0.55, 0, 1))
        return np.clip(h, 0, 1)

    if kind == "crystal":
        d1, edge, cid = voronoi(size, 5, s)
        rng = np.random.default_rng(s)
        heights = rng.uniform(0.45, 1.0, size=size * size)
        facet = heights[cid.ravel()].reshape(size, size).astype(np.float32)
        return np.clip(facet - (1.0 - edge) * 0.5, 0, 1)

    if kind == "fabric":
        yy, xx = np.mgrid[0:size, 0:size].astype(np.float32)
        weave = np.sin(xx * np.pi * 0.25) * np.cos(yy * np.pi * 0.25)
        fine = fbm(size, s, 2, 16) * 0.15
        return np.clip(0.55 + weave * 0.18 + fine, 0, 1)

    if kind == "leaf":
        d1, edge, cid = voronoi(size, 6, s)
        rng = np.random.default_rng(s)
        bump = rng.uniform(0.6, 1.0, size=size * size)
        dome = bump[cid.ravel()].reshape(size, size) * (1.0 - np.clip(d1 * 1.2, 0, 1) * 0.5)
        return np.clip(dome + (edge - 0.5) * 0.2, 0, 1)

    if kind == "ember":
        ridged = 1.0 - np.abs(fbm(size, s, 4, 5) - 0.5) * 2
        return np.clip(0.4 + ridged * 0.55, 0, 1)

    if kind == "cactus":
        _, xx = np.mgrid[0:size, 0:size].astype(np.float32)
        ridges = 0.5 + 0.5 * np.sin(xx * np.pi * 16 / size)
        fine = fbm(size, s, 2, 10) * 0.2
        return np.clip(0.45 + ridges * 0.35 + fine, 0, 1)

    if kind == "metal":
        yy, xx = np.mgrid[0:size, 0:size].astype(np.float32)
        panel = size / 2
        xd = (xx % panel) - panel / 2
        yd = (yy % panel) - panel / 2
        seam = np.minimum(_groove(xd, 1.5, 0.3), _groove(yd, 1.5, 0.3))
        tex = fbm(size, s, 2, 6) * 0.08
        return np.clip(0.85 - seam + tex, 0, 1)

    return np.clip(0.5 + (fbm(size, s) - 0.5), 0, 1)


# --------------------------------------------------------------------------
# Geração dos mapas finais
# --------------------------------------------------------------------------

def gen_normal_map(H, cls, alpha=None):
    """Normal map tangent-space (R=0.5-dx, G=0.5-dy, B=z) — convenção dirt_n."""
    s = cls["normal_strength"] * cls["bump"]
    gx = (np.roll(H, -1, axis=1) - np.roll(H, 1, axis=1)) * 0.5   # wrap = tileable
    gy = (np.roll(H, -1, axis=0) - np.roll(H, 1, axis=0)) * 0.5
    nx = -gx * s
    ny = -gy * s
    nz = np.ones_like(H)
    length = np.sqrt(nx * nx + ny * ny + nz * nz) + 1e-6
    r = (0.5 + nx / length * 0.5)
    g = (0.5 + ny / length * 0.5)
    b = (0.5 + nz / length * 0.5)
    out = np.dstack([r, g, b, np.ones_like(H)]).astype(np.float32)
    if alpha is not None:
        out[:, :, 3] = alpha
    return (np.clip(out, 0, 1) * 255).astype(np.uint8)


def gen_material_map(cls):
    """_s: R=smoothness, G=metallic, B=emissive."""
    size = SIZE
    r = np.full((size, size), cls["smooth"], np.float32)
    g = np.full((size, size), cls["metallic"], np.float32)
    b = np.full((size, size), cls["emissive"], np.float32)
    # leve variação para evitar flat shading uniforme
    var = (fbm(size, 12345, 2, 8) - 0.5) * 0.05
    r = np.clip(r + var, 0, 1)
    return (np.dstack([r, g, b, np.ones((size, size))]) * 255).astype(np.uint8)


def gen_parallax_map(H, cls):
    """_b: BRANCO = alto (depth = 1-r no shader). flat => 255 (sem offset)."""
    if cls["flat_b"]:
        return np.full((SIZE, SIZE, 4), 255, np.uint8)
    lo, hi = 0.45, 1.0
    h = np.clip((H - lo) / (hi - lo), 0.05, 1.0)
    out = np.dstack([h, h, h, np.ones_like(h)])
    return (out * 255).astype(np.uint8)


def load_albedo(path):
    im = Image.open(path).convert("RGBA")
    if im.size != (SIZE, SIZE):
        im = im.resize((SIZE, SIZE), Image.NEAREST)
    return np.array(im, dtype=np.float32) / 255.0


# --------------------------------------------------------------------------
# Main
# --------------------------------------------------------------------------

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--verify", action="store_true", help="só reporta, não escreve")
    ap.add_argument("--force", metavar="NAME", help="regenera mapas de uma textura")
    args = ap.parse_args()

    names = [n for n in parse_texture_names() if n]
    stats = {"_n": 0, "_s": 0, "_b": 0}
    missing_base = []
    skipped = []

    for name in names:
        base_path = os.path.join(BLOCKS_DIR, name + ".png")
        if not os.path.isfile(base_path):
            missing_base.append(name)
            continue

        targets = {
            "_n": os.path.join(BLOCKS_DIR, name + "_n.png"),
            "_s": os.path.join(BLOCKS_DIR, name + "_s.png"),
            "_b": os.path.join(BLOCKS_DIR, name + "_b.png"),
        }
        todo = {k: v for k, v in targets.items() if not os.path.isfile(v)}
        if args.force and args.force in name:
            todo = dict(targets)
        if not todo:
            continue

        cls = classify(name)
        if args.verify:
            for k in todo:
                stats[k] += 1
            continue

        albedo = load_albedo(base_path)
        alpha = albedo[:, :, 3] if cls["alpha_mask"] else None
        H = gen_height(name, cls)

        os.makedirs(os.path.dirname(targets["_n"]), exist_ok=True)
        for suffix, data in (
            ("_n", gen_normal_map(H, cls, alpha)),
            ("_s", gen_material_map(cls)),
            ("_b", gen_parallax_map(H, cls)),
        ):
            if suffix in todo:
                Image.fromarray(data, "RGBA").save(targets[suffix])
                stats[suffix] += 1

    total = sum(stats.values())
    mode = "FALTANDO (verify)" if args.verify else "GERADOS"
    print("=" * 60)
    print(f"PBR Texture Agent — mapas {mode}")
    print("=" * 60)
    print(f"  texturas registradas : {len(names)}")
    print(f"  _n  {mode.lower():14}: {stats['_n']}")
    print(f"  _s  {mode.lower():14}: {stats['_s']}")
    print(f"  _b  {mode.lower():14}: {stats['_b']}")
    if missing_base:
        print(f"  SEM ALBEDO ({len(missing_base)}): {missing_base[:8]}")
    if total == 0:
        print("  OK: cobertura PBR completa!")
    return 0


if __name__ == "__main__":
    sys.exit(main())



