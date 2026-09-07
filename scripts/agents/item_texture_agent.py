#!/usr/bin/env python3
"""
Item Texture Agent — fnxCraft
=============================
Valida e preenche as texturas de itens registradas em `itemsNamesTextures[]`
(src/gameLayer/gameplay/items.cpp).

- Confere se todo item tem PNG em resources/assets/items/<path> (fallback do
  engine = checker rosa).
- Confere tamanho 16x16 (padrão do projeto; o loader faz padding p/ 28x28,
  então tamanhos diferentes renderizam, mas ficam fora do padrão visual).
- Gera pixel-art 16x16 temática (contorno preto + sombreamento, fundo
  transparente) para qualquer PNG faltante — seguindo
  .opencode/skills/create-item/SKILL.md.

Uso:
  python3 scripts/agents/item_texture_agent.py                # valida + gera faltantes
  python3 scripts/agents/item_texture_agent.py --verify       # só relatório
  python3 scripts/agents/item_texture_agent.py --outdir /tmp  # teste, não toca nos assets
"""
import argparse
import os
import re
import sys

from PIL import Image, ImageDraw

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
ITEMS_DIR = os.path.join(ROOT, "resources", "assets", "items")
ITEMS_CPP = os.path.join(ROOT, "src", "gameLayer", "gameplay", "items.cpp")

BLACK = (25, 20, 20, 255)


def parse_item_paths():
    src = open(ITEMS_CPP, encoding="utf-8", errors="replace").read()
    m = re.search(r"const char\s*\*\s*itemsNamesTextures\[\]\s*=\s*\{(.*?)\};", src, re.S)
    if not m:
        raise SystemExit("ERRO: itemsNamesTextures[] não encontrado em items.cpp")
    return re.findall(r'"([^"]*)"', m.group(1))


# --------------------------------------------------------------------------
# Pixel-art helpers (canvas 16x16, contorno preto, fundo transparente)
# --------------------------------------------------------------------------

def new_canvas():
    im = Image.new("RGBA", (16, 16), (0, 0, 0, 0))
    return im, ImageDraw.Draw(im)


def _px(d, x, y, c):
    d.point((x, y), fill=c)


def _shade(c, f):
    return (max(0, min(255, int(c[0] * f))),
            max(0, min(255, int(c[1] * f))),
            max(0, min(255, int(c[2] * f))), c[3])


def gen_bottle(liquid, cap=(120, 90, 60, 255)):
    """Garrafa/frasco de vidro com líquido colorido."""
    im, d = new_canvas()
    glass = (200, 225, 235, 255)
    glass_dark = _shade(glass, 0.75)
    d.rectangle([5, 7, 10, 13], fill=glass)
    d.rectangle([6, 8, 9, 13], fill=liquid)
    d.line([6, 8, 9, 8], fill=_shade(liquid, 1.25))
    d.rectangle([6, 4, 9, 6], fill=glass_dark)
    d.rectangle([6, 2, 9, 3], fill=cap)
    d.rectangle([5, 2, 10, 13], outline=BLACK)
    _px(d, 6, 9, (255, 255, 255, 180))
    _px(d, 6, 10, (255, 255, 255, 120))
    return im


def gen_bowl(soup, bowl_color=(150, 95, 45, 255)):
    """Tigela com sopa (stew, chickenSoup)."""
    im, d = new_canvas()
    d.polygon([(3, 8), (12, 8), (11, 13), (4, 13)], fill=bowl_color)
    d.rectangle([3, 7, 12, 8], fill=soup)
    d.rectangle([4, 7, 11, 7], fill=_shade(soup, 1.2))
    d.rectangle([3, 7, 12, 13], outline=BLACK)
    d.rectangle([3, 7, 12, 8], outline=BLACK)
    return im


def gen_meat(raw):
    """Bife cru (rosa) ou assado (marrom)."""
    im, d = new_canvas()
    c = (215, 100, 105, 255) if raw else (140, 85, 45, 255)
    dark = _shade(c, 0.7)
    d.ellipse([2, 4, 13, 12], fill=c)
    d.ellipse([4, 6, 9, 10], fill=dark)
    d.ellipse([2, 4, 13, 12], outline=BLACK)
    if not raw:
        _px(d, 6, 5, (240, 220, 190, 255))
        _px(d, 9, 7, (240, 220, 190, 255))
    return im


def gen_bread(color=(190, 130, 60, 255)):
    im, d = new_canvas()
    d.ellipse([2, 5, 13, 11], fill=color)
    for x, y in ((5, 6), (8, 7), (10, 6)):
        _px(d, x, y, _shade(color, 1.3))
    d.ellipse([2, 5, 13, 11], outline=BLACK)
    return im


def gen_cheese():
    im, d = new_canvas()
    c = (235, 190, 60, 255)
    d.polygon([(2, 11), (13, 11), (13, 5), (2, 8)], fill=c)
    for x, y in ((5, 9), (9, 8), (11, 10)):
        d.ellipse([x, y, x + 1, y + 1], fill=_shade(c, 0.75))
    d.rectangle([2, 5, 13, 11], outline=BLACK)
    return im


