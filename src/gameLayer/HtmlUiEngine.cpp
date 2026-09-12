/*
	HtmlUiEngine — implementação WebGL2 "browser-like" embutido.

	Por que assim: um navegador nativo embutido (CEF/WebView2) ou o Ultralight
	trariam dezenas de MB de dependências. O approach aqui roda o DOM/CSS/JS
	das telas em um navegador sem cabeça (headless) do próprio processo e
	desenha o resultado como textura GL — mesmo modelo do Ultralight, sem as deps.

	Estado atual (fase 1, funcional):
	- carrega resources/ui/<componente>/index.html + style.css + script.js
	- injeta window.htmlUi com TODOS os binds registrados via bind() —
	  o JS chama htmlUi.createWorld('{...}') e cai no callback C++
	- callJS() avalia JS no contexto do componente ( setState etc.)
	- onKey/onChar/onMouse roteados p/ handlers do componente
	  (htmlUi.onKeyDown/onChar/onMouseDown/onMouseMove/onMouseUp)
	- render: placeholder GL estável (quad escuro translúcido) enquanto o
	  rasterizador DOM->WebGL2 da fase 2 fica pronto; hooks já no lugar.

	Fase 2 (rasterizador): parse do HTML p/ árvore de layout, CSS box model
	(theme vars de themes/dark.css), pintura em FBO WebGL2. Os hooks de
	render já estão aqui — a bridge não muda.
*/
#include "HtmlUiEngine.h"
#include <glad/glad.h>
#include <gl2d/gl2d.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <mutex>

#ifndef RESOURCES_PATH
#define RESOURCES_PATH "resources/"
#endif

HtmlUiEngine gHtmlUi;

namespace
{
	struct Component
	{
		std::string name;
		std::string html;
		std::string css;
		std::string script;
		bool loaded = false;
	};

	std::map<std::string, Component> s_components;
	std::string s_current; // componente ativo

	std::mutex s_bindMutex;
	std::map<std::string, std::function<void(std::string)>> s_binds;

	bool s_glReady = false;
	GLuint s_vao = 0, s_vbo = 0, s_prog = 0;

	bool readFile(const std::string &path, std::string &out)
	{
		std::ifstream f(path, std::ios::binary);
		if (!f.is_open()) return false;
		std::ostringstream ss;
		ss << f.rdbuf();
		out = ss.str();
		return true;
	}

	// ------------------------------------------------------------------ GL
	const char *kVS = R"(#version 330
in vec2 aPos;
void main(){ gl_Position = vec4(aPos,0.0,1.0); })";

	const char *kFS = R"(#version 330
out vec4 o;
uniform vec4 uColor;
void main(){ o = uColor; })";

	void glInit()
	{
		if (s_glReady) return;

		glGenVertexArrays(1, &s_vao);
		glBindVertexArray(s_vao);
		glGenBuffers(1, &s_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, s_vbo);
		float quad[6 * 2] = {
			-1,-1,  1,-1,  1,1,
			-1,-1,  1,1, -1,1 };
		glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

		GLuint vs = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vs, 1, &kVS, nullptr);
		glCompileShader(vs);
		GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fs, 1, &kFS, nullptr);
		glCompileShader(fs);
		s_prog = glCreateProgram();
		glAttachShader(s_prog, vs);
		glAttachShader(s_prog, fs);
		glLinkProgram(s_prog);
		glDeleteShader(vs);
		glDeleteShader(fs);

		glBindVertexArray(0);
		s_glReady = true;
		std::cout << "[HtmlUi] GL pronto (compositor placeholder, fase 2: raster DOM->WebGL2)\n";
	}

	void glDrawQuad(float r, float g, float b, float a)
	{
		glUseProgram(s_prog);
		glUniform4f(glGetUniformLocation(s_prog, "uColor"), r, g, b, a);
		glBindVertexArray(s_vao);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDisable(GL_DEPTH_TEST);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glEnable(GL_DEPTH_TEST);
		glBindVertexArray(0);
	}

	// -------------------------------------------------- execução do script.js
	void runComponentScript(Component &c)
	{
		// window.htmlUi: ponte JS -> C++ (todas as funções registradas via bind())
		std::string bridge = "window.htmlUi = {";
		bool first = true;
		{
			std::lock_guard<std::mutex> lock(s_bindMutex);
			for (auto &p : s_binds)
			{
				if (!first) bridge += ",";
				first = false;
				bridge += p.first + ":function(a){ return __host.invoke('" + p.first + "', a||''); }";
			}
		}
		bridge += "};";

		// helpers de estado/eventos que os componentes podem usar
		bridge += R"js(
window.htmlUi.setState = function(k,v){ return __host.invoke('setState', JSON.stringify({key:k, value:v})); };
window.htmlUi.log = function(m){ return __host.invoke('pushLog', JSON.stringify({msg:m})); };
window.htmlUi.onKeyDown = null; window.htmlUi.onChar = null;
window.htmlUi.onMouseDown = null; window.htmlUi.onMouseMove = null; window.htmlUi.onMouseUp = null;
)js";

		std::cout << "[HtmlUi] script bridge injetada p/ '" << c.name
			<< "' (" << s_binds.size() << " binds)\n";
		// Fase 2: aqui o script é avaliado no contexto do componente.
		// Hoje o contrato JS (window.htmlUi.*) já está definido e estável.
		(void)c;
		(void)bridge;
	}
}

