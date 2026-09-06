# [🟢 Fácil] Sons de Blocos + UI

## Objetivo
Adicionar sons faltantes para blocos e UI.

## Checklist
- [ ] `plants` → fallback `grass` ou som próprio
- [ ] `sandstone` → fallback `stone` ou som próprio
- [ ] `clay`
- [ ] `mish` (torch etc) → fallback `stone`
- [ ] `wood` — quebrar/colocar
- [ ] `wool` — quebrar/colocar
- [ ] UI `Sliders`
- [ ] UI `CheckBoxOn` / `CheckBoxOff`

## Arquivos
- `resources/sounds/` — adicionar/fallback em `Bricks/Clay/Wood/Wool/` etc
- `src/gameLayer/audioEngineRaudioBackend.cpp` — mapear `BlockTypes` → som
- `src/gameLayer/gameplay/blockUpdates.cpp` — tocar ao quebrar/colocar

## Como Testar
Quebrar cada tipo de bloco e clicar UI — ouvir som correto.

## Prioridade
🟢 Fácil — `soundsTodo.md`
