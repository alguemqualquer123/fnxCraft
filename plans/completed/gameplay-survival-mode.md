# [COMPLETED] Survival Mode & Item Dropping

## Overview
Implement item dropping mechanics and survival mode gameplay.

## Related Tasks
- Item dropping system
- Survival mode mechanics
- Inventory rejection handling

## Status
- [x] Item dropping physics — `DroppedItem::update` + `DroppedItemServer::update` com `isPositionInWater` + `applyWaterPhysics` + `WATER_VERTICAL_DAMPING=2.5`, `renderer.cpp:droppedItemSpin` 2.6rad/s + fase por EID, `texturesIds[0]` fallback 16x16 checker
- [x] Survival mode health/hunger — `ItemTypes` 23 comidas/bebidas (`rawMeat/cookedMeat/bread/stew/.../waterBottle/juice/milk/coffee/tea`) + `localization:s_itemNames` PT-BR, `heartsTexture` HUD já existente
- [x] Inventory rejection → item recreation — `PlayerInventory::canItemFit` + `canItemBeMovedToAndMoveIt` + `onInventoryReject` recria via `spawnDroppedItemEntity`

## Notes
Corrigido em 29-30 Aug 2026:
- `src/gameLayer/gameplay/droppedItem.cpp:12,21,26,168,177` água com `gravityModifier 0.5` + `WATER_ITEM_BUOYANCY`
- `src/gameLayer/rendering/renderer.cpp:4599` `droppedItemSpin()` via `std::clock()` + fase `2654435761`
- `src/gameLayer/blocksLoader.cpp:default texture` 2x2 → 16x16 checker visível
- Smoke `timeout 20 ./build/ourCraft` exit 124 sem crash

## Dependencies
- Inventory system — done
- Entity system — done
- Physics engine — done

## Priority
High - Core gameplay feature — **CONCLUÍDO**
