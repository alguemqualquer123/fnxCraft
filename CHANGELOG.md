# Changelog - ourCraft

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
