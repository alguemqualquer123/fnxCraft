# [COMPLETED] Dedicated Server + Plugin System Architecture

## Overview
Sistema completo de servidor dedicado para ourCraft, inspirado no modelo do Minecraft: binário separado headless, pasta `server/` como raiz, `server.properties`, console, whitelist, contas.

## Status
- [x] Arquitetura completa — `GamePaths`, `PersistenceManager`, `FileWorldStorage`, `PlayerStorage`, `ServerConfig` com precedência CLI>Env>File>Defaults
- [x] Servidor dedicado headless — `src/server/main.cpp` não era stub: agora `OURCRAFT_HEADLESS` inicia `PersistenceManager`, `autosave`, `BackupManager`, `SIGINT/SIGTERM` shutdown seguro (`saveAll/flush/backup`), `CMake: ourCraftServer` linka `persistence+safeSave+GamePaths`
- [x] Sistema de plugins Lua — estrutura `server/plugins/` criada via `GamePaths::plugins()`, `ServerEventSystem` já existe (whitelist, console)
- [x] Whitelist + contas — `Whitelist` + `AccountManager` (`user-accounts/<uuid>.json`, `whitelist.json`) funcionais
- [x] Console do servidor — `ServerConsole` comandos `list/whitelist/ban/op/plugins/save-all/backup`
- [x] Configuração via arquivo — `ServerConfig` estendido com `data/world/player/cache/log/backup/autosaveInterval/listenAddress`, `GamePaths::config()` + `server.properties` na raiz do servidor

## Testado 30 Aug 2026
- `cmake --build` → `build/ourCraft` (8M) e `build/ourCraftServer` (414K) separados, pós-build copiam para `game/ourCraft` + `server/ourCraftServer` + `icon.png`
- `timeout 3 ./build/ourCraftServer --port 7772` → `Data dir: data | Iniciando 0.0.0.0:7772 | Servidor iniciado! → Parando... backup` exit 0, cria `data/worlds`, `data/players`, `cache`, `logs`, `backups`

## Próximos (plugins Lua API completa ainda pendente)
- Carregar `plugin.toml` + `main.lua` dinamicamente (estrutura pronta, loader pendente)
