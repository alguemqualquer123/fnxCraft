# Finalizar Todos os Stubs

**Total:** 40 stubs — 34 entidades + 6 sistema

## Fase 1 — HtmlUiEngine (1 semana)
- [ ] Integrar RmlUi 4.x ou Ultralight SDK em `thirdparty/`
- [ ] `HtmlUiEngine.cpp` real: `Rml::Context::Create`, `LoadDocument(resources/ui/index.html)`, `Render()` → textura, `FileWatcher` hot-reload
- [ ] Bind JS: `htmlUi.createWorld`, `selectWorld`, `saveWorldConfig`, `openCargos` → C++ `hostServer`/`saveWorldConfig`
- [ ] Substituir `glui` nos menus `main-menu`, `world-config`, `cargos`

## Fase 2 — Animais Básicos (1 semana, 7 entidades)
- [ ] `cow.cpp` - quadrupede, pastar, ordenhar, drop leite/couro
- [ ] `sheep.cpp` - lã tosar, cor, drop lã
- [ ] `wolf.cpp` - domesticar com osso, seguir, atacar
- [ ] `fox.cpp` - roubar itens, dormir
- [ ] `crow.cpp` - voar, roubar brilho
- [ ] `bee.cpp` - polinizar, colmeia
- [ ] `manatee.cpp` - aquático, lento

## Fase 3 — Mobs Hostis (1.5 semanas, 12 entidades)
- [ ] `creeper.cpp` - explodir 3s, silenciar com tesoura
- [ ] `caveSpider.cpp` - veneno, teia
- [ ] `slime.cpp` - dividir ao morrer
- [ ] `skeleton.cpp` - arco, strafe
- [ ] `skeletonPirate.cpp` - variante navio
- [ ] `enderling.cpp` - teleporte
- [ ] `lavaSlug.cpp` - trilha fogo
- [ ] `sandSerpent.cpp` - enterrar areia
- [ ] `mistGhost.cpp` - invisível névoa
- [ ] `armoredBoar.cpp` - carga
- [ ] `crystalSentinel/Bat/Golem.cpp` - trio cristal

## Fase 4 — Bosses e NPCs (2 semanas, 15 entidades)
- [ ] `hydra.cpp` - 3 cabeças Fire/Ice/Venom, já tem header
- [ ] `queenBee.cpp` - rainha, spawna abelhas
- [ ] `honeyBear.cpp` - rouba mel
- [ ] `juvenileDragon.cpp` - voo, fogo
- [ ] `treeEnt.cpp` - gigante floresta
- [ ] `stoneGolem.cpp` / `riverGuardian.cpp`
- [ ] `mimicChest.cpp` - baú armadilha
- [ ] `lightFairy.cpp`, `hermitCrab.cpp`
- [ ] `blacksmithVillager`, `herbalistVillager`, `nomadTrader`, `capybaraChef` - trade UI

## Fase 5 — Sistemas
- [x] `JsonDatabase` já feito, remover `NullDatabase` legado
- [ ] `headlessGL.h` - implementar GL real ou remover (só server)
- [ ] `stubEntities.cpp` - apagar após todos `.cpp` criados (34→0)

## Verificação
- [ ] `scripts/test_automation.py` cobre spawn de cada mob via ovo
- [ ] `python3 scripts/test_automation.py` 9/9 + manual F5 3ª pessoa
- [ ] Performance: `workerThreads=4`, `viewDistance=8`, `maxLights=20`

Estimativa: 5-6 semanas, incremental 1 mob/dia. Começar por Fase 2.
