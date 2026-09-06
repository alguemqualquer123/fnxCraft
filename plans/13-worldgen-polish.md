# [🟡 Médio] WorldGen Polish + Sprites

## Objetivo
Melhorar geração de mundo e sprites.

## Checklist
- [ ] Rios entram no oceano (`todo.txt`)
- [ ] Rios na altura das planícies (níveis diferentes)
- [ ] Continentalness/montanhas mais vastos + bias biomas maiores
- [ ] Dried rivers vs roads — road igual dried river (`todo.txt`)
- [ ] Mover sprites restantes in-game (`todo.txt`)
- [ ] View distance sem resetar chunks (`todo.txt`)
- [ ] `structures` spawn testado (39 `.structure` existem)

## Arquivos
- `src/gameLayer/worldGenerator.cpp` — 2416 linhas
- `shared/worldGeneratorSettings.h`
- `shared/voronoi.*`, `shared/splines.*`
- `resources/gameData/structures/` — 39

## Como Testar
Gerar mundo novo, verificar rios entram no oceano e road parece dried river.

## Prioridade
🟡 Médio — `todo.txt`
