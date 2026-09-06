# [🟢 Fácil] Sons de Água

## Objetivo
Adicionar sons de água: entrar, sair e nadar.

## Checklist
- [ ] `water_in` — ao entrar na água
- [ ] `water_out` — ao sair da água
- [ ] `water_swim` — loop ao nadar (0.65s já existe `waterSwim`, verificar)

## Arquivos
- `resources/sounds/Water_Flowing/` — adicionar `water_in.ogg`, `water_out.ogg`, `water_swim.ogg`
- `src/gameLayer/audioEngineRaudioBackend.cpp` — tocar som em `player.isInWater` transição
- `src/gameLayer/gameplay/player.cpp` — detectar `headUnderwater` + trigger

## Como Testar
Entrar/sair da água e nadar — ouvir sons 3D.

## Prioridade
🟢 Fácil — `soundsTodo.md`
