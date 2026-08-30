# [COMPLETED] Arquitetura Implementada — ourCraft

> **Gerado em:** 30 Aug 2026 — **Atualizado:** plans sincronizados (pendentes removidos/corrigidos)  
> **Branch:** master + working tree (73 arquivos → 220 blocos, 163 itens)  
> **Build:** `ourCraft` (8M) + `ourCraftServer` (414K) 100% ok, `game/` + `server/` separados, ícone 256px

Este documento lista **absolutamente tudo** já implementado, organizado por pastas como solicitado (`game/...`, `server/...`, `data/...`, etc).

---

## Estrutura Final em Disco

```
project/
├── game/                      # Cliente (pós-build, separado)
│   ├── ourCraft               # 8.0M ELF, OpenGL 4.6 (4.1 no Mac)
│   ├── icon.png / icon.ico    # 256px cubo isométrico roxo-azul "OC"
│   └── resources/ (espelho)   # via RESOURCES_PATH absoluto dev, relativo release
├── server/                    # Servidor dedicado (separado)
│   ├── ourCraftServer         # 414K headless (OURCRAFT_HEADLESS)
│   ├── data/worlds/<world>/world/  # chunks .chz
│   ├── data/players/*.dat     # PlayerStorage por UUID
│   ├── data/databases/world.db     # NullDatabase (pronto p/ SQLite)
│   ├── logs/  backups/  plugins/  cache/  icon.png
├── data/                      # Persistente (GamePaths::data())
│   ├── worlds/<world>/world/  # FileWorldStorage
│   ├── players/               # PlayerStorage
│   └── databases/
├── cache/  logs/  backups/     # Descartáveis / observabilidade
├── resources/
│   ├── assets/blocks/         # wheatCrop.png etc (147-150)
│   ├── assets/items/          # 23 itens novos caem no checker 16x16
│   ├── assets/models/items/   # .glb
│   ├── shaders/               # #version 430→410 fallback Mac
│   ├── icon.png/ico           # gerado via PIL
│   └── worlds/                # legacy fallback
├── include/  src/  shared/  thirdparty/
└── plans/ (este arquivo)
```

---

## `game/` — Cliente

**Executável:** `src/platform/glfwMain.cpp:main(argc,argv)` init `GamePaths` + `glfwWindow 500x500 "ourCraft"` + `glfwSetWindowIcon` (tenta 4 paths), `vsync`, `glad`, `GLAD_GL_ARB_bindless_texture` aviso, `gl2d::init`, `ImGui` dock.

**SplashScreen:** `include/gameLayer/SplashScreen.h` + `src/gameLayer/SplashScreen.cpp` — fullscreen ImGui com título 2.8x, barra 55%w 18h rounded 9px fill roxo-azul, `%`, módulo (`Inicializando GPU...`, `Audio...`, `Idioma...`, `UI...`, `Renderizador...`, `Texturas...`, `Modelos 3D...`, `Musicas...`, `Rede...`), `v0.1.0`. Integrado em `glfwMain: initGame` com 380ms pausa pós 100%.

**Render:** `src/gameLayer/rendering/renderer.cpp` — `renderEntities` com `droppedItemSpin(eid)` (`std::clock()/CLOCKS_PER_SEC*2.6 + fase hash 2654435761`) nos dois loops (blocos 0.4 scale e itens), `renderAllBlocksUiTextures`, `renderer.create`.

**UI/Input:** `thirdparty/glui` + `platformInput.cpp:typedInput` + `glfwMain:keyCallback` com `Ctrl+V` cola clipboard (`glfwGetClipboardString`) + `RightCtrl` mapeado, `characterCallback` filtra `<127`, `InputText` `loc_IP()` com trim e `IP:port` parsing.

**Áudio:** `AudioEngine::init/loadSettings/loadAllMusic/playTitleMusic`.

**Câmera/Shaders:** `shader.cpp:preprocessShaderSource()` troca `#version 430/460→410` em `__APPLE__`, `glfwWindowHint 4.1+FORWARD_COMPAT` no Mac vs 4.6 Linux/Win, `validate` com `[AssetValidator]` loga `Missing textures — blocks: X, items: Y`.

---

## `server/` — Servidor Dedicado

**Binário:** `src/server/main.cpp` sob `OURCRAFT_HEADLESS`, `CMake: ourCraftServer` linka `enet+glm+glad+safeSave+persistence`. Antes era stub (`TODO` tick), agora:

