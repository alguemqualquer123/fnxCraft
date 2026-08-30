# [PENDING] Survival Mode: Hunger, Thirst & Farming

## Overview
Implement a full survival mode with hunger, thirst, farming, food items, and all related mechanics.

## Related Tasks
- Hunger bar that depletes over time
- Thirst bar that depletes faster than hunger  
- Starvation damage when hunger is empty
- Dehydration damage when thirst is empty
- Food items with different saturation values
- Drink items (water bottles, juices, potions)
- Cooking system (furnace, cooking pot)
- Farming: crop blocks, soil hydration, growth stages
- Animal breeding (feed animals to breed)
- Fishing with fishing rod
- Composting (turn organic matter into fertilizer)

## Status
- [ ] Hunger/thirst bars on HUD — pendente (HUD `heartsTexture` existe, falta barra)
- [ ] Hunger/thirst depletion over time — pendente
- [ ] Starvation/dehydration damage — pendente
- [x] Food items (bread, stew, cooked meats, etc.) — **FEITO 30 Aug 2026**: `ItemTypes` 9 comidas (`rawMeat/cookedMeat/bread/stew/bakedPotato/roastedCorn/cheese/cookedChicken/chickenSoup`) + `loc_ItemName` PT-BR
- [x] Drink items (water bottle, juice, milk) — **FEITO**: 5 bebidas (`waterBottle/juice/milk/coffee/tea`) + fallback checker 16x16
- [ ] Cooking recipes (furnace) — pendente (furnace/cookingPot já existem como blocos)
- [x] Farming blocks (soil, crops at growth stages) — **FEITO**: `BlockTypes` 4 crops (`wheatCrop/potatoCrop/cornCrop/carrotCrop` BlocksCount 220, `blockReorder` + `blocksLookupTable` 147-150)
- [ ] Crop growth mechanics (water nearby, light, bone meal) — pendente (crescimento tick ainda não)
- [ ] Animal breeding (feed to breed) — pendente
- [x] Fishing mechanics — **FEITO**: `fishSpawnEgg/cookedFish/rawFish/fishingRod` + `Fish` entity com `applyWaterPhysics`
- [x] Composting bin — **FEITO** parcial: `ItemTypes` `boneMeal/fertilizer/wateringCan/compost` + `seeds/wheatSeeds/...` (falta bloco compostor)

> **Atualizado 30 Aug 2026:** Itens e blocos de farming/comida/bebida/pesca criados (27 PNGs fallback). HUD e mecânicas de depleção/growth seguem pendentes.

## Priority
High - Core survival gameplay

## Implementation Details
1. Extend Effects system with Hunger and Thirst effects
2. Add hunger/thirst values to PlayerServer
3. Deplete hunger/thirst every tick in survival mode
4. Add food items that restore hunger when eaten
5. Add drink items that restore thirst when drunk
6. Create farming crop blocks with growth stages
7. Add farming growth logic (needs water, light, time)
8. Add cooking recipes for raw food items
9. Add fishing rod interaction with water
