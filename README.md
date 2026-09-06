# fnxCraft

É a terceira vez que tento fazer Minecraft do zero. Desta vez com recursos difíceis: blocos transparentes, sombras com luz, e multiplayer!

Vídeos no [YouTube](https://www.youtube.com/watch?v=StNAG_tLEoU&list=PLKUl_fMWLdH-0H-tz0S144g5xXliHOIxC&index=4)

![image](https://github.com/user-attachments/assets/9f97b795-8f7e-4de0-abca-2945338721ca)
![image](https://github.com/user-attachments/assets/08b148c9-4c80-4cbc-83f1-c1ace1e61e0a)
![image](https://github.com/user-attachments/assets/d02a6717-8b47-4923-880d-1bc8e2574943)
![image](https://github.com/alguemqualquer123/fnxCraft/assets/36445656/7e57cdc4-6f6c-4cc9-bce5-c8ff9131ab55)
![image](https://github.com/alguemqualquer123/fnxCraft/assets/36445656/fd5ad17e-1bee-441d-8747-d4df4fdb850c)
![image](https://github.com/alguemqualquer123/fnxCraft/assets/36445656/3f6c8976-8f63-4259-a1de-3305c4c52467)

---

## Stack

| Camada | Tecnologia |
|--------|------------|
| Linguagem | C++17 / C17 |
| Gráficos | OpenGL 4.6 (4.1 no Mac), GLAD bindless + fallback |
| Janela/Input | GLFW 3.3.7, `gl2d`, `glui` |
| UI Debug | ImGui Docking |
| Áudio | raudio |
| Rede | ENet 1.3.18 UDP `0.0.0.0:7771` 32 peers |
| WorldGen | FastNoiseSIMD + FastNoise2, Voronoi, Splines, Easing |
| Matriz | GLM |
| Compressão | zstd 1.5.5 |
| Modelos | Assimp 5.2.4 (OBJ/GLTF/MTL) |
| Util | magic_enum 0.9.3, safeSave (.tmp atômico), profilerLib |
| Scripting | Lua (opcional) |
| Build | CMake 3.16, AVX2, MSVC static runtime |

## Como Compilar e Rodar

```sh
./scripts/configure.sh   # cmake -B build -S . -DCMAKE_POLICY_VERSION_MINIMUM=3.5
./scripts/build.sh       # cmake --build build --target fnxCraft -j$(nproc)
./scripts/build-all.sh   # build client + server
./scripts/run.sh         # ./build/fnxCraft | tee game_runtime.log (+ console externo + splash 380ms)
./scripts/run-server.sh  # ./build/fnxCraftServer
./scripts/run-console.sh # console dedicado (gnome-terminal)
./scripts/clean.sh       # rm -rf build
./scripts/dev.sh         # configure + build + run (Debug)
```

Binários: `game/fnxCraft` + `server/fnxCraftServer`. Recursos via `RESOURCES_PATH` (absoluto dev, relativo release). Servidor headless (`FNXCRAFT_HEADLESS=1`) cria `data/worlds/players/logs/backups/plugins`.

## Estrutura de Pastas (real no disco)

```
src/gameLayer/            35+ cpp — gameLayer, blocksLoader, worldGenerator, chunkSystem
src/gameLayer/rendering/  13 cpp — renderer, chunk, modelRenderer, skyBox, sunShadow, camera
src/gameLayer/gameplay/   31 cpp — player, zombie, goblin, fish, items, crafting, physics, ai
src/gameLayer/multyPlayer/ 10 cpp — server, tick, packet, undoQueue, waterSimulation
src/gameLayer/persistence/  6 cpp — FileWorldStorage, PlayerStorage, BackupManager, Autosave
src/platform/              5 cpp — glfwMain, platformInput, platformTools
src/server/                4 cpp — main (headless), serverCore, serverConsole, eventSystem
include/gameLayer/       142 headers (gameplay 66, multyPlayer 16, rendering 13, persistence 10)
shared/                   13 arquivos — blocks.h (309 blocos), biome, voronoi, splines, easing
resources/shaders/        47 shaders (rendering 16, postProcess 14, skyBox 7, root 10)
resources/assets/       2126 arquivos — blocks 1741 PNGs PBR, items, models/*.glb, sky, weather
resources/gameData/       39 structures (.structure) + worldGenerator/default.wgenerator
resources/sounds/         55 arquivos — 20 categorias + music/titleScreen/night
thirdparty/               19 libs (glfw, glad, stb, glm, imgui, enet, assimp, zstd...)
.opencode/skills/          4 skills — animation-system, create-item, f3-debug, karpathy-guidelines
scripts/                  10 .sh + generate_weather_textures.py
plans/                    15 .md — roadmap, completados, pendentes, em progresso
```

---

## ✅ O Que TEM (implementado e funciona)

### Rendering — PBR completo

| Feature | Status | Onde |
|---------|--------|------|
| PBR pipeline `_n/_s/_b` + BRDF LUT | ✅ | `blocksLoader.cpp`, `resources/assets/blocks/` 1741 PNGs |
| Água animada (DUDV + caustics) | ✅ | `defaultShader.frag` |
| HDR + ACES tone mapping | ✅ | `postProcess/toneMap.frag` |
| Auto-exposure | ✅ | `postProcess/*` |
| Bloom (addMips/filterBloom/gauss) | ✅ | `postProcess/bloom/*`, `addMipsShader`, `filterBloomData` |
| SSAO/HBAO | ✅ | `postProcess/hbao.frag`, `applyHBAO.frag` |
| SSR | ✅ | `postProcess/ssr.frag` |
| Fog + underwater fog | ✅ | `defaultShader.frag` |
| God rays | ✅ | `postProcess/radialBlur.frag` |
| CSM cascaded shadows | ✅ | `sunShadow.cpp` (otimizar pendente) |
| Fake shadows (todas as luzes) | ✅ | `lightSystem.cpp` luz 0-15 |
| Lens flare/dirt (9 tex) | ✅ | `otherTextures/lensFlare/` |
| FXAA, warp, maskDepth | ✅ | `postProcess/*` |
| Decals (transições grama) | ✅ | `rendering/decal.frag` |
| Fallback checker 16×16 rosa `146,52,235` | ✅ | `blocksLoader.cpp` |
| `#version 430→410` Mac fallback | ✅ | `shader.cpp` |
| `bigGpuBuffer`, `frustumCulling`, `GyzmosRenderer` | ✅ | `rendering/*` |
| `modelRenderer` Assimp GLB 6-bone skinning | ✅ | `modelRenderer.cpp` |
| Weather (rain/snow/lightning) | ✅ | `weatherRenderer.cpp` |

### Chunk / Mundo — 16×256×16

| Feature | Status | Onde |
|---------|--------|------|
| Chunk system + dirty flags | ✅ | `chunkSystem.cpp`, `chunk.cpp` |
| `ServerChunkStorer` + `chunkSaver` | ✅ | `multyPlayer/*` |
| WorldGen FastNoise2+SIMD+Voronoi+Splines | ✅ | `worldGenerator.cpp` 2416 linhas |
| 39 structures (.structure) | ✅ | `resources/gameData/structures/` |
| Biomas (`biome.h`) | ✅ | `shared/biome.*` |
| Continentalness/montanhas/rios | ✅ básico | `worldGenerator.cpp` |

### Blocos — 309 `BlockTypes` (`shared/blocks.h`)

`air(0)` → `fire` + `BlocksCount`. Inclui: `grassBlock`, `dirt`, `stone`, `sand`, `gravel`, `clay`, ores (`gold/copper/iron/silver/tin/mithril`), logs (`woodLog`, `birch_log`, `jungle_log`, `palm_log`), `water` (nível 0-7), plantas (`grass`, `rose`, `cactus_bud`), crops (`wheat/potato/corn/carrotCrop` stage 0-7), `furnace`, `lamp/torch/goblinTorch`, `fence/wall/stairs/slab`, `redstoneDust/Torch/Lamp` (power 0-15). Flags: rotação, `lightLevel`, `waterLevel`, `cropStage`, `redstonePower`, 6 faces, `isGrassMesh`/`isColidable`/`isAnyPlant`.

### Itens — 205 `ItemTypes` (`include/gameLayer/gameplay/items.h`, `ItemsStartPoint=2048` → `lastItem` sentinel)

- Ferramentas: `copper/lead/iron/silver/gold` Pickaxe/Axe/Shovel/Sword (power 25-100)
- Armas: `WarHammer/Spear/Knife/BattleAxe` + `training*` + `copper*`
- Arcos (7): `wooden/copper/lead/iron/silver/gold/goblinBow` — hold `RMB` 0-1.1s, força 9-26 m/s, barra central, consome flecha `ARROWS_START_INDEX`, spawna `droppedItem` com física
- Spawn eggs, 4 moedas, 4 flechas, 16 poções, 15 tintas, armaduras `leather→gold`, `tin/mithril` ingot/block, `flint/lighter`
- Comida/bebida: 9 comidas + 5 bebidas + 4 seeds + `boneMeal/fertilizer/wateringCan/compost` (itens existem, HUD pendente)
- Stack 1/100/999, `getItemStats`/`getWeaponStats`, `PlayerInventory` 63 slots + cursor/armour/offhand

### Entidades — 30 com `.cpp` implementado, 36 só header (stub)

| Entidade | Status | Nota |
|----------|--------|------|
| `Player` | ✅ | completo |
| `Zombie` | ✅ | 510 linhas, AI básica |
| `Pig` | ✅ |  |
| `Cat` | ✅ |  |
| `Goblin` | ✅ | hearBonus+0.3 se <40hp, foge se <18 |
| `Fish` | ✅ | + waterPhysics |
| `ScareCrow` | ✅ |  |
| `TrainingDummy` | ✅ |  |
| `DroppedItem` | ✅ | spin 2.6 rad/s, gravidade 0.5 |
| + `ai`, `animationSystem`, `physics`, `battleUI`, `food`, `crafting`, `loot`, `particleSystem` etc | ✅ | 30 cpp no total |
| `Creeper`, `CaveSpider`, `CrystalSentinel/Bat/Golem`, `Slime`, `QueenBee`, `HoneyBear`, `HermitCrab`, `LightFairy`, `JuvenileDragon`, `Hydra`, `TreeEnt`, `MimicChest`, `ArmoredBoar`, `MistGhost`, `Enderling`, `Cow`, `Sheep`, `Wolf`, `Fox`, `Crow`, `Bee`, `Manatee`, `Skeleton`, `LavaSlug`, `SandSerpent`, `RiverGuardian`, `StoneGolem`, `SkeletonPirate`, villagers (`Blacksmith`, `Herbalist`, `NomadTrader`, `CapybaraChef`) + `fishing` | ❌ stub | 36 headers sem `.cpp` |

Sistemas: `animationSystem` 19 bones (`AnimationClip/BlendTree/StateMachine` → `glm::mat4 m[6]`), `physics` (`GRAVITY -9.8*2.9`, `WATER_BUOYANCY 9.8*3.0`, `WATER_DAMPING 2.5`), `particleSystem` 6 cubos 0.14 vida 0.6s, `life`, `damageNumbers`, `swingTrail`, `battleUI`, `effects`, `lootTables`.

### Gameplay — o que funciona

| Feature | Status | Detalhe |
|---------|--------|---------|
| Crafting | ✅ | `crafting.cpp` — tools/armour/food/furnace, `cookingPot`, `anvil` |
| Farming blocos | ✅ | `wheat/potato/corn/carrotCrop` cross stage 0-7 |
| Farming plantio | ✅ | `seeds` em `dirt` com água raio 4 |
| Farming boosters | ✅ | `boneMeal`+1, `fertilizer`+3, `compost`+2, `wateringCan` 50% |
| Farming crescimento | ✅ | tick servidor 2s |
| Farming drop | ✅ | stage7 dropa `wheat`+2 seeds |
| Fornalha fuel | ✅ | `wooden_plank` consome, filtra UI se sem fuel |
| Água espalha 8 blocos nível 0-7 | ✅ | `waterSimulation.cpp` |
| Água infinita 2×2 | ✅ | ≥2 sources adjacentes/diagonal vira source |
| Água evapora se isolada | ✅ |  |
| Buoyancy player/itens | ✅ | `+0.6` tronco, `+0.15` itens, `WATER_DAMPING 2.5` |
| Drowning | ✅ | `isInWater/isSwimming`, `drowningTimer` |
| Arco 7 bows | ✅ | hold 0-1.1s, 9-26 m/s |
| Skins 64×64/32→128 | ✅ | `loadPlayerSkin` 8 skins |
| Partículas quebrar bloco | ✅ | 6 cubos 0.14 |
| Tocha flicker 11-15 @7Hz | ✅ | `sin(flick+hash)` |
| `waterSwim` loop 0.65s | ✅ |  |
| Mão balanço `0.22` | ✅ | `132°/6°/8°` |

### Multiplayer — ENet 1.3.18

| Feature | Status | Onde |
|---------|--------|------|
| Bind `0.0.0.0:7771` 32 peers | ✅ | `server.cpp` |
| `ServerConfig` CLI>Env>File | ✅ | `serverCore.h` |
| `createConnection` IP:port+clipboard | ✅ | `createConnection.cpp` trim + `Ctrl+V` |
| `Packet`, `ServerChunkStorer`, `chunkSaver` | ✅ | `multyPlayer/*` |
| `tick.cpp` 1s/serverTps | ✅ |  |
| `undoQueue` + `RubberBand` 200ms | ✅ |  |
| Validação de movimento | ✅ |  |
| `serverConsole` `list/whitelist/ban/op/plugins/save-all/backup` | ✅ |  |
| Handshake | ✅ |  |
| Entidades sync | ✅ | todas as 43 entidades (`tick.cpp` 0.5s `headerUpdateGenericEntity` por chunk carregado) |

### Persistência

| Feature | Status | Onde |
|---------|--------|------|
| `GamePaths` singleton | ✅ | `--data-dir`, `FNXCRAFT_ROOT/DATA_DIR` |
| `FileWorldStorage` `.bin` + `.tmp` atômico | ✅ | `persistence/FileWorldStorage.cpp` |
| `PlayerStorage` UUID `.dat` | ✅ | `persistence/PlayerStorage.cpp` |
| `CacheManager` | ✅ |  |
| `AutosaveManager` 1s | ✅ |  |
| `BackupManager` prune | ✅ |  |
| `PersistenceManager` fachada | ✅ |  |
| `JsonDatabase` | ✅ | `put/get/removeKey/listKeys` + `connect/begin/commit/rollback` JSON em `data/databases/*.json` (safeSave) |
| `JsonRepository<T>` | ✅ | template `IdFn/ToJsonFn/FromJsonFn` — `save/load/remove/listIds` via `JsonDatabase` |
| `NullDatabase` | ⚠️ legado | mantido p/ compatibilidade, `PersistenceManager` agora usa `JsonDatabase` |

### Áudio — raudio 3D

| Feature | Status | Onde |
|---------|--------|------|
| `AudioEngine` 3D | ✅ | `audioEngineRaudioBackend.cpp` |
| `resources/sounds/` 55 arquivos | ✅ | `Bricks`, `Clay`, `Crates`, `Cracking`, `Death`, `Entities`, `Fire`, `Hit`, `Hurt`, `Ice`, `Metal`, `Paint`, `Sandstone`, `VolcanicRock`, `Water_Flowing`, `Wood`, `Wool` |
| `music/titleScreen` + `night` | ✅ | `.ogg` |
| `waterSwim` loop | ✅ |  |
| `water_in/out/swim` | ❌ | `soundsTodo.md` pendente |
| `plants/sandstone/clay/mish/wood/wool` fallback | ❌ | pendente |
| `Sliders/CheckBoxOn/Off` | ❌ | pendente |

### UI / Debug / Plataforma

| Feature | Status | Onde |
|---------|--------|------|
| `UiEngine` + `gl2d` + `ImGui docking` + `glui` | ✅ |  |
| `SplashScreen` 380ms 55%w roxo-azul | ✅ | `SplashScreen.cpp` |
| F3 overlay (FPS/XYZ/Block/Chunk/Facing/Biome/Light/Entities) | ✅ | `F3` |
| F3+G chunk borders 5×5 | ✅ | `GyzmosRenderer.drawLine` vermelho/amarelo |
| F5 3 modos câmera raycast 2-10 | ✅ | `Scroll` distância, mão só 1ª pessoa |
| Minimapa zoom 0.6-10 marcador vermelho | ✅ | `mapEngine.cpp` |
| Inventário 63 slots + cursor | ✅ |  |
| Inventário criativo com busca | ✅ | `INVENTORY_TAB_BLOCKS`/`ITEMS` com busca filtrada, grid 9×6 paginado (`UiEngine.cpp`) |
| `glfwMain` 500×500 vsync ícone 4 tamanhos | ✅ |  |
| `localization` EN/PT | ✅ |  |
| `platformInput` `Ctrl+V` + `RightCtrl` | ✅ |  |

### Skills (`.opencode/skills/` — 4 existem, 22 planejadas)

| Skill | Status | Trigger |
|-------|--------|---------|
| `animation-system` | ✅ | animação, skeleton, bones |
| `create-item` | ✅ | criar item, novo item |
| `f3-debug` | ✅ | F3, debug overlay |
| `karpathy-guidelines` | ✅ | — |
| `create-block`, `create-entity`, `create-biome`, `create-structure`, `create-crafting-recipe`, `create-shader`, `create-audio-sound`, `create-persistence`, `create-network-sync`, `create-particle-effect`, `create-weapon-tool`, `create-armor`, `create-crop-farming`, `create-redstone`, `create-ui-hud`, `create-potion-effect`, `water-physics`, `create-light`, `create-furnace-recipe`, `create-worldgen-feature`, `pro-texture-shader-ideas` | ❌ | planejadas (`plans/README.md`) |

---

## ❌ O Que NÃO TEM / Falta Fazer

### 🟢 Fácil — dá para fazer em 1-3 dias cada

- [ ] Sons `water_in`, `water_out`, `water_swim` (`soundsTodo.md`)
- [ ] Sons fallback `plants→grass`, `sandstone→stone`, `clay`, `mish` (torch)→stone, `wood`, `wool`
- [ ] Sons UI `Sliders`, `CheckBoxOn`, `CheckBoxOff`
- [ ] HUD fome/sede — barras, depleção, dano inanição (`plans/pending/survival-hunger-thirst.md` 5/10 feito, falta HUD/depleção/breeding). Itens existem, mecânica não
- [ ] `transparentGeometryCounter` — cache p/ não bakear chunk sem transparência (`hardertodos.md`)
- [ ] Mover `world/`+`settings/` para fora de `resources/` (`todo.txt`)
- [ ] `shrink UVs` levemente p/ modelos (`todo.txt`)
- [ ] `load only dimensions` (`todo.txt`)
- [ ] Câmera não mover ao sair do inventário (`hardertodos.md`)
- [ ] `sendPlayerOtherInfo` (`hardertodos.md`)
- [ ] Item dropping + survival — rejeitar drop recria inventário (`hardertodos.md`)
- [x] F3 polish: `chunkSection`, `light level`, `simulationDistance`
- [ ] F3 polish (resto): `seed` no F3, cor borda por Y
- [ ] Farming polish: `blockNames` cor por stage, `boneMeal` partícula verde, `F3` mostra `light`/`hasWater`
- [x] Tocha flicker em `torch/torchWood/goblinTorch/lamp` (via `sin(flick+hash)` em `gamePlayLogic.cpp`)
- [ ] Skins grid paginado 4×4 preview 32×32
- [ ] View distance sem resetar chunks (`todo.txt`)

### 🟡 Médio — 1-2 semanas cada

- [ ] **SkyBox refactor** (`plans/in-progress/rendering-skybox-refactor.md`) — sun/fog/underwater/day-night/reflexões cubemap. Hoje funciona mas com `skyBox reflection` pendente
- [ ] **Culling entidades** por chunk visível (`plans/failed/entity-visibility-chunks.md` — parcial, `ServerChunkStorer` mitiga)
- [ ] **Água flow completo** (`plans/in-progress/gameplay-water-physics.md` 3/10 feito) — falta flow Minecraft direcional, `activeWaterChunks` perf, ondas/refração, som splash
- [ ] **Rios/continentalness** — rios entrarem no oceano, rios altura planícies, continentalness/montanhas mais vastos (`todo.txt`)
- [ ] **Mover sprites restantes** in-game (`todo.txt`)
- [ ] **Fornalha UI 3 slots** — `FurnaceBlock` real com input/fuel/output + barra progresso + `coal` 8s + chama (`plans/future/full-roadmap.md` #3)
- [x] **JsonDatabase** — FEITO (`JsonDatabase` JSON `data/databases/*.json` + `JsonRepository<T>`) — `NullDatabase` legado mantido
- [x] **Inventário Criativo** — FEITO (`INVENTORY_TAB_BLOCKS`/`ITEMS` com busca `itemSearchBuf`, grid paginado, `isCreative` infinito)
- [ ] **Minimapa bioma cor** (`full-roadmap` #8)
- [ ] **Goblin AI grupo** — pathfinding+flee já, falta grupo (`full-roadmap` #9)
- [ ] **Áudio blocos/água** (`plans/pending/audio-*.md`)
- [ ] **Cleanup no exit** — server salvar ao sair (`hardertodos.md`)
- [ ] **Player push** por outros players (`hardertodos.md`)
- [ ] **Render player/zombie** correto + **line drawing** blocos/colisões (`hardertodos.md`)
- [x] **Enviar todas entidades ao join** — FEITO (`tick.cpp` 0.5s broadcast todas as 43 por chunk carregado)

### 🎨 Renderização Gráfica — Falta / Melhorar

#### Shaders — faltam ou precisam melhorar (47 existem)

- [ ] **Bloom reativar** — desabilitado (`renderSettings.h: bloom=1 // disabled - causes black screen, re-enable after skybox fix`)
- [ ] **SSR melhorar** — `ssr.frag` com `//todo check/test`, `F0` metallic, `pow(roughness)` pendente
- [ ] **HBAO/SSAO polish** — funciona mas sem tuning fino
- [ ] **Depth of Field** blur distante (`README antigo` — não existe)
- [ ] **Motion blur** — não existe
- [ ] **Volumetric fog/lighting** — não existe (só fog linear)
- [ ] **Parallax occlusion mapping** — PBR tem `metallic/roughness` mas sem parallax
- [ ] **Luzes em cube maps** — `pointLight.h` existe mas sombras cube map não (`README antigo`)
- [ ] **SkyBox reflections** — cubemap `overworld_cubemap` existe mas reflexão PBR não (`hardertodos.md: skybox reflections`)
- [ ] **Shader unificado** — hoje `defaultShader` + `blockEntity` + `itemEntity` + `basicEntity` separados (`README antigo: use same shader for all`)
- [ ] **Água UVs contínuas** — `defaultShader.frag: //todo water uvs so the texture is continuous on all sides`
- [ ] **Água light sub-scatter** — `defaultShader.frag: //todo light sub scatter`
- [ ] **Água darken deep** — `//darken deep stuff, todo reenable and use final depth`
- [ ] **Water DUDV/normal** — `u_dudv`, `u_dudvNormal`, `u_caustics` existem mas `waterMove` precisa polish
- [ ] **Decal/crack shader** — `decal.frag` usa `zpass` mas `//todo change, also reuse in decal shader`
- [ ] **Weather shaders** — `weatherParticles/Lens/Flash` existem mas sem integração chuva/neve densa

#### Texturas — faltam ou precisam melhorar (2126 assets, 1741 blocks, 37 items)

- [ ] **23 itens sem PNG** caem no checker rosa `146,52,235` (`rawMeat`, `compost`, `fertilizer` etc — `blocksLoader.cpp` fallback 16×16)
- [ ] **PBR `_n/_s` incompleto** — muitos blocos sem normal/specular (`_n.png`/`_s.png` auto-gen mas sem artista)
- [ ] **Shrink UVs** levemente p/ modelos (`todo.txt: shrink UVs extremely slightly for the models` — evita bleeding)
- [ ] **Mover sprites restantes** in-game (`todo.txt: move the remaining sprites in game`)
- [ ] **Texture packs** incompleto — `renderSettings.cpp: //TODO delete unused entries`, `getUsedTexturePacksAndResetFlag` parcial
- [ ] **Anisotropy/MSAA/FSR/VSync** — settings em `ShadingSettings` (`anisotropy=4`, `msaa=0`, `fsr=0`, `vsyncMode=1`) mas sem implementação total
- [ ] **BRDF LUT** — `otherTextures/brdf.png` existe mas sem `preFilterSpecular` tuning (`skyBox/preFilterSpecular.frag: //todo obtain resolution in shader`)
- [ ] **Lens dirt/flare** — 9 `lensFlare/` + `lensDirt.png` existem mas `applyBloomData` `u_waterDropsPower`/`u_hitIntensity` sem polish

#### Renderização — faltam ou precisam melhorar

- [ ] **SkyBox refactor completo** — `hardertodos.md: big refactor for SKYBOX! + fix sun and fog and underwater stuff and day night and skybox reflections` (sun fix, day/night, reflections pendente)
- [ ] **Fog improve** — `defaultShader.frag` fog existe mas `README antigo: Fog -(todo improve)` + `ShadingSettings.fogGradient=16.f` precisa tuning
- [ ] **Underwater fog improve** — `defaultShader.frag` underwater existe mas `README antigo: Underwater fog -(todo improve)` + `underwaterDarkenStrength=0.94`
- [ ] **God rays improve** — `postProcess/radialBlur.frag` existe, `renderer.cpp: fboSunForGodRays` mas sem volumétrico real
- [ ] **Fake Shadows improve** — existe mas `README antigo: Fake Shadows for all light types (todo improve)`
- [ ] **Shadows optimize** — `sunShadow.cpp` CSM existe mas `README antigo: Shadows (todo optimize)` + `renderer.cpp: //TODO OPTIMIZE!`
- [ ] **Transparent geometry** — `chunk.cpp: transparentGeometry` existe mas `hardertodos.md: transparentGeometryCounter cache to skip bake if not necessary` pendente
- [ ] **Depth peeling água** — `renderer.h: u_depthPeelwaterPass`, `u_hasPeelInformation`, `u_PeelTexture` existem mas incompleto
- [ ] **Z prepass optimize** — `renderer.h: zprepass=1`, `zpassShader` existe mas `//todo optimize with buffer storage`
- [ ] **BigGpuBuffer modernizar** — `bigGpuBuffer.cpp: //todo look into a modern function here`, `glBufferData` → `glBufferStorage`
- [ ] **Texture binding optimize** — `renderer.cpp: //todo optimize texture binding so it is done only once`
- [ ] **Chunk sorting** — `renderer.h: sortChunks=1` mas `//todo only copy chunks that are close`, `maxLights` sem culling
- [ ] **Frustum culling polish** — `frustumCulling.cpp` existe mas entidades ainda sem culling por chunk
- [ ] **Gyzmos line far distance** — `renderer.h: GyzmosRenderer::drawLine //todo not working at far distances rn`
- [ ] **Player/zombie rendering correto** — `hardertodos.md: propper player and zombie rendering` (hoje `basicEntityShader` + 6 bones mas sem polish)
- [ ] **Line drawing colisões** — `hardertodos.md: propper line drawing for placing blocks and for drawing collisions` (Gyzmos incompleto)
- [ ] **Model freeing** — `model.cpp: //todo check if it frees all of them`, `//todo implement` skinning
- [ ] **Camera rotate** — `camera.cpp: //todo better rotate function`
- [ ] **Vignette/toneMap polish** — `toneMap.frag` com `saturation/vibrance/gamma/shadowBoost/highlightBoost/vignette/lift/gain` mas sem UI tuning fino

### 🔴 Avançado — semanas/meses

- [ ] **Buffering** multiplayer (README antigo)
- [ ] **Redstone completo** (`plans/future/next-cycle.md` #1) — `Repeater`/`Piston` + BFS `redstoneSimulation.cpp` `power-1`, tocha inverte, delay 2 ticks. Hoje só `Dust/Torch/Lamp` power 0-15 sem simulação
- [ ] **Montarias** (`next-cycle` #2) — `HorseServer/Client`, `saddle`/`horseArmor`, `WASD` controla cavalo
- [ ] **Modding/Scripting Lua** (`plans/future/scripting-plugins-modding-architecture.md`) — loader `plugins/`, `EventBus`/`ScriptingIntegration` só interface hoje
- [ ] **Otimização CSM/shadows** (`todo optimize` README antigo)
- [ ] **36 entidades stub → implementar `.cpp`** — `Creeper`, `CrystalSentinel`, `Slime`, `QueenBee`, `Hydra`, `TreeEnt`, `MimicChest` etc (66 headers → 30 cpp hoje)
- [ ] **Structures spawn** — 39 `.structure` existem mas spawn não testado/documentado
- [ ] **Multiplayer buffering + validação completa** + `Buffering` + `RubberBand` avançado

---

## Changelog Recente (Unreleased 2026-08-30)

F3 Debug, Câmera F5 raycast 2-10, Água Minecraft 8 blocos infinita 2×2, Farming 4 crops stage 0-7, Fornalha fuel `wooden_plank`, 7 Bows 9-26 m/s, Skins 64×64/32→128, F3+G 5×5 bordas, 6 cubos partículas 0.14/0.6s, Tocha flicker 11-15 @7Hz, `waterSwim` loop, Minimapa zoom 0.6-10, Goblin hearBonus+0.3/flee<18, 8 scripts, 2 skills. Ver `CHANGELOG.md`.

## Licença

Ver `LICENSE`.
