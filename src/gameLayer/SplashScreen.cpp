#include <glad/glad.h>
#include <gameLayer/SplashScreen.h>
#include <GLFW/glfw3.h>

#if REMOVE_IMGUI == 0
#include "imgui.h"
#include "imgui_internal.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#endif

GLFWwindow *SplashScreen::s_window = nullptr;
float SplashScreen::s_progress = 0.f;
std::string SplashScreen::s_module = "";
std::string SplashScreen::s_detail = "";

void SplashScreen::init(GLFWwindow *window)
{
	s_window = window;
	s_progress = 0.f;
	s_module = "Iniciando...";
}

void SplashScreen::draw(float progress, const std::string &module, const std::string &detail)
{
	if (!s_window) return;
	if (glfwWindowShouldClose(s_window)) return;
	s_progress = progress;
	if (!module.empty()) s_module = module;
	s_detail = detail;

	int w, h;
	glfwGetFramebufferSize(s_window, &w, &h);
	if (w <= 0 || h <= 0) return;
	glViewport(0, 0, w, h);
	glClearColor(0.07f, 0.07f, 0.09f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT);

#if REMOVE_IMGUI == 0
	if (!ImGui::GetCurrentContext()) { glfwSwapBuffers(s_window); glfwPollEvents(); return; }
	ImGuiIO &io = ImGui::GetIO();
	if (io.BackendPlatformUserData == nullptr) { glfwSwapBuffers(s_window); glfwPollEvents(); return; }

	bool withinFrame = ImGui::GetCurrentContext()->WithinFrameScope;

	if (!withinFrame)
	{
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}

	ImGui::SetNextWindowPos(ImVec2(0, 0));
	ImGui::SetNextWindowSize(ImVec2((float)w, (float)h));
	ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove
		| ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoBringToFrontOnFocus;

	ImGui::Begin("Splash", nullptr, flags);

	float centerX = w * 0.5f;
	float centerY = h * 0.5f;

	ImGui::SetCursorPosY(centerY - 120);
	ImGui::SetCursorPosX(centerX - 170);
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.95f, 0.95f, 1.f, 1.f));
	ImGui::SetWindowFontScale(2.8f);
	ImGui::Text("fnxCraft");
	ImGui::PopStyleColor();
	ImGui::SetWindowFontScale(1.0f);

	ImGui::SetCursorPosX(centerX - 90);
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.65f, 0.65f, 0.75f, 1.f));
	ImGui::Text("Carregando mundo infinito...");
	ImGui::PopStyleColor();

	float barW = w * 0.55f;
	float barH = 18.f;
	float barX = centerX - barW * 0.5f;
	float barY = centerY + 30;

	ImVec2 p0(barX, barY);
	ImVec2 p1(barX + barW, barY + barH);
	ImDrawList *dl = ImGui::GetWindowDrawList();
	dl->AddRectFilled(p0, p1, IM_COL32(30, 32, 40, 255), 9.f);
	dl->AddRect(p0, p1, IM_COL32(60, 62, 70, 255), 9.f, 0, 1.5f);

	float fillW = barW * s_progress;
	if (fillW > 0)
	{
		ImVec2 f1(barX + fillW, barY + barH);
		dl->AddRectFilled(p0, f1, IM_COL32(120, 110, 245, 255), 9.f);
	}

	int pct = int(s_progress * 100.f);
	std::string pctStr = std::to_string(pct) + "%";
	ImGui::SetCursorPosY(barY + 28);
	ImGui::SetCursorPosX(centerX - ImGui::CalcTextSize(pctStr.c_str()).x * 0.5f);
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.85f, 0.85f, 0.9f, 1.f));
	ImGui::Text("%s", pctStr.c_str());
	ImGui::PopStyleColor();

	ImGui::SetCursorPosY(barY + 50);
	ImGui::SetCursorPosX(centerX - ImGui::CalcTextSize(s_module.c_str()).x * 0.5f);
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.78f, 0.78f, 0.85f, 1.f));
	ImGui::Text("%s", s_module.c_str());
	ImGui::PopStyleColor();

	if (!s_detail.empty())
	{
		ImGui::SetCursorPosX(centerX - ImGui::CalcTextSize(s_detail.c_str()).x * 0.5f);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.55f, 0.55f, 0.62f, 1.f));
		ImGui::Text("%s", s_detail.c_str());
		ImGui::PopStyleColor();
	}

	ImGui::SetCursorPosY(h - 28);
	ImGui::SetCursorPosX(w - 210);
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.45f, 0.45f, 0.52f, 1.f));
	ImGui::Text("v0.1.0  •  Preparando aventura");
	ImGui::PopStyleColor();

	ImGui::End();

	if (!withinFrame)
	{
		ImGui::Render();
		int display_w, display_h;
		glfwGetFramebufferSize(s_window, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			GLFWwindow* backup = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(backup);
		}

		glfwSwapBuffers(s_window);
		glfwPollEvents();
	}
#endif
}

void SplashScreen::shutdown()
{
	s_progress = 1.f;
}

bool SplashScreen::isActive()
{
	return s_window != nullptr && s_progress < 1.f;
}
