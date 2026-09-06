#pragma once
#include <string>
#include <functional>

struct GLFWwindow;

class SplashScreen
{
public:
	static void init(GLFWwindow *window);
	static void draw(float progress, const std::string &module, const std::string &detail = "");
	static void shutdown();
	static bool isActive();

private:
	static GLFWwindow *s_window;
	static float s_progress;
	static std::string s_module;
	static std::string s_detail;
};
