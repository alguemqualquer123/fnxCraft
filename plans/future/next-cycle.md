# Next Cycle - ourCraft (2026-08-30)

> Sempre criar plano antes de implementar. Ordem sugerida após ciclo anterior (F3, água, farming, fornalha, partículas etc).

## 1. Redstone Básico (prioridade alta)
**Objetivo:** pó, tocha, repetidor, pistão simples, lamp acende.

- **Blocos novos:** `redstoneDust`, `redstoneTorch`, `redstoneRepeater`, `piston`, `redstoneLamp` (usar `lamp` já existe como base)
- **Dados:** `BlocksWithDataHolder` → `RedstoneBlock { bool powered; int powerLevel 0-15; }` similar a `ChestBlock`
- **Simulação:** `redstoneSimulation.cpp` (BFS) a cada tick: propaga `powerLevel-1` para 4 vizinhos se bloco é `redstoneDust` ou `lamp`, tocha inverte, repeater delay 2 ticks
- **Render:** `blocksLoader` texturas `redstone_dust`, `redstone_torch_on/off`, `repeater`, `lamp_on/off`; `isTransparentGeometry` para pó
- **Interação:** `E` coloca pó, `RMB` em tocha desliga, `Q` dropa pó
- **Teste:** linha 8 blocos com tocha acende lamp no fim; pistão empurra 1 bloco

## 2. Montarias (cavalo/pig) (prioridade alta)
**Objetivo:** domar, montar, mover, pular.

- **Entidades:** `HorseServer/Client` (reusar `Pig` como base, novo modelo `horse.bbmodel` ou recolor pig), `Pig` já existe como montaria? Habilitar `Pig` montável via `saddle` item novo
- **Item:** `saddle`, `horseArmor` (usar `leatherChestPlate` como placeholder)
- **Mecânica:** `E` com `saddle` em `Pig/Horse` entra em modo montado: `player.entity.position = horse.position + offset`, `WASD` controla `horse.moveFPS` não player, `Space` pula, `Shift` desmonta, `horse.update` com `doCollisionWithOthers`
- **Dados:** `HorseServer` com `riderId`, `speed`, `jumpStrength`
- **Render:** `entityManager.players` + `horse` render, esconder pernas player quando montado
- **Teste:** domar pig com `carrot`, por sela, cavalgar por rio

## 3. Inventário Criativo (prioridade alta)
**Objetivo:** aba com todos os blocos/itens, busca, pegar infinito.

- **UI:** nova aba `INVENTORY_TAB_CREATIVE` em `UiEngine.cpp` (ícone estrela), grid paginado 9x6, `InputText` busca filtra `loc_ItemName`
- **Dados:** `PlayerInventory::MAX_EQUIPEMENT_SLOTS` não usado, usar `creative` flag `gameMode==CREATIVE` já existe
- **Lógica:** `isCreative` → `tryPickupItem` infinito, `placeBlockByClient` não consome, `breakBlockByClient` instantâneo (já), `Q` dropa stack 64
- **Render:** `UiEngine::renderGameUI` adiciona aba, `icons/creative.png`
- **Teste:** abrir `E` em criativo, buscar "stone", pegar 64 e colocar

## 4. Polir o que já está feito (média)
**Checklist:**
- **Água:** clamp diagonal infinite, evaporar se sem suporte, performance `activeWaterChunks`
- **Farming:** mostrar stage via `blockNames` com cor, `F3` mostra `light` e `hasWater`, `boneMeal` partícula verde
- **Fornalha:** UI 3 slots (input/fuel/output) com `FurnaceBlock` real e progresso barra, fuel `wooden_plank`/`coal` 8s, anim chama
- **F3/G:** mostrar `biome` nome (não veg float), `simulationDistance`, `seed` em F3, chunk border cores por Y
- **Partículas:** pooling e `alpha` fade, som `break` já, adicionar `place` som
- **Tocha:** `lamp` emissor já piscando, fazer `torch` também via `LightSystem` não só `lamp`
- **Skins:** paginação grid 4x4 com preview 32x32, `loadPlayerSkin` já suporta 64
- **Scripts:** `dev.sh` com `-DCMAKE_POLICY_VERSION_MINIMUM=3.5`, `README` já feito
- **Geral:** `clang-tidy`, `sanitize` `Block::normalize`, `CHANGELOG` já feito

## Ordem de execução
1. Redstone básico (1 semana)
2. Montarias (1 semana)
3. Criativo (3 dias)
4. Polir (contínuo, cada PR inclui testes)

Cada item vira `skill` em `.opencode/skills/<nome>/SKILL.md` e `CHANGELOG`.
