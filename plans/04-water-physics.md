# [🟡 Médio] Física da Água Completa

## Objetivo
Implementar flow Minecraft completo + visuais + pressão.

## Checklist
- [ ] Flow direcional (procura menor altura, espalha até 8 blocos)
- [ ] Source criação/destruição correta
- [ ] Interação terreno (preenche buracos, contorna blocos)
- [ ] Clamp diagonal infinita + evapora sem suporte
- [ ] `activeWaterChunks` performance
- [ ] Ondas superfície + underwater distortion
- [ ] Caustics/refração melhor
- [ ] Splash som integração (`audio-water-sounds`)
- [x] Buoyancy player/itens — FEITO
- [x] Drowning — FEITO
- [x] Entity buoyancy — FEITO

## Arquivos
- `src/gameLayer/multyPlayer/waterSimulation.cpp` — flow BFS
- `shared/blocks.h` — `waterLevel` 0-7
- `src/gameLayer/gameplay/physics.cpp` — `applyWaterPhysics`
- `resources/shaders/rendering/defaultShader.frag` — DUDV/caustics

## Como Testar
Colocar água no alto, ver escorrer até 8 blocos e preencher buraco.

## Prioridade
🟡 Médio — 3/10 feito, 7 pendente