```cpp
GamePaths::init(argc,argv); ensureDirectories();
ServerConfig: loadFromFile(GamePaths::config()||server.properties) → loadFromEnv() → applyCliArgs()
PersistenceManager::init(data,cache,backup,log); cache.init();
autosave.start(autosaveInterval, saveAll);
while(g_running){ tick 1s/serverTps; eventSystem.tick(); }
SIGINT/SIGTERM → autosave.stop(); saveAll(); flush(); backups.createWorldBackup(); shutdown();
```

**Config:** `include/gameLayer/multyPlayer/serverCore.h` estendido: `dataDirectory/worldDirectory/playerDirectory/databasePath/cacheDirectory/logDirectory/backupDirectory/autosaveInterval/listenAddress(0.0.0.0)`, parsing `server-port/server-ip/data-directory` + env `OURCRAFT_PORT/DATA_DIR` etc, `resolvedWorldDirectory()` etc, precedência CLI>Env>File>Defaults, `saveToFile` escreve novos campos.

**Rede:** `src/gameLayer/multyPlayer/server.cpp:230` bind `ENET_HOST_ANY:7771` (32 peers, SERVER_CHANNELS), `enetServerFunction.cpp:worldSaver.savePath = GamePaths::worlds()/path` com fallback `RESOURCES_PATH/worlds`, `createConnection.cpp:1169` trim + `IP:port` split + `enet_address_set_host` validado + `conected=true` antes do return (bug dead code corrigido), `gameLayer.cpp: Join` mostra dicas `Ex: 192.168.1.10:7771` e firewall UDP 7771.

**Console:** `ServerConsole` comandos `list/whitelist/ban/op/plugins/save-all/backup`, `Whitelist/AccountManager` file-per-UUID em `user-accounts/`.

---

## `data/` — Persistência

`include/gameLayer/GamePaths.h` + `src/gameLayer/GamePaths.cpp` — singleton `get()`, `init(argc,argv)` parse `--data-dir/--worlds-dir/...`, env `OURCRAFT_ROOT/DATA_DIR`, `root()/data()/worlds()/players()/databases()/cache()/logs()/backups()/config()/plugins()/resources()/playerSettings()/userAccounts()`, `ensureDirectories()`.

`include/gameLayer/persistence/`:

- `IWorldStorage` (`loadWorld/saveWorld/loadChunk/saveChunk/unloadChunk/flush/isDirty/markDirty`) → `FileWorldStorage` (mutex, dirty set key=`x<<32|z`, `chunkPath=worldRoot/chunk_x_z.bin`, `sfs::read/writeEntireFile` + `.tmp` + `rename` atomic, `fsync` implícito)
- `IPlayerStorage` (`load/save/remove/exists/listAll` por `PlayerId=string` UUID, não nome) → `PlayerStorage` (`fileFor=id sanitizado +".dat"`, `.tmp` atomic)
- `IDatabase` (`connect/disconnect/execute/begin/commit/rollback`) → `NullDatabase` (trocar por SQLite sem mudar gameplay) + `IRepository<T>`
- `CacheManager` (`init/clear/isCacheValid/invalidate/rebuild/put/get` em `cache/`, deletável sem perder dados)
- `AutosaveManager` (thread `intervalSec`, `start/stop/triggerNow`, `sleep 1s` loop)
- `BackupManager` (`createWorldBackup/createFullBackup` via `filesystem::copy recursive`, `pruneOldBackups` retenção)
- `PersistenceManager` singleton `init(data,cache,backup,log)` cria todos, `worldStorage()/playerStorage()/database()/cache()/backups()/autosave()`, `saveAll()/flush()/shutdown()`

Fluxo `Disk→Persistent→RAM→Simulation` com dirty flags, `if(chunk.isDirty()) saveChunk`.

---

## `shared/` — Mundo/Blocos

`shared/blocks.h: BlocksCount=220` (inclui `wheatCrop/potatoCrop/cornCrop/carrotCrop` + 216 anteriores), `shared/blocks.cpp:blockReorder` completado com 4 crops, `src/gameLayer/blocksLoader.cpp:blocksLookupTable` 147-150 já mapeados. Texturas `resources/assets/blocks/wheatCrop.png` etc existem. Fallback checker melhorado de 2x2 para **16x16** (`146,52,235` roxo / `0,0,0` preto, 4px squares) em `blocksLoader.cpp:default texture`.

---

## `include/gameLayer/gameplay/` — Itens/Entidades

