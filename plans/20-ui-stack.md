# [PRIORIDADE] Stack de UI Unificada + Bridge Única (RmlUi / Dear ImGui / CEGUI / Nuklear / Ultralight)

## Objetivo

Definir e implementar a stack de UI do fnxCraft com **papéis fixos por ferramenta** e uma
**bridge única** que o resto do jogo usa — nunca chamar a lib de UI diretamente do gameplay:

| Ferramenta | Papel no fnxCraft | Quando usar |
|---|---|---|
| **RmlUi** (C++) | **UI do jogo** — HUD, menus, diálogos, lojas, inventário | 100% do que o jogador vê em jogo |
| **Dear ImGui** (C++) | **Editor/Debug** — F3 tools, entity inspector, profiler, world editor, spawn tools | Builds de dev (`REMOVE_IMGUI == 0`), nunca no build final |
| **Lua** (sol2/LuaJIT) | **Lógica da UI** — hooks (`on_open`, `on_click`) que alimentam a RmlUi via `data_binding` | Toda lógica de menu/HUD scriptável sem recompilar |
| **CEGUI** (C++) | **Opcional/plugável** — UI tradicional de jogos, alternativa à RmlUi | Só se o time preferir; a bridge isola a escolha |
| **Nuklear** (C) | **Opcional/plugável** — UI imediata extremamente leve para tools in-game | Painéis mínimos; desligado por default |
| **Ultralight** (HTML/CSS/JS) | **Opcional/plugável** — telas Web-like (launcher, loading, tela título animada) | Fora do processo de gameplay quando possível |
| **GLM** | Matemática — projeta UI 3D→2D (`uiWorldToScreen`) para HUDs ancorados em entidades | Base da bridge |
| **SDL3/GLFW** | Janela/Input — a bridge consome eventos do GLFW e distribui para a UI ativa | Já existe (`src/platform/glfwMain.cpp`) |
| **OpenGL** (GL 4.3 core) | Rendering — renderer de UI por lib, sem fog/atlas do mundo | Vulkan/D3D12 ficam como backend futuro |

**Regra de ouro:** gameplay nunca inclui `<RmlUi/Core.h>`, `<imgui.h>` etc. — só
`<gameLayer/ui/UiSystem.h>`. A bridge escolhe o backend em runtime e o código do jogo
fica idêntico se amanhã o backend mudar (RmlUi → CEGUI, por exemplo).

> **Nota:** já existe `include/gameLayer/HtmlUiEngine.h` (experimento web-like atual).
> Não fazer download de engine externa; a bridge substitui/augmenta esse experimento.
> O único path de "browser embutido" válido é Ultralight (offline, sem dependência de browser instalado).

## Arquitetura

```
                 ┌─────────────────────────────┐
                 │        UiSystem             │  ← ÚNICO header que o gameplay inclui
                 │  (include/gameLayer/ui/)    │
                 │  UiSystem::init()           │  escolhe backend (RmlUi default)
                 │  UiSystem::loadScreen()     │  carrega documento/árvore de UI
                 │  UiSystem::pushEvent()      │  input do GLFW → UI
                 │  UiSystem::luaExpose()      │  Lua → UI (data binding)
                 │  UiSystem::render()         │  desenha por cima do 3D (GLM p/ anchor)
                 │  UiSystem::setBackend()     │  troca RmlUi↔CEGUI↔Nuklear↔Ultralight
                 └──────────┬──────────────────┘
        ┌───────────────────┼─────────────────────┬──────────────────┐
   RmlUiRenderer      ImGuiRenderer        CEGUIRenderer     NuklearRenderer
   (HUD/menus)        (editor/debug)       (plugável)        (plugável)
        │                    │
        └── data model ──────┴── EventBus já existente ── Lua (sol2)
```

Fluxo por frame:

```
GLFW poll → UiSystem::pushEvent(win, ev) → backend ativo consome? sim → UI engole input
                                         ↘ não → gameplay (mouse/teclado normais)
3D render (mundo) → UiSystem::render(dt) → backend ativo desenha overlay 2D
```

## Arquivos (a criar/alterar)

- `include/gameLayer/ui/UiSystem.h` — facade única; enum `UiBackend { RmlUi, ImGui, CEGUI, Nuklear, Ultralight }`
- `include/gameLayer/ui/UiEvents.h` — tipos de eventos UI↔gameplay (click, hover, select, submit)
- `src/gameLayer/ui/UiSystem.cpp` — init/shutdown, troca de backend, roteamento de eventos
- `src/gameLayer/ui/UiEvents.cpp` — conversão GLFW event → evento de UI
- `src/gameLayer/ui/backends/RmlUiBackend.cpp` — RmlUi: context, `data_binding`, renderer GL
- `src/gameLayer/ui/backends/ImGuiBackend.cpp` — envolve o setup que já existe no `glfwMain.cpp` (`ImGui_Impl_glfw` + `ImGui_Impl_opengl3`), mas restrito a janelas de debug
- `src/gameLayer/ui/backends/CeguiBackend.cpp` / `NuklearBackend.cpp` / `UltralightBackend.cpp` — stubs plugáveis atrás de `#ifdef FNX_UI_CEGUI` / `FNX_UI_NUKLEAR` / `FNX_UI_ULTRALIGHT`
- `resources/ui/rml/` — `.rml` + `.rcss` (HUD, pause, inventário, fornalha, lojas)
- `resources/ui/lua/` — scripts de lógica de UI carregados em runtime
- `thirdparty/CMakeLists.txt` + root `CMakeLists.txt` — `add_subdirectory(rmlui)` etc., flags `FNX_UI_*`
- `CMakeLists.txt` — `target_link_libraries(... ui)` + options `FNX_UI_BACKEND`

