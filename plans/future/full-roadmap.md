# Full Roadmap - ourCraft (Minecraft-like)

> Criado a pedido: implementar tudo de forma incremental, um por vez.

## Ordem de implementação

### 1. Arco carregado + flecha com física [ATUAL]
- Hold right-click carrega (0-1s), scale força 8-25 m/s
- Consome 1 flecha do slot `ARROWS_START_INDEX`
- Spawna `droppedItem` com velocity + gravidade, dano por raycast entidade
- UI barra de carga, som puxar/soltar
- Diferentes bows: wooden 8, copper 10, iron 15, etc.

### 2. Farming growth (água/luz/boneMeal)
- `seeds` plantáveis só em `dirt` com água num raio 4 e luz >8
- Crescimento por tick: checa água, luz, aleatório, estágios 0-7
- `boneMeal`/`fertilizer` avança estágio, `wateringCan` molha
- Quebrar estágio final dropa `wheat` + seeds

### 3. Fornalha / Furnace
- Bloco `furnace` com UI 3 slots (input, fuel, output)
- Combustíveis: `coal`, `wooden_plank`
- Receitas: ore -> ingot, rawFood -> cooked
- Progress bar, consome fuel, anima chama

### 4. F3+G chunk borders + melhorias F3
- F3 já feito, adicionar F3+G toggle render de bordas de chunk via `GyzmosRenderer`
- Mostrar `chunkSection` e `light level` no F3

### 5. Água infinita 2x2
- Quando 2x2 de source com cantos, centro vira source

### 6. Tocha luz piscando + partículas quebrar bloco
- `lamp` flicker via `lightSystem`, crack texture já existe
- Partículas ao quebrar

### 7. Som passos/água + mão balançando
- `AudioEngine` já tem fall sounds, adicionar swim/water
- Hand oscillator já existe, expandir para corrida

### 8. Minimapa melhorado (mapEngine já existe)
- Zoom, player marker, bioma color

### 9. Mobs AI melhor
- Goblin pathfinding já parcial, adicionar flee, group

### 10. Sistema de skills reutilizável (já criado `create-item`, `f3-debug`)
- Cada feature acima vira skill em `.opencode/skills/<nome>/SKILL.md`

## Skills já criadas
- `create-item` - checklist completo para novo item
- `f3-debug` - overlay F3

## Como executar
Cada item é um PR/commit separado. Rodar `cmake --build build --target ourCraft -j4` após cada.

