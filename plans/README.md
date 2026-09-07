# Plans — fnxCraft

> **REGRA:** `plans/` contém **só** o que falta fazer. Cada arquivo `.md` é uma task pendente.
> Quando concluir, **remova o arquivo de `plans/`** e **adicione em `README.md` na seção ✅ O Que TEM**.

## Índice (tasks pendentes)

| # | Task | Prioridade | Arquivo |
|---|------|------------|---------|
| 1 | Sons de água (`water_in/out/swim`) | 🟢 Fácil | `01-audio-water-sounds.md` |
| 2 | Sons de blocos (`plants/sandstone/clay/mish/wood/wool`) + UI | 🟢 Fácil | `02-audio-block-sounds.md` |
| 3 | Fome/sede HUD + depleção + dano | 🟡 Médio | `03-survival-hunger-thirst.md` |
| 4 | Física da água completa (flow, pressão, visuais) | 🟡 Médio | `04-water-physics.md` |
| 5 | SkyBox refactor (sun/day-night/reflexão) | 🟡 Médio | `05-skybox-refactor.md` |
| 6 | Culling entidades por chunk visível | 🟡 Médio | `06-entity-visibility.md` |
| 7 | Fornalha UI 3 slots + `coal` + barra progresso | 🟡 Médio | `07-furnace-ui.md` |
| 9 | Redstone básico (pó/tocha/repeater/pistão) | 🔴 Avançado | `09-redstone-basic.md` |
| 10 | Montarias (saddle, domar, montar) | 🔴 Avançado | `10-mounts.md` |
| 11 | Scripting/Plugins Lua (`plugins/` + EventBus) | 🔴 Avançado | `11-scripting-plugins.md` |
| 12 | Fixes fáceis (câmera, UVs, world/settings, etc) | 🟢 Fácil | `12-easy-fixes.md` |
| 13 | WorldGen rios/continentalness + sprites | 🟡 Médio | `13-worldgen-polish.md` |
| 14 | 36 entidades stub → implementar `.cpp` | 🔴 Avançado | `14-entities-stub.md` |
| 15 | Shaders — Bloom/SSR/água/DoF | 🟡 Médio | `15-rendering-shaders.md` |
| 16 | Texturas — PBR/UVs/sprites | 🟢 Fácil | `16-rendering-textures.md` |
| 17 | Render otimização + fixes | 🟡 Médio | `17-rendering-optimization.md` |
| 18 | Persistência JSON (JsonDatabase + Repository) | 🟡 Médio | `18-persistence-json-database.md` |
| 19 | Sync completo entidades (todas, não só players) | 🟡 Médio | `19-entity-sync-full.md` |
| 20 | Stack de UI unificada + bridge (RmlUi/ImGui/CEGUI/Nuklear/Ultralight + Lua) | 🔴 Avançado | `20-ui-stack.md` |

## Como Usar

1. Escolha uma task de `plans/`
2. Implemente seguindo o checklist do `.md`
3. `cmake --build build -j4` + teste
4. **Remova** o `.md` de `plans/` e **adicione** em `README.md` ✅
5. Commit

## Template

Ver `TEMPLATE.md` para criar nova task.
