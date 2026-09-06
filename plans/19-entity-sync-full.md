# [🟡 Médio] Sync Completo de Entidades

## Objetivo
Servidor envia **todas** as entidades (não só players) para clientes com chunks carregados.

## Checklist
- [x] Loop `sendAllEntitiesTimer` 0.5s em `tick.cpp` após `sendEntityTimer`
- [x] Para cada `client` + cada `chunk` em `loadedChunks`, envia todas as 43 listas `entityData.*`
- [x] Usa `headerUpdateGenericEntity` + `Packet_UpdateGenericEntity{eid,timer}` + `entity` bytes
- [x] Cobre: zombies/pigs/cats/goblins/fish/droppedItems/trainingDummy/scareCrows/bees/queenBees/slime/creepers/enderlings/skeletons/stoneGolems/wolves/foxes/crows/manatees/caveSpiders/crystalBats/capybaraChefs/riverGuardians/treeEnts/nomadTraders/mimicChests/lightFairies/armoredBoars/sandSerpents/mistGhosts/hermitCrabs/honeyBears/lavaSlugs/crystalSentinels/blacksmithVillagers/herbalistVillagers/skeletonPirates/juvenileDragons/crystalGolems/hydras/sheeps/cows
- [ ] Rate-limit / delta-compress (só envia se mudou)
- [ ] Cliente culling: só renderiza se chunk visível (ligar com `06-entity-visibility`)

## Arquivos
- `src/gameLayer/multyPlayer/tick.cpp` — `#pragma region server send entity position data`
- `include/gameLayer/multyPlayer/packet.h` — `headerUpdateGenericEntity`
- `include/gameLayer/gameplay/allentities.h` — `EntitiesTypesCount=43`

## Como Testar
Conectar 2 clientes, spawnar pig/zombie via `/flatmobs`, ver outro cliente receber.

## Prioridade
🟡 Médio — feito, falta otimizar
