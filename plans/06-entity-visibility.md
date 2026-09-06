# [🟡 Médio] Culling Entidades por Chunk

## Objetivo
Entidades de chunks não visíveis não devem renderizar.

## Checklist
- [ ] Frustum culling em `entity` level
- [ ] Distance-based visibility
- [ ] Chunk-based entity pooling
- [x] `ServerChunkStorer` + dirty flags — FEITO (mitiga parcial)
- [x] `isPositionInWater` + chunk culling — FEITO (parcial)

## Arquivos
- `src/gameLayer/rendering/frustumCulling.cpp`
- `src/gameLayer/gameplay/entityManagerClient.cpp`
- `src/gameLayer/rendering/renderer.cpp` — filtrar `renderEntities`
- `include/gameLayer/multyPlayer/serverChunkStorer.h`

## Como Testar
Spawnar 100 entidades em chunk distante, verificar não renderizam.

## Prioridade
🟡 Médio — `failed/entity-visibility-chunks` parcial, precisa frustum explícito
