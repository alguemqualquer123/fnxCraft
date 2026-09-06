# Plans — ourCraft

> Diretório de planejamento. Tudo que já existe no jogo está consolidado em `ROADMAP.md`.

## Estrutura Atual

```
plans/
├── README.md      # este arquivo
├── ROADMAP.md     # inventário completo + roadmap master (225 blocos, 172 itens, 22 skills)
├── completed/     # vazio - histórico movido para ROADMAP FASE 0
├── in-progress/   # vazio - ver ROADMAP FASE 1
├── pending/       # vazio - ver ROADMAP FASE 2
├── future/        # vazio - ver ROADMAP FASE 3-5
└── failed/        # vazio - ver ROADMAP FASE 1.3 (entity visibility parcial)
```

> **Histórico:** 14 .md antigos (`COMPLETED-IMPLEMENTED.md`, `TEMPLATE.md`, `gameplay-survival-mode.md` etc) foram consolidados em `ROADMAP.md` em 30 Aug 2026 e removidos. Git mantém histórico.

## Inventário Rápido (o que já tem no jogo)

| Sistema | O que existe | Onde |
|---------|--------------|------|
| **Blocos** | 225 `BlockTypes` (`air`→`redstoneLamp`), 6 faces por bloco, PBR `_n/_s/_b` auto-gen | `shared/blocks.h`, `src/gameLayer/blocksLoader.cpp` (1696 PNGs em `resources/assets/blocks/`) |
| **Itens** | 172 `ItemTypes` (`stick`→`goblinBow`, 163→172 após crops/bows), stack 1/100/999, tools 25-100 power | `include/gameLayer/gameplay/items.h`, `src/gameLayer/gameplay/items.cpp` |
| **Entidades** | `Player`, `Zombie`, `Pig`, `Cat`, `Goblin` (foge/hearBonus), `Fish`, `ScareCrow`, `TrainingDummy`, `DroppedItem` (spin 2.6rad/s) | `include/gameLayer/gameplay/*.h`, `src/gameLayer/gameplay/` (27 arquivos) |
| **Render** | PBR, SSAO/HBAO, SSR, HDR ACES, Bloom, Fog/underwater, God rays, CSM shadows, água animada, `GLAD bindless` + fallback, `#version 430→410` Mac | `include/gameLayer/rendering/` (11 headers), `resources/shaders/` (11 + rendering/ skyBox/ postProcess/) |
| **Áudio** | `raudio` backend, `AudioEngine` 3D, `resources/sounds/` (Bricks/Clay/Sandstone/Water/Hit etc) + `resources/music/titleScreen/night` | `include/gameLayer/audioEngine.h` |
| **Mundo** | `FastNoise2`+`SIMD`, `Voronoi`, `Splines`, `Easing`, chunks 16x256x16, rios/ocean/continentalness, estruturas `structure.h`, `worldGeneratorSettings` | `shared/voronoi.*`, `src/gameLayer/worldGenerator.cpp` |
| **Rede** | ENet 1.3.18 `0.0.0.0:7771` 32 peers, `ServerConfig` CLI>Env>File, `createConnection` IP:port+clipboard, `ServerChunkStorer`, `Packet`, `RubberBand` 200ms | `include/gameLayer/multyPlayer/` (16 headers) |
| **Persistência** | `GamePaths` singleton, `FileWorldStorage` (`chunk_x_z.bin` .tmp atomic), `PlayerStorage` UUID `.dat`, `NullDatabase`, `CacheManager`, `Autosave` 1s, `BackupManager` | `include/gameLayer/persistence/` (9 headers) |
| **Gameplay** | Crafting `crafting.cpp` (tools/armour/food/furnace), Hunger/Thirst items (9 comidas+5 bebidas), Farming 4 crops stage 0-7 água raio4 + boneMeal, Pesca, Fornalha fuel `plank`, Arco 7 bows hold 0-1.1s 9-26 m/s, Redstone 3 blocos (`Dust/Torch/Lamp` power 0-15) | `src/gameLayer/gameplay/crafting.cpp`, `shared/blocks.h:water/crop/redstone` |
| **UI** | `UiEngine` + `gl2d` + `ImGui docking`, `SplashScreen` 380ms, F3 overlay, F3+G chunk borders, Minimapa zoom 0.6-10, Inventário 63 slots + creative | `include/gameLayer/rendering/UiEngine.h` |
| **Skills** | 22 skills em `.opencode/skills/` (`create-block`, `create-entity`, `pro-texture-shader-ideas` etc) | `.opencode/skills/*/SKILL.md` |

## Skills Disponíveis

```
create-block, create-entity, create-biome, create-structure, create-crafting-recipe,
create-shader, create-audio-sound, create-persistence, create-network-sync,
create-particle-effect, create-weapon-tool, create-armor, create-crop-farming,
create-redstone, create-ui-hud, create-potion-effect, water-physics, create-light,
create-furnace-recipe, create-worldgen-feature, create-item, f3-debug,
pro-texture-shader-ideas
```

Ver `ROADMAP.md` para checklist por fase e ` .opencode/skills/<nome>/SKILL.md` para como usar.

## Como Usar

1. Ler `ROADMAP.md` → escolher fase `FASE 1` (em progresso) ou `FASE 2` (pendente)
2. Usar skill: `opencode` detecta `SKILL.md` por `description` — ex: "criar bloco" → `create-block`
3. Após implementar: `cmake --build build -j4` + `timeout 3 ./build/ourCraftServer --port 7772` + `timeout ./build/ourCraft`
4. Marcar `[x]` em `ROADMAP.md` e commit `PR` separado

## Categorias

`rendering` `server` `gameplay` `audio` `worldgen` `ui` `performance` `bugfix`
