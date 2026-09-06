# [🟡 Médio] Renderização — Otimização + Fixes

## Objetivo
Otimizar e corrigir renderização existente.

## Checklist
- [ ] `transparentGeometryCounter` cache p/ skip bake vazio (`hardertodos.md`)
- [ ] Depth peeling água completo (`u_depthPeelwaterPass`, `u_PeelTexture`)
- [ ] Z prepass + `glBufferStorage` (`//todo optimize with buffer storage`)
- [ ] `bigGpuBuffer` modernizar (`//todo look into a modern function`)
- [ ] Texture binding once (`//todo optimize texture binding`)
- [ ] Chunk sorting `//todo only copy chunks that are close`
- [ ] CSM/shadows optimize (`//TODO OPTIMIZE!`, `README: Shadows todo optimize`)
- [ ] Fog/underwater/god rays/fake shadows improve (`README todo improve`)
- [ ] Gyzmos line far fix (`//todo not working at far distances`)
- [ ] Player/zombie rendering correto (`hardertodos.md`)
- [ ] Line drawing colisões/blocos (`hardertodos.md`)
- [ ] Model freeing + camera rotate (`model.cpp`/`camera.cpp` todos)

## Arquivos
- `src/gameLayer/rendering/chunk.cpp` — `transparentGeometry`
- `src/gameLayer/rendering/renderer.cpp` — 6833 linhas, passes, `fboMain`/`HBAO`/`SunForGodRays`
- `src/gameLayer/rendering/bigGpuBuffer.cpp`
- `include/gameLayer/rendering/renderer.h` — `GyzmosRenderer`, `PointDebugRenderer`
- `src/gameLayer/rendering/skyBoxRenderer.cpp` — `atmosphericScattering`

## Como Testar
Perf `profiler` antes/depois, sem bake chunk vazio, sem glitch far lines.

## Prioridade
🟡 Médio — perf + polish