## Checklist

- [ ] **Fase 0 — fundação**
  - [ ] Criar `UiSystem` (facade) + `UiEvents` (contratos)
  - [ ] Option CMake `FNX_UI_BACKEND` (default `rmlui`), options `FNX_UI_CEGUI`, `FNX_UI_NUKLEAR`, `FNX_UI_ULTRALIGHT` (todas `OFF` por default)
  - [ ] Migrar setup ImGui existente do `glfwMain.cpp` para `ImGuiBackend.cpp` **sem mudar comportamento** (dockspace, temas, ini)
- [ ] **Fase 1 — RmlUi como UI do jogo**
  - [ ] Vendor RmlUi (5.1+) em `thirdparty/rmlui` (submódulo ou vendored)
  - SpA — Renderer OpenGL 4.3 core compatível com o pipeline atual (mesmo VAO/shader estilo do renderer)
  - [ ] `RmlUiBackend`: `Rml::Context`, carregamento de `.rml/.rcss`, system interface (tempo/arquivos)
  - [ ] Portar HUD atual (vida/fome/sede) e menu pause (`resources/ui/pause/`) para `.rml`
  - [ ] Hook no `EventBus` já existente: eventos de gameplay → variáveis de UI (ex.: `player_health`)
- [ ] **Fase 2 — Lua como lógica da UI**
  - [ ] Vendor sol2 (header-only) + Lua 5.4 em `thirdparty/`
  - [ ] `UiSystem::luaExpose()` — API: `ui.open("pause")`, `ui.set("player_health", 18)`, `ui.on("btn_resume", fn)`
  - `scripts/ui.lua` + hot-reload de scripts de UI (F5 no editor ImGui)
  - [ ] Migrar lógica do pause/inventário para Lua
- [ ] **Fase 3 — Editor/Debug com ImGui**
  - [ ] Entity inspector (pos, vida, IA state, vel), profiler FPS/frame-time, world editor
  - [ ] Console de comandos ligado ao `commandSystem.cpp` existente
  - [ ] Debug de rede: estado do ENet, ping, entidades syncadas
- [ ] **Fase 4 — Backends plugáveis (opcional)**
  - [ ] `CeguiBackend` / `NuklearBackend`: implementar só se houver necessidade real
  - [ ] `UltralightBackend`: telas fora do gameplay (launcher/título); exige binários do Ultralight (não é código aberto completo — usar só se aceitar o licensing)
  - [ ] Teste de troca de backend em runtime (`ui.set_backend("nuklear")` via console)
- [ ] **Fase 5 — corte final**
  - [ ] Build release: `REMOVE_IMGUI == 1` remove ImGui + editor; gameplay só depende da bridge
  - shippable build: só RmlUi + Lua no binário final (menor footprint)
  - [ ] Remover/absorver `HtmlUiEngine` antigo quando a RmlUi cobrir os mesmos casos

## Bridge — exemplo da API única (contrato)

```cpp
// include/gameLayer/ui/UiSystem.h — ÚNICO include permitido no gameplay
namespace ui
{
    enum class Backend { RmlUi, ImGui, CEGUI, Nuklear, Ultralight };

    bool init(glfwWindow*, UiRendererGL* gl, Backend b = Backend::RmlUi);
    void shutdown();

    bool loadScreen(const char* screen /* "pause", "hud", "inventory" */);
    void closeScreen(const char* screen);

    void pushEvent(const WindowEvent& ev);      // GLFW → UI; retorna "consumido"
    bool wantsKeyboard(); bool wantsMouse();    // gameplay pergunta antes de mover câmera

    void setVar(const char* name, const UiValue& v);   // gameplay → UI
    void onEvent(const char* id, UiCallback cb);       // UI → gameplay

    void render(float dt);                       // depois do 3D, antes do swap

    // HUD ancorado em mundo (usa GLM p/ projetar pos 3D → tela)
    void setWorldAnchor(const char* id, glm::vec3 worldPos);
}
```

```cpp
// uso no gameplay — idêntico em qualquer backend
ui::loadScreen("hud");
ui::onEvent("btn_inventory", [] { openInventory(); });
ui::setVar("player_health", player.health);
ui::render(dt);
```

## Como Testar

Comando: `cmake --build build -j4 && ./game/fnxCraft`
- [ ] Jogo abre com HUD RmlUi; pause (`ESC`) abre/fecha; input não vaza p/ gameplay quando UI ativa
- [ ] `~` abre console ImGui (só em dev); inspetor de entidades mostra dados reais
- [ ] `scripts/ui.lua` muda um texto do HUD **sem recompilar** (hot-reload)
- [ ] Trocar `FNX_UI_BACKEND=cegui/nuklear` compila e roda a mesma tela de pause
- [ ] Build release (`REMOVE_IMGUI=1`) compila sem nenhum símbolo de ImGui

## Prioridade

🔴 Avançado — múltiplas libs de UI + bridge + Lua; grande refactor de input/render, mas destrava todas as tasks de UI (fornalha #07, lojas, HUD #03).
