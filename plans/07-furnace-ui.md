# [🟡 Médio] Fornalha UI 3 Slots

## Objetivo
Fornalha com UI real: input/fuel/output + barra progresso + chama.

## Checklist
- [ ] `FurnaceBlock` com `BlocksWithDataHolder` (input, fuel, output, progress)
- [ ] UI 3 slots em `UiEngine`
- [ ] Combustíveis: `wooden_plank` + `coal` (8s cada)
- [ ] Receitas: ore→ingot, rawFood→cooked
- [ ] Barra progresso + anim chama
- [ ] Filtra UI se sem fuel (já parcial)

## Arquivos
- `shared/blocks.h` — `furnace` + `FurnaceBlock`
- `src/gameLayer/gameplay/crafting.cpp` — receitas `requiresFurnace`
- `include/gameLayer/rendering/UiEngine.h` — furnace UI
- `resources/assets/blocks/furnace.png`

## Como Testar
Colocar `ironOre` + `coal` na fornalha, ver virar `ironIngot` em 8s.

## Prioridade
🟡 Médio — fuel `wooden_plank` já, falta UI e `coal`
