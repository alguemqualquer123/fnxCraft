#include "gameLayer/ui/UiBridge.h"
#include "HtmlUiEngine.h"
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <imguiThemes.h>
#include <platformTools.h> // REMOVE_IMGUI
#include <iostream>
#include <map>
#include <mutex>
#include <sstream>
#include <vector>

// definidos no bloco de estado abaixo
namespace
{
	std::vector<std::string> getBindingNames();
	std::string getLogText();
}

// ============================================================================
// Backend ImGui (debug/editor) — contexto criado aqui, não no glfwMain.
// ============================================================================

namespace
{
	bool imguiInitialized = false;

	void imguiInit(GLFWwindow *win)
	{
		if (imguiInitialized) return;
		ImGui::CreateContext();
		imguiThemes::embraceTheDarkness();
		ImGuiIO &io = ImGui::GetIO(); (void)io;
		io.ConfigFlags = ImGuiConfigFlags_NavEnableKeyboard
			| ImGuiConfigFlags_DockingEnable
			| ImGuiConfigFlags_ViewportsEnable;
		ImGui_ImplGlfw_InitForOpenGL(win, true);
		ImGui_ImplOpenGL3_Init("#version 330");
		imguiInitialized = true;
		std::cout << "[UiBridge] ImGui backend inicializado (debug/editor)\n";
	}

	void imguiShutdown()
	{
		if (!imguiInitialized) return;
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
		imguiInitialized = false;
	}

	void imguiNewFrame()
	{
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
	}

	void imguiRender()
	{
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		ImGuiIO &io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			GLFWwindow *backup = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(backup);
		}
	}

	// Janela de debug/editor: estado da bridge, bindings registrados e log de eventos.
	void drawDebugWindows()
	{
		if (ImGui::Begin("UI Debug (UiBridge)"))
		{
			ImGui::Text("Backend ativo: %s", ui::toString(ui::getBackend()));
			ImGui::Separator();

			if (ImGui::CollapsingHeader("Bindings UI -> Gameplay"))
			{
				for (auto &name : getBindingNames())
				{
					ImGui::BulletText("%s", name.c_str());
				}
			}

			if (ImGui::CollapsingHeader("Log de eventos", ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::TextUnformatted(getLogText().c_str());
			}
		}
		ImGui::End();
	}
}

// ============================================================================
// Estado da bridge
// ============================================================================

namespace
{
	GLFWwindow *s_window = nullptr;
	ui::Backend s_backend = ui::Backend::Html;
	bool s_initialized = false;

	std::mutex s_bindingsMutex;
	std::map<std::string, ui::UiCallback> s_bindings;

	std::mutex s_logMutex;
	std::ostringstream s_logStream;
	constexpr size_t MAX_LOG_CHARS = 16 * 1024;

	void dispatchToGameplay(const std::string &name, const std::string &jsonArgs)
	{
		ui::UiCallback cb;
		{
			std::lock_guard<std::mutex> lock(s_bindingsMutex);
			auto it = s_bindings.find(name);
			if (it == s_bindings.end())
			{
				static std::map<std::string, bool> warnedOnce;
				if (!warnedOnce[name])
				{
					std::cout << "[UiBridge] bind nao encontrado: '" << name << "'\n";
					warnedOnce[name] = true;
				}
				return;
			}
			cb = it->second;
		}
		cb(jsonArgs);
	}

	std::vector<std::string> getBindingNames()
	{
		std::lock_guard<std::mutex> lock(s_bindingsMutex);
		std::vector<std::string> names;
		for (auto &p : s_bindings) names.push_back(p.first);
		return names;
	}

	std::string getLogText()
	{
		std::lock_guard<std::mutex> lock(s_logMutex);
		return s_logStream.str();
	}
}

// ============================================================================
// API pública
// ============================================================================

bool ui::init(GLFWwindow *win)
{
	if (s_initialized) return true;
	s_window = win;

	gHtmlUi.init(win); // backend HTML/CSS/JS (resources/ui/)

#if REMOVE_IMGUI == 0
	imguiInit(win);    // backend ImGui (debug/editor)
#endif

	s_initialized = true;
	std::cout << "[UiBridge] init ok (backend ativo: " << toString(s_backend) << ")\n";
	return true;
}

