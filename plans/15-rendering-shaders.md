# [🟡 Médio] Shaders — Bloom/SSR/Água/DoF

## Objetivo
Reativar e polir shaders faltantes/melhorar existentes (47 shaders).

## Checklist
- [ ] Reativar Bloom (`renderSettings.h: bloom disabled - causes black screen, re-enable after skybox fix`)
- [ ] SSR polish — `ssr.frag` `//todo check/test`, `F0` metallic, `pow(roughness)`
- [ ] Água UVs contínuas (`defaultShader.frag: //todo water uvs continuous`)
- [ ] Água sub-scatter + darken deep (`//todo light sub scatter`, `//darken deep stuff`)
- [ ] Decal/crack shader reuse (`//todo also reuse in decal shader`)
- [ ] Depth of Field blur distante (novo)
- [ ] Volumetric fog/lighting (novo, hoje só fog linear)
- [ ] Parallax occlusion mapping p/ PBR
- [ ] Luzes cube maps (`pointLight.h` → sombras cube map)
- [ ] Shader unificado (mesmo shader p/ tudo)

## Arquivos
- `resources/shaders/postProcess/*` (14), `rendering/defaultShader.frag` (todo water)
- `resources/shaders/skyBox/*`, `rendering/decal.frag`
- `include/gameLayer/rendering/renderSettings.h` — `bloom`, `SSR`, `PBR`
- `src/gameLayer/rendering/renderer.cpp` — passes

## Como Testar
Ativar bloom sem black screen, SSR refletindo água, DoF blur distante.

## Prioridade
🟡 Médio — 10 itens, shader work
