#pragma once
#include <string>
#include <glad/glad.h>

struct GLFWwindow;

namespace Performance
{
	enum class VSyncMode { Off = 0, On = 1, Adaptive = 2 };
	enum class MSAA { Off = 0, X2 = 2, X4 = 4, X8 = 8 };
	enum class FSRMode { Off = 0, Quality = 1, Balanced = 2, Performance = 3, Ultra = 4 };

	void init(GLFWwindow *window);
	void applyVSync(VSyncMode mode);
	void applyMSAA(MSAA msaa);
	void applyAnisotropy(int level);
	void setFrameGeneration(bool enabled, int mode = 0);
	void setNvidiaBoost(bool enabled);
	void setAmdAntiLag(bool enabled);
	void beginFrame();
	void endFrame();
	bool isFrameGenerationEnabled();
	void tickFrameGeneration(float deltaTime);
	void renderFSR(FSRMode mode, int srcW, int srcH, int dstW, int dstH);
	std::string getGpuVendor();
	void autoOptimize();
	std::string getPerformanceReport();
}