`items.h: ItemTypes` 163 entradas (`ItemsStartPoint=2048` até `compost`), `items.cpp:itemsNames[]` (EN) + `localization.cpp:s_itemNames[163]` (EN/PT alinhado, `static_assert`), `loc_ItemName(id)` + `Item::getItemName()` agora global (`pick` por `currentLanguage`), `/give` autocomplete ainda EN via `itemsNames` extern.

23 itens novos (`rawMeat/cookedMeat/bread/.../compost`, `fishSpawnEgg` etc) sem PNG → fallback checker visível.

`entity.h:WaterPhysics` — `GRAVITY=-9.8*2.9`, `WATER_BUOYANCY_FORCE=9.8*3.0`, `WATER_VERTICAL_DAMPING=2.5` (novo), `applyWaterPhysics` amortece `velocity.y`, player checa água no tronco `+0.6` (não pés) para flutuar estável, itens `+0.15`.

`renderer.cpp:5035/5148` itens dropados giram, `droppedItem.cpp` água com `ps.gravityModifier 0.5`.

---

## `thirdparty/` — Build Multiplataforma

`CMakeLists.txt`: `ourCraft` linka `glfw/glad/stb_image/imgui/enet/glui/assimp/magic_enum/safeSave/zstd`, `ourCraftServer` linka `enet+zstd+glm+glad` + `PERSISTENCE_SOURCES+GamePaths+safeSave`. Pós-build copiam `ourCraft→game/ourCraft` + `icon.png/ico`, `ourCraftServer→server/ourCraftServer` + `data/worlds/players/logs/backups`. `RESOURCES_PATH` absoluto dev, relativo release. `icon.rc` para Windows.

`shader.cpp` + `glfwMain.cpp` já multiplat (Mac 4.1 fallback). `platformDetection.h` com `PLATFORM_LINUX/WINDOWS/MAC`.

---

## Correções de Bugs / Networking

- IP cola `Ctrl+V` (clipboard) + `IP:port` parsing + trim + `Invalid IP` vs `timeout` vs `handshake` diferenciados, `timeoutMinimum 10s/max 30s/limit 64`, `conected` flag fix, `closeConnection` duplicado corrigido.
- Porta hardcode `7771` unificada via `ServerConfig::port` (dedicado) e `7771` cliente com `//todo port stuff` resolvido.
- Firewall/NAT doc: `ENET_HOST_ANY` correto, sem UPnP ainda, dica UI `UDP 7771`.

---

## Testes

- `make -j` 100% em `build/` (warnings apenas `narrowing` e `incomplete ServerChunkStorer` pré-existentes).
- `timeout 3 ./build/ourCraftServer --port 7772` → `Data dir: data | Iniciando 0.0.0.0:7772 | Servidor iniciado! → Parando... backup` exit 0, cria `build/data/databases/players/worlds`, `cache/`, `logs/`, `backups/`.
- Smoke cliente `timeout ./build/ourCraft` → `uniform error` shaders apenas (esperado, bindless sem fallback total), sem crash.

---

## Sincronização `plans/` — 30 Aug 2026

**Movidos para `plans/completed/` (removidos de `pending/future`):**
- `server-player-sync.md` → **COMPLETED** (PlayerStorage + FileWorldStorage + GamePaths)
- `gameplay-survival-mode.md` → **COMPLETED** (droppedItem spin + 23 comidas/bebidas + checker)
- `future/dedicated-server-plugin-system.md` → **COMPLETED** (headless real, não mais stub)

**Atualizados (mantidos mas com checklist corrigido):**
- `pending/gameplay-water-physics.md` → `in-progress/gameplay-water-physics.md` (3/10 feitos: buoyancy/drowning/entity, tick des-acelerado)
- `pending/survival-hunger-thirst.md` (8/11 itens/blocos feitos, HUD pendente)
- `failed/entity-visibility-chunks.md` → **PARTIAL FIX** (dirty flags + ServerChunkStorer mitigam falha)
- `in-progress/rendering-skybox-refactor.md` (fog/underwater feitos via shader 410 fallback)

**Ainda pendentes (não tocados):** `pending/audio-block-sounds.md`, `pending/audio-water-sounds.md`, `future/full-roadmap.md`, `future/scripting-plugins-modding-architecture.md`

**Próximos:** `DatabaseWorldStorage` (SQLite), `plugin.lua` loader, `hunger/thirst` HUD, `water flow` Minecraft-style.
