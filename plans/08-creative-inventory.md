# [🟡 Médio] Inventário Criativo

## Objetivo
Aba criativa com todos blocos/itens, busca, pegar infinito.

## Checklist
- [ ] Nova aba `INVENTORY_TAB_CREATIVE` em `UiEngine` (ícone estrela)
- [ ] Grid paginado 9×6 com todos `BlockTypes`/`ItemTypes`
- [ ] `InputText` busca filtra `loc_ItemName`
- [ ] `isCreative` → `tryPickupItem` infinito, `placeBlock` não consome
- [ ] `breakBlock` instantâneo (já)
- [ ] `Q` dropa stack 64

## Arquivos
- `include/gameLayer/rendering/UiEngine.h` — nova aba
- `src/gameLayer/gameplay/player.h` — `gameMode==CREATIVE` flag
- `src/gameLayer/gameLayer.cpp` — `placeBlockByClient` criativo
- `resources/assets/ui/creative.png` — ícone

## Como Testar
Abrir `E` em criativo, buscar "stone", pegar 64 e colocar sem consumir.

## Prioridade
🟡 Médio — `next-cycle` #3
