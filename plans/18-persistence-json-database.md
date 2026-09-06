# [🟡 Médio] Persistência JSON — JsonDatabase + JsonRepository

## Objetivo
Trocar `NullDatabase` stub por `JsonDatabase` real + `JsonRepository<T>` genérico.

## Checklist
- [x] `JsonDatabase` com `put/get/removeKey/listKeys` + `connect/disconnect/begin/commit/rollback` + `execute("FLUSH")`
- [x] `JsonRepository<T>` template com `IdFn`/`ToJsonFn`/`FromJsonFn`
- [x] `PersistenceManager::init` usa `JsonDatabase(dataPath/databases)` em vez de `NullDatabase`
- [x] Arquivo `src/gameLayer/persistence/JsonDatabase.cpp` (safeSave + escape JSON)
- [ ] Testar `JsonRepository<PlayerData>` round-trip save/load
- [ ] Migrar `PlayerStorage` para usar `JsonRepository` (opcional)
- [ ] Adicionar testes `put/get/listKeys` com múltiplas tabelas

## Arquivos
- `include/gameLayer/persistence/IDatabase.h` — `JsonDatabase` + `JsonRepository<T>`
- `src/gameLayer/persistence/JsonDatabase.cpp` — novo
- `src/gameLayer/persistence/PersistenceManager.cpp` — `JsonDatabase` no init

## Como Testar
`JsonDatabase db("data/databases"); db.connect(""); db.put("players","uuid1","{...}"); db.get(...)`

## Prioridade
🟡 Médio — feito, falta testes
