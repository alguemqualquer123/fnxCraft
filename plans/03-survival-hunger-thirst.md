# [🟡 Médio] Fome/Sede — HUD + Depleção + Dano

## Objetivo
Implementar survival completo: fome/sede com HUD, depleção e dano.

## Checklist
- [ ] HUD barras fome/sede (`heartsTexture` já existe, adicionar barras)
- [ ] Depleção fome ao longo do tempo
- [ ] Depleção sede (mais rápido que fome)
- [ ] Dano inanição quando fome zerada
- [ ] Dano desidratação quando sede zerada
- [ ] Comer/beber restaura (`food.cpp` + `effects.h`)
- [ ] Receitas cozidas (furnace/cookingPot)
- [ ] Farming: `crop growth` precisa água+luz+tempo (tick condicional)
- [ ] Breeding animais (alimentar p/ reproduzir)

## Arquivos
- `include/gameLayer/gameplay/player.h` — `hunger/thirst` + `deplete()`
- `src/gameLayer/gameplay/food.cpp` — `eat()` restaura
- `include/gameLayer/rendering/UiEngine.h` — barras HUD
- `src/gameLayer/gameplay/entity.cpp` — dano starvation

## Como Testar
Jogar em survival, esperar fome cair, comer `bread` e ver restaurar.

## Prioridade
🟡 Médio — itens/blocos já existem (5/10), falta mecânica
