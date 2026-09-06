# [🔴 Avançado] Montarias (Cavalo/Pig)

## Objetivo
Domar, montar, mover, pular.

## Checklist
- [ ] Entidade `HorseServer/Client` (reusar `Pig` ou `horse.bbmodel`)
- [ ] Item `saddle`, `horseArmor` (placeholder `leatherChestPlate`)
- [ ] Mecânica: `E` com `saddle` em `Pig/Horse` entra montado
- [ ] `WASD` controla `horse.moveFPS` não player, `Space` pula, `Shift` desmonta
- [ ] `HorseServer { riderId, speed, jumpStrength }` + `doCollisionWithOthers`
- [ ] Render: esconder pernas player quando montado
- [ ] Teste: domar pig com `carrot`, por sela, cavalgar por rio

## Arquivos
- `include/gameLayer/gameplay/horse.h` — novo
- `src/gameLayer/gameplay/horse.cpp` — novo
- `include/gameLayer/gameplay/items.h` — `saddle`
- `src/gameLayer/gameplay/player.cpp` — mount logic

## Como Testar
Domar pig com `carrot`, `E` com sela, `WASD` cavalga, `Space` pula.

## Prioridade
🔴 Avançado — `next-cycle` #2 (1 semana)
