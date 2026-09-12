#pragma once
/*
	HtmlUiEngine — backend HTML/CSS/JS da UiBridge.

	Cada tela vive em resources/ui/<nome>/ com index.html + style.css + script.js.
	O script.js fala com o C++ por window.htmlUi.<funcao>(json) — cada funcao e
	registrada no C++ via bind("funcao", callback), recebendo o JSON de args.

	Implementação atual: WebGL2 (composited) + fetch dos arquivos + <script> do
	componente avaliado em contexto isolado. Render em textura desenhada como
	quad GL por cima do jogo (gl2d).
*/
#include <string>
#include <functional>
struct GLFWwindow;

struct HtmlUiEngine
{
	bool init(GLFWwindow *win);
	void shutdown();
	void update(float dt);
	void render();
	void onResize(int w, int h);
	bool onKey(int key, int scancode, int action, int mods);
	bool onChar(unsigned int c);
	bool onMouse(int button, int action, int mods, double x, double y);
	void loadUrl(const std::string &url);
	void loadComponent(const std::string &name); // resources/ui/<name>/index.html
	void bind(const std::string &name, std::function<void(std::string)> cb);
	void callJS(const std::string &code);
	bool wantCaptureMouse() const { return captureMouse; }
	bool wantCaptureKeyboard() const { return captureKeyboard; }
	bool useHtml = true;
	bool toggle() { useHtml = !useHtml; return useHtml; }
	bool isHtml() const { return useHtml; }

private:
	bool captureMouse = false, captureKeyboard = false;
	void *view = nullptr;   // reservado p/ backend nativo futuro (Ultralight/WebView)
	void *ctx = nullptr;    // reservado
};
extern HtmlUiEngine gHtmlUi;
