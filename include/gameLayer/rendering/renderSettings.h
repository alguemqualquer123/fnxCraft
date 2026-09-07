#pragma once

#include "glui/glui.h"
#include <filesystem>

struct ProgramData;


void displayWorldSelectorMenuButton(ProgramData &programData);

void displayWorldSelectorMenu(ProgramData &programData);

void displayRenderSettingsMenuButton(ProgramData &programData);
void displayRenderSettingsMenu(ProgramData &programData);

void displaySettingsMenuButton(ProgramData &programData);
void displaySettingsMenu(ProgramData &programData);

bool shouldReloadTexturePacks();

std::vector<std::filesystem::path> getUsedTexturePacksAndResetFlag();

void displayTexturePacksSettingsMenuButton(ProgramData &programData);

void displayTexturePacksSettingsMenu(ProgramData &programData);

void displaySkinSelectorMenu(ProgramData &programData);

void displaySkinSelectorMenuButton(ProgramData &programData);

void displayVolumeMenu(ProgramData &programData);

void displayVolumeMenuButton(ProgramData &programData);

void displayLanguageMenuButton(ProgramData &programData);

void displayWorldConfigMenu(ProgramData &programData, std::string &selectedWorld);
void displayWorldConfigMenuButton(ProgramData &programData, std::string &selectedWorld);

void displayPlayerRolesMenu(ProgramData &programData);
void displayPlayerRolesMenuButton(ProgramData &programData);

void displayWorldSettingsMenu(ProgramData &programData);
void displayWorldSettingsMenuButton(ProgramData &programData);


std::string getSkinName();

struct ShadingSettings
{

	int viewDistance = 8;
	int tonemapper = 0;
	int shadows = 0;
	int waterType = 1;
	int workerThreadsForBaking = 4;
	int lodStrength = 1;
	int PBR = 1;
	int maxLights = 20;
	int useLights = 1;
	float lightsStrength = 1.f;
	bool FXAA = 1;

	int frameGeneration = 0;
	int frameGenerationMode = 0;
	int nvidiaBoost = 0;
	int msaa = 0;
	int vsyncMode = 1;
	int fsr = 0;
	int amdAntiLag = 0;
	int anisotropy = 4;

	glm::vec3 waterColor = (glm::vec3(6, 42, 52) / 255.f);
	glm::vec3 underWaterColor = glm::vec3(0, 17, 25) / 255.f;

	float underwaterDarkenStrength = 0.94;
	float underwaterDarkenDistance = 29;
	float fogGradientUnderWater = 1.9;
	
	float bloomTresshold = 0.5;
	float bloomMultiplier = 0.5;

	float exposure = 0;
	float fogGradient = 16.f;
	int bloom = 1; // disabled - causes black screen, re-enable after skybox fix

	int SSR = 1;

	float parallaxStrength = 0.03f; // POM height scale, 0 = disabled

	float toneMapSaturation = 1;
	float toneMapVibrance = 1;
	float toneMapGamma = 1;
	float toneMapShadowBoost = 0;
	float toneMapHighlightBoost = 0;
	float vignette = 0.15;
	glm::vec3 toneMapLift = glm::vec3(0.5);
	glm::vec3 toneMapGain = glm::vec3(0.5);

	void normalize();

	// Equality operator
	bool operator==(const ShadingSettings &other) const
	{
		return std::memcmp(this, &other, sizeof(ShadingSettings)) == 0;
	}

	// Inequality operator
	bool operator!=(const ShadingSettings &other) const
	{
		return !(*this == other);
	}

	std::string formatIntoGLSLcode();
};

ShadingSettings &getShadingSettings();

bool checkIfShadingSettingsChangedForShaderReloads();

void saveShadingSettings();

void loadShadingSettings();