def gen_seeds(color):
    im, d = new_canvas()
    for x, y in ((5, 5), (9, 4), (7, 8), (4, 10), (10, 10), (8, 12)):
        d.ellipse([x, y, x + 2, y + 2], fill=color)
        _px(d, x + 1, y, _shade(color, 1.3))
    return im


def gen_bag(band, label=None):
    """Saco/pacote (fertilizer, compost, boneMeal)."""
    im, d = new_canvas()
    paper = (196, 160, 105, 255)
    d.rectangle([4, 4, 11, 13], fill=paper)
    d.rectangle([5, 2, 10, 3], fill=_shade(paper, 0.8))   # topo dobrado
    d.rectangle([4, 7, 11, 9], fill=band)                 # faixa colorida
    if label:
        d.rectangle([6, 10, 9, 11], fill=label)
    d.rectangle([4, 2, 11, 13], outline=BLACK)
    return im


def gen_watering_can():
    im, d = new_canvas()
    metal = (150, 160, 170, 255)
    d.rectangle([4, 6, 11, 13], fill=metal)               # corpo
    d.line([11, 8, 14, 5], fill=metal, width=2)           # bico
    d.arc([5, 2, 11, 8], 180, 360, fill=_shade(metal, 0.7), width=2)  # alça
    d.ellipse([3, 4, 6, 7], fill=_shade(metal, 0.85))     # topo
    d.rectangle([4, 6, 11, 13], outline=BLACK)
    _px(d, 14, 4, BLACK)
    return im


# mapa: substring do path -> gerador
GENERATORS = [
    ("waterBottle", lambda: gen_bottle((90, 160, 220, 255))),
    ("juice", lambda: gen_bottle((230, 150, 40, 255))),
    ("milk", lambda: gen_bottle((240, 240, 235, 255), cap=(200, 200, 200, 255))),
    ("coffee", lambda: gen_bottle((90, 55, 30, 255))),
    ("tea", lambda: gen_bottle((160, 200, 60, 255))),
    ("stew", lambda: gen_bowl((190, 90, 40, 255))),
    ("chickenSoup", lambda: gen_bowl((235, 205, 130, 255))),
    ("rawMeat", lambda: gen_meat(raw=True)),
    ("cookedMeat", lambda: gen_meat(raw=False)),
    ("cookedChicken", lambda: gen_bread((215, 160, 80, 255))),
    ("bread", lambda: gen_bread()),
    ("applePie", lambda: gen_bread((225, 175, 90, 255))),
    ("bakedPotato", lambda: gen_bread((200, 150, 70, 255))),
    ("roastedCorn", lambda: gen_bread((220, 190, 70, 255))),
    ("cheese", gen_cheese),
    ("wheatSeeds", lambda: gen_seeds((140, 190, 70, 255))),
    ("potatoSeeds", lambda: gen_seeds((170, 200, 110, 255))),
    ("cornSeeds", lambda: gen_seeds((220, 200, 80, 255))),
    ("carrotSeeds", lambda: gen_seeds((230, 140, 60, 255))),
    ("seeds.png", lambda: gen_seeds((150, 190, 90, 255))),
    ("boneMeal", lambda: gen_bag((220, 220, 215, 255), (240, 240, 235, 255))),
    ("fertilizer", lambda: gen_bag((90, 170, 70, 255), (150, 220, 120, 255))),
    ("compost", lambda: gen_bag((110, 80, 50, 255), (90, 120, 60, 255))),
    ("wateringCan", gen_watering_can),
]


def generate_for(path):
    """Escolhe o gerador pelo nome do arquivo; fallback = saco marrom."""
    name = os.path.basename(path)
    for key, fn in GENERATORS:
        if key.lower() in name.lower():
            return fn()
    return gen_bag((150, 110, 70, 255))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--verify", action="store_true")
    ap.add_argument("--outdir", metavar="DIR", help="gera em outro diretório (teste)")
    args = ap.parse_args()

    out_base = args.outdir or ITEMS_DIR
    paths = parse_item_paths()
    missing, wrong_size = [], []

    for p in paths:
        fp = os.path.join(ITEMS_DIR, p)
        if not os.path.isfile(fp):
            missing.append(p)
            continue
        im = Image.open(fp)
        if im.size != (16, 16):
            wrong_size.append((p, im.size))

    generated = 0
    if not args.verify:
        for p in missing:
            dst = os.path.join(out_base, p)
            os.makedirs(os.path.dirname(dst), exist_ok=True)
            generate_for(p).save(dst)
            generated += 1

    print("=" * 60)
    print("Item Texture Agent")
    print("=" * 60)
    print(f"  itens registrados      : {len(paths)}")
    print(f"  PNGs faltando          : {len(missing)} {missing[:10]}")
    print(f"  tamanho != 16x16 (ok)  : {len(wrong_size)} {[w[0] for w in wrong_size[:8]]}")
    if not args.verify:
        print(f"  gerados agora          : {generated}")
    if not missing and not wrong_size:
        print("  OK: cobertura completa e no padrão!")
    return 0


if __name__ == "__main__":
    sys.exit(main())

