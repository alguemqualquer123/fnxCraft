# Changelog - fnxCraft

## [Unreleased] - 2026-09-06
### Adicionado
- **PBR completo para todos os blocos**: `scripts/agents/pbr_texture_agent.py` gerou **855 mapas** (`_n` normal, `_s` material, `_b` altura) para os 307 slots de textura — antes só 44 blocos tinham normal map. Convenção validada empiricamente contra `dirt_n` (R=0.5−dH/dx, G=0.5−dH/dy, flat=127/127/255). Classificação de material por nome (madeira com grão, tijolos com argamassa, minérios com protuberâncias, cristais facetados voronoi, tecidos, metais com costuras, terra granulada, folhas celulares, plantas planas com alpha)
- **Parallax Occlusion Mapping ativado**: código que existia comentado no `defaultShader.frag` foi reimplementado e ligado — steep parallax + oclusão interpolada usando a textura `_b` (antes carregada e nunca usada), TBN construído da normal da face, guard `viewVector.z > 0.05`, UV com `fract()` para manter tiling. Controlado por `ShadingSettings.parallaxStrength` (default 0.03) com slider no menu **Settings > Parallax** (0 = off). Fallback `_b` do loader agora é BRANCO (255) para texturas sem heightmap não sofrerem offset
- **Agentes de assets** (`scripts/agents/`): `pbr_texture_agent.py` (blocos PBR, `--verify`/`--force`), `item_texture_agent.py` (itens 16×16 pixel-art com contorno, `--verify`/`--outdir`), `validate_assets_agent.py` (cross-check C++↔assets↔shaders: albedo obrigatório, cobertura PBR, PNGs de itens, uniformes GET_UNIFORM2 presentes no .frag) e `run_all_agents.sh` (orquestrador com `--build`/`--verify`)
- **Validator**: 205/205 itens com PNG 16×16 confirmado (README desatualizado falava em 23 faltando; 9 itens de arma fora do padrão 16×16 renderizam ok pois o loader faz padding para 28×28)
### Corrigido
- **Build quebrado no HEAD** (`d708418c`): 13 `undefined reference` — `audioEngine.h` declarava `isAnyStone(unsigned int)` etc., mas as definições em `shared/blocks.cpp` usam `BlockType` = `uint16_t` (mangling diferente). Declarações alinhadas para `uint16_t` (+ `#include <stdint.h>`)
- `unbreakable` (símbolo inexistente) em `audioEngineRaudioBackend.cpp:553` — check redundante removido (`isAnyUnbreakable` já cobre)
- `Launcher.h` quebrado por edição paralela (`dpd` não declarado, `ProgramData` sem membro `launcher`) — restaurado do HEAD

## [Unreleased] - 2026-08-30
### Adicionado
- **F3 Debug** (`F3`): overlay com FPS, XYZ, Block, Chunk, Facing, Biome veg, Light, Entities, modo câmera
- **Câmera F5**: 3 modos (1ª pessoa, 3ª costas, 3ª frente) com colisão raycast, distância 2-10 via Scroll, mão só em 1ª pessoa e player renderizado em 3ª
- **Água Minecraft**: espalhamento até 8 blocos com nível 0-7 (altura decrescente), queda, evaporação se isolada, **infinita 2x2** (≥2 sources adjacentes/diagonal vira source)
- **Farming**: blocos `wheat/potato/corn/carrotCrop` (cross, stage 0-7), plantio `seeds` em `dirt` com água raio 4, `boneMeal`+1, `fertilizer`+3, `compost`+2, `wateringCan` 50%, crescimento servidor a cada 2s, drop stage7 `wheat`+2 seeds etc
- **Fornalha**: receitas `requiresFurnace` agora consomem `wooden_plank` como combustível (filtra UI se sem fuel)
- **Arco**: 7 bows `wooden/copper/lead/iron/silver/gold/goblinBow` (`bows/*.png`), hold `RMB` carrega 0-1.1s, barra central, força 9-26, consome flecha de `ARROWS_START_INDEX`, spawna `droppedItem` com física
- **Skins**: `loadPlayerSkin` agora aceita `64x64`/`64x32` (upscale 2x para 128), coleta de `RESOURCES_PATH/skins` + `./resources/skins` + `resources/skins`, ordena e mostra `Default (1/9)` etc, fallback triplo
- **F3+G Chunk Borders**: `F3+G` toggle grid 5x5 chunks ao redor do player via `GyzmosRenderer.drawLine` (vermelho central, amarelo demais)
- **Partículas**: 6 cubos 0.14 ao quebrar bloco, gravidade, vida 0.6s, cor por tipo, render via `GyzmosRenderer`
- **Tocha Piscando**: `torch/torchWood/goblinTorch/lamp` variam `lightLevel 11-15` com `sin(flick+hash)` a 7Hz
- **Sons/Mão**: `waterSwim` loop 0.65s quando nadando, mão balanço mais pronunciado (`Oscilator 0.22`, `132°/6°/8°`)
- **Minimapa**: segue player (`centerPos=chunkPos`), zoom `Scroll` e `Ctrl+Scroll` (0.6-10), marcador vermelho 4x4 + cruz
- **IA Goblin**: `hearBonus +0.3` se vida <40, foge se <18 (direção oposta ao player <14, `keepJumpingTimer 0.35`)
- **Scripts**: pasta `scripts/` com 8 `.sh` (`build.sh`, `build-all.sh`, `configure.sh`, `run.sh`, `run-console.sh`, `run-server.sh`, `clean.sh`, `dev.sh`)
- **Skills**: `.opencode/skills/create-item` e `f3-debug` + `opencode.json` com `skills.paths`
- **Fixes**: `BackupManager` includes, `raudio.h` stdbool, `isColidable` via `isGrassMesh`, `isAnyPlant` inclui crops, `seed` parsing suporta string hash (`hash*31+c`), `wheat` conflito removido, `postProcess`/`skyBox` case
