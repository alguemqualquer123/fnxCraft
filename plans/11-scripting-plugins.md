# [🔴 Avançado] Scripting/Plugins Lua

## Objetivo
Sistema extensível: Lua, plugins, hot reload, sandbox, permissões.

## Checklist
- [ ] `PluginManager` + manifest `.toml` parser
- [ ] `EventBus` + `ScriptingIntegration` (interface já existe)
- [ ] Lua Runtime + bindings + sandbox + hot reload
- [ ] `plugins/` dir + lifecycle (load/enable/disable)
- [ ] Permissões capability-based (`world.read/write`, `entity.*`, `network.*`)
- [ ] Native plugins `.so/.dll` (futuro), C# (futuro)

## Arquivos
- `include/gameLayer/scripting/EventBus.h` — já existe
- `include/gameLayer/scripting/ScriptingIntegration.h` — já existe
- `src/gameLayer/scripting/` — novo
- `server/plugins/` — dir
- `thirdparty/lua/` — já opcional

## Como Testar
Criar `plugins/test/main.lua` com `onPlayerJoin` hook, ver rodar.

## Prioridade
🔴 Avançado — `scripting-plugins-modding-architecture` (8 fases)
