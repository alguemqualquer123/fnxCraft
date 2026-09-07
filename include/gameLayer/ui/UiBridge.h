#pragma once
/*
	UiBridge — fachada ÚNICA de UI do fnxCraft.

	O gameplay NUNCA inclui HtmlUiEngine/imgui/glui diretamente; sempre esta bridge.

	Backends:
	- Html    : HTML/CSS/JS (HtmlUiEngine — recursos em resources/ui/<nome>/index.html,
	            temas em themes/*.css, lógica em script.js via window.htmlUi). Estilo Ultralight.
	- ImGui   : Dear ImGui — janelas de debug/editor (F12 alterna). Compilado fora em release
	            (REMOVE_IMGUI == 1).
	- Nuklear / CEGUI / RmlUi : backends plugáveis (stubs) — ver plans/20-ui-stack.md.

	Fluxo por frame (já ligado no glfwMain.cpp):
		GLFW callbacks -> ui::onKey/onChar/onMouseButton (retornam true se a UI consumiu)
		gameLogic      -> ui::update(dt)
		antes do swap  -> ui::render()

	Gameplay -> UI:
		ui::setState("health", 18.5f)   vira htmlUi.setState("health", 18.5) no JS
		ui::callUi("refreshWorlds", "[{...}]")  chama htmlUi.<funcao>(<json>) no JS
		ui::pushLog("msg")              feed visível na janela de debug ImGui

	UI -> Gameplay:
		ui::bind("createWorld", [](const std::string &jsonArgs){ ... });
		O JS chama htmlUi.createWorld(...) e a bridge entrega o JSON aqui.

	Plug-in de um novo backend (Nuklear/CEGUI/RmlUi):
		1) crie src/gameLayer/ui/backends/<Nome>Backend.cpp
		2) trate o caso nos switch() de UiBridge.cpp
		3) ligue a lib no CMake atrás de um option FNX_UI_<NOME>
*/
#include <string>
#include <functional>

struct GLFWwindow;

namespace ui
{
	enum class Backend
	{
		Html,    // HTML/CSS/JS (HtmlUiEngine)
		ImGui,   // Dear ImGui (debug/editor)
		Nuklear, // plugável
		CEGUI,   // plugável
		RmlUi,   // plugável
	};

	using UiCallback = std::function<void(const std::string &jsonArgs)>;

	// ciclo de vida
	bool init(GLFWwindow *win);
	void shutdown();

	// troca de backend em runtime (F12 alterna Html <-> ImGui)
	bool reconfigure(Backend b);
	void toggleBackend();
	Backend getBackend();
	const char *toString(Backend b);

	// por frame
	void update(float dt);
	void render();

	// input (retornam true se a UI consumiu o evento -> gameplay não reage)
	bool onKey(int key, int scancode, int action, int mods);
	bool onChar(unsigned int c);
	bool onMouseButton(int button, int action, int mods, double x, double y);

	bool wantCaptureMouse();
	bool wantCaptureKeyboard();

	// gameplay -> UI
	void setState(const std::string &key, float v);
	void setState(const std::string &key, const std::string &v);
	void callUi(const std::string &func, const std::string &jsonArgs = "");
	void pushLog(const std::string &msg); // feed na janela de debug ImGui

	// UI -> gameplay
	void bind(const std::string &name, UiCallback cb);
}
