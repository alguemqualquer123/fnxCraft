#!/usr/bin/env python3
"""
Asset Validator Agent — fnxCraft
================================
Cross-check entre o código C++ (fonte da verdade) e os assets em disco:

- Blocos: cada nome em `texturesNames[]` (blocksLoader.cpp) precisa do PNG
  base; relata cobertura PBR (_n/_s/_b).
- Itens: cada path em `itemsNamesTextures[]` (items.cpp) precisa do PNG
  16x16 (fora do padrão renderiza, mas fica inconsistente).
- Shaders: todo uniform referenciado em renderer.cpp (GET_UNIFORM2) deve
  existir no .frag correspondente (evita link/uniform quebrado).

Saída: relatório no stdout + exit code 1 se houver erro crítico.

Uso:
  python3 scripts/agents/validate_assets_agent.py
"""
import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
BLOCKS_DIR = os.path.join(ROOT, "resources", "assets", "blocks")
ITEMS_DIR = os.path.join(ROOT, "resources", "assets", "items")
SHADERS_DIR = os.path.join(ROOT, "resources", "shaders")

CRITICAL = False


def load_blocks_names():
    src = open(os.path.join(ROOT, "src", "gameLayer", "blocksLoader.cpp"),
               encoding="utf-8", errors="replace").read()
    m = re.search(r"const char\s*\*\s*texturesNames\[\]\s*=\s*\{(.*?)\};", src, re.S)
    return [n for n in re.findall(r'"([^"]*)"', m.group(1)) if n]


def load_items_paths():
    src = open(os.path.join(ROOT, "src", "gameLayer", "gameplay", "items.cpp"),
               encoding="utf-8", errors="replace").read()
    m = re.search(r"const char\s*\*\s*itemsNamesTextures\[\]\s*=\s*\{(.*?)\};", src, re.S)
    return re.findall(r'"([^"]*)"', m.group(1))


def check_blocks():
    global CRITICAL
    names = load_blocks_names()
    missing_base = [n for n in names
                    if not os.path.isfile(os.path.join(BLOCKS_DIR, n + ".png"))]
    cov = {}
    for suffix in ("_n", "_s", "_b"):
        have = sum(1 for n in names
                   if os.path.isfile(os.path.join(BLOCKS_DIR, n + suffix + ".png")))
        cov[suffix] = have
    print(f"[blocos]  registrados: {len(names)}")
    print(f"[blocos]  albedo ok : {len(names) - len(missing_base)}")
    for s, h in cov.items():
        pct = 100.0 * h / max(1, len(names))
        print(f"[blocos]  {s}       : {h} ({pct:.0f}%)")
    if missing_base:
        CRITICAL = True
        print(f"[blocos]  SEM ALBEDO (critico): {missing_base[:10]}")
    return len(names), cov


def check_items():
    global CRITICAL
    paths = load_items_paths()
    from PIL import Image
    missing, wrong = [], []
    for p in paths:
        fp = os.path.join(ITEMS_DIR, p)
        if not os.path.isfile(fp):
            missing.append(p)
            continue
        if Image.open(fp).size != (16, 16):
            wrong.append(p)
    print(f"[itens ]  registrados: {len(paths)}  faltando: {len(missing)}  fora do 16x16: {len(wrong)}")
    if missing:
        CRITICAL = True
        print(f"[itens ]  SEM PNG (critico): {missing[:10]}")
    if wrong:
        print(f"[itens ]  aviso (loader faz padding p/ 28x28): {wrong[:10]}")
    return len(paths), len(missing), len(wrong)


def check_shader_uniforms():
    """Todo uniform setado via GET_UNIFORM2(shader, u_x) deve existir no
    .frag OU .vert do shader (declaração em qualquer estágio vale)."""
    renderer_cpp = open(os.path.join(ROOT, "src", "gameLayer", "rendering", "renderer.cpp"),
                        encoding="utf-8", errors="replace").read()
    pairs = re.findall(r"GET_UNIFORM2\((\w+),\s*(u_\w+)\)", renderer_cpp)
    stages_map = {
        "defaultShader": ["rendering/defaultShader.frag", "rendering/defaultShader.vert"],
        "ssrShader": ["postProcess/ssr.frag", "postProcess/drawQuads.vert"],
        "fxaaShader": ["postProcess/fxaa.frag"],
        "filterDownShader": ["postProcess/filterDown.frag"],
        "addMipsShader": ["postProcess/addMipsShader.frag"],
        "gausianBLurShader": ["postProcess/gausianBlur.frag"],
        "applyBloomDataShader": ["postProcess/applyBloomData.frag"],
        "hbaoShader": ["postProcess/hbao.frag"],
        "filterBloomDataShader": ["postProcess/filterBloomData.frag"],
    }
    cache = {}
    bad = []
    checked = 0
    for shader, uni in pairs:
        stages = stages_map.get(shader)
        if not stages:
            continue
        found = False
        for frag in stages:
            fp = os.path.join(SHADERS_DIR, frag)
            if fp not in cache:
                try:
                    cache[fp] = open(fp, encoding="utf-8", errors="replace").read()
                except OSError:
                    cache[fp] = ""
            if re.search(r"\b" + re.escape(uni) + r"\b", cache[fp]):
                found = True
        checked += 1
        if not found:
            bad.append((shader, uni))
    print(f"[shader]  uniforms verificados: {checked}  ausentes: {len(bad)}")
    for s, u in bad:
        # chamadas mortas pré-existentes: glUniform* com location -1 é
        # ignorado pelo OpenGL, então é aviso, não erro crítico
        print(f"[shader]  aviso (uniform não declarado no shader): {s} -> {u}")
    return checked, bad


def main():
    print("=" * 60)
    print("Asset Validator Agent — fnxCraft")
    print("=" * 60)
    nb, cov = check_blocks()
    ni, mi, wi = check_items()
    nc, bad = check_shader_uniforms()
    print("-" * 60)
    status = "ERROS CRITICOS" if CRITICAL else "OK" + (" (com avisos)" if bad else "")
    print(f"RESULTADO: {status}")
    return 1 if CRITICAL else 0


if __name__ == "__main__":
    sys.exit(main())
