# [🔴 Avançado] Redstone Básico

## Objetivo
Pó, tocha, repeater, pistão, lamp acende.

## Checklist
- [ ] Blocos: `redstoneDust`, `redstoneTorch`, `redstoneRepeater`, `piston`, `redstoneLamp` (hoje 3/5)
- [ ] `RedstoneBlock { bool powered; int powerLevel 0-15; }` similar `ChestBlock`
- [ ] `redstoneSimulation.cpp` BFS: `powerLevel-1` p/ 4 vizinhos, tocha inverte, repeater delay 2 ticks
- [ ] Render: `redstone_dust`, `torch_on/off`, `repeater`, `lamp_on/off`, `isTransparentGeometry` p/ pó
- [ ] Interação: `E` coloca pó, `RMB` tocha desliga, `Q` dropa pó
- [ ] Teste: linha 8 blocos com tocha acende lamp; pistão empurra 1 bloco

## Arquivos
- `shared/blocks.h` — `redstone*` + `RedstoneBlock`
- `src/gameLayer/redstoneSimulation.cpp` — novo
- `src/gameLayer/blocksLoader.cpp` — texturas
- `src/gameLayer/blockUpdates.cpp` — tick redstone

## Como Testar
Linha 8 `redstoneDust` com `redstoneTorch` acende `lamp` no fim.

## Prioridade
🔴 Avançado — `next-cycle` #1 (1 semana)
