#include <rendering/performance.h>
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <algorithm>
#include <iostream>
#include <string>

//NOTE: DLSS/FSR/reflex-style features (frame generation, nvidia boost, AMD
//anti-lag, FSR upscaling) are not wired up yet - the state is tracked so the
//UI/settings code can keep working, but applying them is a TODO for when the
//vendor SDKs are integrated.

namespace Performance
{
	namespace
	{
		GLFWwindow *s_window = nullptr;
		bool s_glReady = false;

		VSyncMode s_vsync = VSyncMode::On;
		int s_msaaSamples = 0;
		int s_anisotropy = 0;

		bool s_frameGeneration = false;
		int s_frameGenerationMode = 0;
		bool s_nvidiaBoost = false;
		bool s_amdAntiLag = false;

		std::string s_vendor = "Unknown GPU";
	}

	void init(GLFWwindow *window)
	{
		s_window = window;
		if (!s_window) return;
		glfwMakeContextCurrent(s_window);

		const char *renderer = (const char *)glGetString(GL_RENDERER);
		const char *vendor = (const char *)glGetString(GL_VENDOR);
		std::string v = vendor ? vendor : "";
		std::string r = renderer ? renderer : "";
		s_vendor = v + (v.empty() || r.empty() ? "" : " - ") + r;
		s_glReady = true;

		std::cout << "[performance] GPU: " << s_vendor << "\n";
	}

	void applyVSync(VSyncMode mode)
	{
		s_vsync = mode;
		if (!s_window) return;

		int interval = 0;
		if (mode == VSyncMode::On)
		{
			interval = 1;
		}
		else if (mode == VSyncMode::Adaptive)
		{
			//adaptive vsync (-1) where supported, otherwise GLFW keeps it as-is
			interval = -1;
		}
		glfwSwapInterval(interval);
	}

	void applyMSAA(MSAA msaa)
	{
		s_msaaSamples = (int)msaa;
		if (!s_glReady) return;

		//the default framebuffer's sample count is decided when the window is
		//created (glfwWindowHint GLFW_SAMPLES); toggling at runtime only affects
		//the multisample enable state.
		if (s_msaaSamples > 0)
		{
			glEnable(GL_MULTISAMPLE);
		}
		else
		{
			glDisable(GL_MULTISAMPLE);
		}
	}

	void applyAnisotropy(int level)
	{
		s_anisotropy = level;
		if (!s_glReady) return;

		float maxSupported = 1.f;
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxSupported);
		if (level <= 0) return;

		//TODO: apply to textures as they load, this only clamps/stores the value
		s_anisotropy = (int)std::min((float)level, maxSupported);
	}

	void setFrameGeneration(bool enabled, int mode)
	{
		s_frameGeneration = enabled;
		s_frameGenerationMode = mode;
	}

	void setNvidiaBoost(bool enabled)
	{
		s_nvidiaBoost = enabled;
		//TODO: NVAPI boost clock when the SDK is integrated
	}

	void setAmdAntiLag(bool enabled)
	{
		s_amdAntiLag = enabled;
		//TODO: ADL anti-lag when the SDK is integrated
	}

	void beginFrame()
	{
		//TODO: hook for frame generation / latency markers
	}

	void endFrame()
	{
		//TODO: hook for frame generation / latency markers
	}

	bool isFrameGenerationEnabled()
	{
		return s_frameGeneration;
	}

	void tickFrameGeneration(float)
	{
		//TODO: drive frame generation interpolation when implemented
	}

	void renderFSR(FSRMode, int, int, int, int)
	{
		//TODO: FSR upscaling passes when implemented
	}

	std::string getGpuVendor()
	{
		return s_vendor;
	}

	void autoOptimize()
	{
		if (!s_glReady) return;
		//keep it simple: only adjust settings we can actually apply right now.
		if (s_msaaSamples == 0) applyMSAA(MSAA::X4);
		if (s_anisotropy < 4) applyAnisotropy(4);
		applyVSync(VSyncMode::On);
	}

	std::string getPerformanceReport()
	{
		std::string r = "GPU: " + s_vendor + "\n";
		r += "VSync: ";
		switch (s_vsync)
		{
		case VSyncMode::Off: r += "Off"; break;
		case VSyncMode::On: r += "On"; break;
		case VSyncMode::Adaptive: r += "Adaptive"; break;
		}
		r += " | MSAA: " + std::to_string(s_msaaSamples);
		r += " | Anisotropy: " + std::to_string(s_anisotropy);
		r += " | FrameGen: " + std::string(s_frameGeneration ? "On" : "Off");
		r += " | NvidiaBoost: " + std::string(s_nvidiaBoost ? "On" : "Off");
		r += " | AntiLag: " + std::string(s_amdAntiLag ? "On" : "Off");
		return r;
	}
}
