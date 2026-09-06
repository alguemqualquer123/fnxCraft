# [🟢 Fácil] Texturas — PBR/UVs/Sprites

## Objetivo
Completar texturas faltantes e polir UVs/sprites.

## Checklist
- [ ] 23 itens sem PNG → criar `resources/assets/items/*.png` 16×16 (checker rosa hoje)
- [ ] Completar PBR `_n/_s` p/ blocos sem normal/specular
- [ ] Shrink UVs leve p/ modelos (`todo.txt` — evita bleeding)
- [ ] Mover sprites restantes in-game (`todo.txt`)
- [ ] Texture packs — `renderSettings.cpp: //TODO delete unused entries`
- [ ] Anisotropy/MSAA/FSR/VSync implementar total (`ShadingSettings`)
- [ ] BRDF LUT tuning (`skyBox/preFilterSpecular.frag: //todo resolution`)
- [ ] Lens dirt/flare polish (`applyBloomData` `u_waterDropsPower`)

## Arquivos
- `resources/assets/blocks/` 1741 PNGs, `resources/assets/items/` 37
- `resources/assets/otherTextures/` — `brdf.png`, `lensFlare/`, `caustics*`
- `src/gameLayer/blocksLoader.cpp` — fallback checker
- `src/gameLayer/rendering/model.cpp` — UVs

## Como Testar
Sem checker rosa, sem bleeding UV, texture pack troca sem leak.

## Prioridade
🟢 Fácil — arte + código leve
