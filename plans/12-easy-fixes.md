# [🟢 Fácil] Fixes Fáceis (polish)

## Objetivo
Pequenos fixes de polish, 1 dia cada.

## Checklist
- [ ] Câmera não mover ao sair do inventário (`hardertodos.md`)
- [ ] `sendPlayerOtherInfo` function (`hardertodos.md`)
- [ ] Item dropping survival — rejeitar drop recria inventário (`hardertodos.md`)
- [ ] Player push por outros (`hardertodos.md`)
- [ ] `transparentGeometryCounter` cache p/ não bakear chunk vazio (`hardertodos.md`)
- [ ] Cleanup no exit — server salvar ao sair (`hardertodos.md`)
- [ ] Salvar `currentEntityId` + enviar todas entidades ao join (`hardertodos.md`)
- [ ] `shrink UVs` leve p/ modelos (`todo.txt`)
- [ ] `load only dimensions` (`todo.txt`)
- [ ] Mover `world/`+`settings/` fora de `resources/` (`todo.txt`)
- [ ] F3 polish: `chunkSection`/`light`/`simulationDistance`/`seed` + cor borda por Y
- [ ] Farming polish: `blockNames` cor por stage, `boneMeal` partícula verde
- [ ] Tocha flicker em `torch` também (hoje só `lamp`)
- [ ] Skins grid 4×4 preview 32×32

## Arquivos
- `src/gameLayer/gameLayer.cpp` — câmera/inventário
- `shared/blocks.h` — `Block::normalize` UVs
- `src/gameLayer/GamePaths.cpp` — mover `world/settings`
- `src/gameLayer/gameplay/player.cpp` — push/drop

## Como Testar
Cada fix isolado: abrir inventário e fechar sem mover câmera etc.

## Prioridade
🟢 Fácil — `hardertodos.md` + `todo.txt`