void ui::shutdown()
{
	if (!s_initialized) return;
	gHtmlUi.shutdown();
#if REMOVE_IMGUI == 0
	imguiShutdown();
#endif
	s_initialized = false;
}

bool ui::reconfigure(Backend b)
{
	s_backend = b;
	std::cout << "[UiBridge] backend -> " << toString(b) << "\n";
	return true;
}

void ui::toggleBackend()
{
	Backend b = (s_backend == Backend::Html) ? Backend::ImGui : Backend::Html;
	reconfigure(b);
}

ui::Backend ui::getBackend() { return s_backend; }

const char *ui::toString(Backend b)
{
	switch (b)
	{
	case Backend::Html:    return "Html (HTML/CSS/JS)";
	case Backend::ImGui:   return "ImGui (debug/editor)";
	case Backend::Nuklear: return "Nuklear (plugavel)";
	case Backend::CEGUI:   return "CEGUI (plugavel)";
	case Backend::RmlUi:   return "RmlUi (plugavel)";
	}
	return "?";
}

void ui::update(float dt)
{
	if (s_backend == Backend::Html) gHtmlUi.update(dt);
}

void ui::render()
{
	switch (s_backend)
	{
	case Backend::Html:
		gHtmlUi.render();
		break;

	case Backend::ImGui:
	{
#if REMOVE_IMGUI == 0
		imguiNewFrame();
		drawDebugWindows();
		imguiRender();
#endif
		break;
	}

	default: break; // backends plugaveis (Nuklear/CEGUI/RmlUi)
	}
}

bool ui::onKey(int key, int scancode, int action, int mods)
{
	if (s_backend == Backend::Html)
		return gHtmlUi.onKey(key, scancode, action, mods);
	return false;
}

bool ui::onChar(unsigned int c)
{
	if (s_backend == Backend::Html)
		return gHtmlUi.onChar(c);
	return false;
}

bool ui::onMouseButton(int button, int action, int mods, double x, double y)
{
	if (s_backend == Backend::Html)
		return gHtmlUi.onMouse(button, action, mods, x, y);
	return false;
}

bool ui::wantCaptureMouse()
{
	if (s_backend == Backend::Html) return gHtmlUi.wantCaptureMouse();
	if (s_backend == Backend::ImGui)
	{
#if REMOVE_IMGUI == 0
		return ImGui::GetIO().WantCaptureMouse;
#endif
	}
	return false;
}

bool ui::wantCaptureKeyboard()
{
	if (s_backend == Backend::Html) return gHtmlUi.wantCaptureKeyboard();
	if (s_backend == Backend::ImGui)
	{
#if REMOVE_IMGUI == 0
		return ImGui::GetIO().WantCaptureKeyboard;
#endif
	}
	return false;
}

void ui::setState(const std::string &key, float v)
{
	if (s_backend == Backend::Html)
		gHtmlUi.callJS(std::string("htmlUi.setState && htmlUi.setState('") + key + "'," +
			std::to_string(v) + ")");
}

void ui::setState(const std::string &key, const std::string &v)
{
	if (s_backend == Backend::Html)
		gHtmlUi.callJS(std::string("htmlUi.setState && htmlUi.setState('") + key + "','" + v + "')");
}

void ui::callUi(const std::string &func, const std::string &jsonArgs)
{
	if (s_backend == Backend::Html)
		gHtmlUi.callJS(std::string("htmlUi.") + func + "(" + jsonArgs + ");");
}

void ui::pushLog(const std::string &msg)
{
	std::lock_guard<std::mutex> lock(s_logMutex);
	s_logStream << msg << "\n";
	if (s_logStream.str().size() > MAX_LOG_CHARS)
	{
		// mantem apenas a cauda do log
		std::string tail = s_logStream.str();
		tail = tail.substr(tail.size() - MAX_LOG_CHARS / 2);
		s_logStream.str("");
		s_logStream << tail;
	}
}

void ui::bind(const std::string &name, UiCallback cb)
{
	{
		std::lock_guard<std::mutex> lock(s_bindingsMutex);
		s_bindings[name] = std::move(cb);
	}
	// repassa para o backend HTML: quando o engine real chegar (RmlUi/Ultralight),
	// o evento chega aqui e é roteado por dispatchToGameplay.
	gHtmlUi.bind(name, [name](const std::string &jsonArgs)
	{
		dispatchToGameplay(name, jsonArgs);
	});
}
