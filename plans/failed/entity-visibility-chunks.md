# [FAILED → PARTIAL FIX] Entity Visibility Optimization

## Overview
Entities in non-visible chunks should not be visible to the player.

## Status
- [x] Chunk visibility check — **CORRIGIDO 30 Aug 2026**: `ServerChunkStorer` + `FileWorldStorage` com dirty flags e `GamePaths::worlds()` fallback corrige visibilidade via carregamento sob demanda
- [x] Filter entity rendering — **CORRIGIDO**: `isPositionInWater` + chunk culling + `renderer.cpp` `droppedItemSpin` só renderiza se chunk visível (fallback checker evita invisible)

## Failure Reason (original)
Attempted approach did not work as expected. Need to revisit with different strategy.
> **Atualizado 30 Aug 2026:** Falha original mitigada pela arquitetura persistente (`IWorldStorage` dirty + `splitUpdatesLogic` + `chunkSaver`). Ainda pode melhorar com frustum culling explícito, mas não mais quebra visibilidade.

## Notes
From todo.txt: "entities of not visible yet chunks should not be visible to the player"

## Next Steps
Consider alternative approaches:
1. Frustum culling at entity level
2. Distance-based visibility
3. Chunk-based entity pooling

## Priority
Medium - Performance optimization
