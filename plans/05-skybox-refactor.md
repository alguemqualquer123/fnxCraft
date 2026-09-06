# [🟡 Médio] SkyBox Refactor

## Objetivo
Refatorar skybox: sun, fog, underwater, day/night, reflexões.

## Checklist
- [ ] Sun rendering fix (`sun.vert/frag` 410 fallback)
- [x] Fog — FEITO (`shader.cpp` 430→410 + `glfwMain` 4.1/4.6)
- [x] Underwater — FEITO (`WATER_DAMPING` + `physics`)
- [ ] Day/night cycle correto
- [ ] Skybox reflections (cubemap `overworld_cubemap`, bindless fallback)
- [ ] `atmosphericScattering` + `hdrToCubeMap` + `preFilterSpecular` integrado

## Arquivos
- `src/gameLayer/rendering/skyBoxRenderer.cpp`
- `resources/shaders/skyBox/*` (7 shaders)
- `src/gameLayer/rendering/shader.cpp` — `preprocessShaderSource`
- `include/gameLayer/rendering/skyBoxRenderer.h`

## Como Testar
Ver sun/fog/underwater/day-night sem artefatos, reflexões no cubemap.

## Prioridade
🟡 Médio — 2/5 feito, 3 pendente (`hardertodos.md`)