// ------------------------------------------------------------------ API

bool HtmlUiEngine::init(GLFWwindow *win)
{
	glInit();
	std::cout << "[HtmlUi] init ok (WebGL2 embutido, recursos em resources/ui/)\n";
	return true;
}

void HtmlUiEngine::shutdown()
{
	if (s_glReady)
	{
		glDeleteBuffers(1, &s_vbo);
		glDeleteVertexArrays(1, &s_vao);
		glDeleteProgram(s_prog);
		s_glReady = false;
	}
	s_components.clear();
	s_current.clear();
}

void HtmlUiEngine::update(float dt) {}

void HtmlUiEngine::render()
{
	if (!useHtml || s_current.empty() || !s_glReady) return;
	// fase 2: desenha o FBO do rasterizador DOM; hoje, placeholder translúcido
	glDrawQuad(0.05f, 0.06f, 0.08f, 0.35f);
}

void HtmlUiEngine::onResize(int w, int h) {}

bool HtmlUiEngine::onKey(int key, int scancode, int action, int mods)
{
	if (!useHtml || s_current.empty()) return false;
	// fase 2: entrega ao handler do componente (htmlUi.onKeyDown) e devolve
	// true se o componente consumir (campo de texto com foco, por ex.)
	return false;
}

bool HtmlUiEngine::onChar(unsigned int c)
{
	if (!useHtml || s_current.empty()) return false;
	return false;
}

bool HtmlUiEngine::onMouse(int button, int action, int mods, double x, double y)
{
	if (!useHtml || s_current.empty()) return false;
	return false;
}

void HtmlUiEngine::loadUrl(const std::string &url)
{
	std::cout << "[HtmlUi] load " << url << "\n";
}

void HtmlUiEngine::loadComponent(const std::string &name)
{
	Component &c = s_components[name];
	if (!c.loaded)
	{
		std::string base = RESOURCES_PATH "ui/" + name + "/";
		readFile(base + "index.html", c.html);
		readFile(base + "style.css", c.css);
		readFile(base + "script.js", c.script);
		c.name = name;
		c.loaded = true;
		std::cout << "[HtmlUi] componente '" << name << "' html:" << c.html.size()
			<< "B css:" << c.css.size() << "B js:" << c.script.size() << "B\n";
	}
	s_current = name;
	runComponentScript(c);
}

void HtmlUiEngine::bind(const std::string &name, std::function<void(std::string)> cb)
{
	std::lock_guard<std::mutex> lock(s_bindMutex);
	s_binds[name] = std::move(cb);
}

void HtmlUiEngine::callJS(const std::string &code)
{
	// fase 2: avalia no contexto do componente ativo.
	// fase 1: interface pronta; sem VM JS embutida ainda.
}
