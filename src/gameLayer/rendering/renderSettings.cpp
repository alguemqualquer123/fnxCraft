#include <rendering/renderSettings.h>
#include <rendering/performance.h>
#include <gamePlayLogic.h>
#include <filesystem>
#include <iostream>
#include <platform/platformInput.h>
#include <platform/platformDetection.h>
#include "multyPlayer/createConnection.h"
#include "multyPlayer/server.h"
#include "multyPlayer/enetServerFunction.h"
#include "gameLayer/Launcher.h"
#include <audioEngine.h>
#include <safeSave.h>
#include <sstream>
#include <localization.h>
#include <gameplay/player.h>

void displayRenderSettingsMenuButton(ProgramData &programData)
{
	programData.ui.menuRenderer.BeginMenu("Rendering", Colors_Gray, programData.ui.buttonTexture);

	displayRenderSettingsMenu(programData);

	programData.ui.menuRenderer.EndMenu();
}

#define DEFAULT_SLIDER Colors_White, programData.ui.buttonTexture, Colors_Gray, programData.ui.buttonTexture, Colors_White
#define DEFAULT_SLIDER_TRANSPARENT {1,1,1,0.65}, programData.ui.buttonTexture, {(float)0x7F / 255.0f, (float)0x7F / 255.0f, (float)0x7F / 255.0f, 0.65}, programData.ui.buttonTexture, {1,1,1,0.65}
#define DEFAULT_COLOR_PICKER programData.ui.buttonTexture, programData.ui.buttonTexture, Colors_Gray, Colors_Gray
#define DEFAULT_COLOR_PICKER_TRANSPARENT programData.ui.buttonTexture, programData.ui.buttonTexture, {(float)0x7F / 255.0f, (float)0x7F / 255.0f, (float)0x7F / 255.0f, 0.65}, {(float)0x7F / 255.0f, (float)0x7F / 255.0f, (float)0x7F / 255.0f, 0.65}

void displayRenderSettingsMenu(ProgramData &programData)
{

	for (auto &s : programData.ui.menuRenderer.internal.allMenuStacks
		[programData.ui.menuRenderer.internal.currentId])
	{
		if (s == "Rendering" || s == "Settings")
		{				programData.ui.renderer2d.renderText({150,50},
					(loc_FPS() + std::to_string(programData.currentFps)).c_str(), programData.ui.font, Colors_Gray, 0.75f);
			break;
		}
	}



	programData.ui.menuRenderer.Text(loc_RenderingSettings(), Colors_White);

	programData.ui.menuRenderer.sliderInt(loc_ViewDistance(), &getShadingSettings().viewDistance,
		1, 32, Colors_White, programData.ui.buttonTexture, Colors_Gray,
		programData.ui.buttonTexture, Colors_White);

	programData.ui.menuRenderer.sliderInt(loc_LodStrength(), &getShadingSettings().lodStrength,
		0, 5, Colors_White, programData.ui.buttonTexture, Colors_Gray,
		programData.ui.buttonTexture, Colors_White);

#pragma region water
{
	programData.ui.menuRenderer.BeginMenu("Water", Colors_Gray, programData.ui.buttonTexture);

	programData.ui.menuRenderer.Text(loc_WaterSettings(), Colors_White);


	programData.ui.menuRenderer.colorPicker(loc_WaterColor(),
		&programData.renderer.defaultShader.shadingSettings.waterColor[0],
		programData.ui.buttonTexture, programData.ui.buttonTexture, Colors_Gray, Colors_Gray);
	getShadingSettings().waterColor = programData.renderer.defaultShader.shadingSettings.waterColor;


	programData.ui.menuRenderer.colorPicker(loc_UnderWaterColor(),
		&programData.renderer.defaultShader.shadingSettings.underWaterColor[0],
		programData.ui.buttonTexture, programData.ui.buttonTexture, Colors_Gray, Colors_Gray);
	getShadingSettings().underWaterColor = programData.renderer.defaultShader.shadingSettings.underWaterColor;


	programData.ui.menuRenderer.sliderFloat(loc_UnderwaterFogStrength(),
		&programData.renderer.defaultShader.shadingSettings.underwaterDarkenStrength,
		0, 1, Colors_White, programData.ui.buttonTexture, Colors_Gray, 
		programData.ui.buttonTexture, Colors_White
		);
	getShadingSettings().underwaterDarkenStrength = programData.renderer.defaultShader.shadingSettings.underwaterDarkenStrength;


	programData.ui.menuRenderer.sliderFloat(loc_UnderwaterFogDistance(),
		&programData.renderer.defaultShader.shadingSettings.underwaterDarkenDistance,
		0, 40, Colors_White, programData.ui.buttonTexture, Colors_Gray,
		programData.ui.buttonTexture, Colors_White);
	getShadingSettings().underwaterDarkenDistance = programData.renderer.defaultShader.shadingSettings.underwaterDarkenDistance;


	programData.ui.menuRenderer.sliderFloat(loc_UnderwaterFogGradient(),
		&programData.renderer.defaultShader.shadingSettings.fogGradientUnderWater,
		0, 32, Colors_White, programData.ui.buttonTexture, Colors_Gray,
		programData.ui.buttonTexture, Colors_White);
	getShadingSettings().fogGradientUnderWater = programData.renderer.defaultShader.shadingSettings.fogGradientUnderWater;


	static glm::vec4 colors[] = {{0.6,0.9,0.6,1}, Colors_Red};

	programData.ui.menuRenderer.toggleOptions("Water type: ", "cheap|fancy",
		&getShadingSettings().waterType, true, Colors_White, colors, programData.ui.buttonTexture,
		Colors_Gray,
		"How the water should be rendered\n-Cheap: \
good performance.\n-Fancy: significant performance cost but looks very nice.");

	programData.ui.menuRenderer.EndMenu();
}
#pragma endregion

#pragma region bloom
	programData.ui.menuRenderer.BeginMenu("Bloom", Colors_Gray, programData.ui.buttonTexture);
	programData.ui.menuRenderer.Text(loc_BloomSettings(), Colors_White);

	programData.ui.menuRenderer.toggleOptions("BLoom: ", "OFF|ON", &getShadingSettings().bloom, true, Colors_White, 0, programData.ui.buttonTexture,
		Colors_Gray);

	if (getShadingSettings().bloom)
	{
		programData.ui.menuRenderer.sliderFloat(loc_BloomMultiplier(), &getShadingSettings().bloomMultiplier, 0, 1, DEFAULT_SLIDER);
		programData.ui.menuRenderer.sliderFloat(loc_BloomThreshold(), &getShadingSettings().bloomTresshold, 0.1, 1, DEFAULT_SLIDER);
	};

	programData.ui.menuRenderer.EndMenu();
#pragma endregion

#pragma region parallax
	programData.ui.menuRenderer.BeginMenu("Parallax", Colors_Gray, programData.ui.buttonTexture);
	programData.ui.menuRenderer.Text("Parallax occlusion mapping (block depth)", Colors_White);

	programData.ui.menuRenderer.sliderFloat("Strength: ", &getShadingSettings().parallaxStrength, 0.0, 0.1, DEFAULT_SLIDER);

	programData.ui.menuRenderer.EndMenu();
#pragma endregion

#pragma region lights
	programData.ui.menuRenderer.BeginMenu("Lights", Colors_Gray, programData.ui.buttonTexture);
	programData.ui.menuRenderer.Text(loc_LightsSettings(), Colors_White);

	programData.ui.menuRenderer.toggleOptions("Lights: ", "OFF|ON", &getShadingSettings().useLights, true, Colors_White, 0,
		programData.ui.buttonTexture, Colors_Gray, "If this is on, torches will contribute with specualr and diffuse lights.");

	if (getShadingSettings().useLights)
	{
		programData.ui.menuRenderer.sliderInt(loc_MaxLights(), &getShadingSettings().maxLights, 1, 100, DEFAULT_SLIDER);
		programData.ui.menuRenderer.sliderFloat(loc_LightsStrength(), &getShadingSettings().lightsStrength, 0.1, 2, DEFAULT_SLIDER);
	};

	programData.ui.menuRenderer.EndMenu();
#pragma endregion

#pragma region color post processing
	programData.ui.menuRenderer.BeginMenu("Color post processing", Colors_Gray, programData.ui.buttonTexture);

	programData.ui.menuRenderer.Text(loc_ColorPostProcessing(), {(float)0x7F / 255.0f, (float)0x7F / 255.0f, (float)0x7F / 255.0f, 0.65});

	//static glm::vec4 colorsTonemapper[] = {{0.6,0.9,0.6,1}, {0.6,0.9,0.6,1}, {0.7,0.8,0.6,1} , {0.4,0.8,0.4,1}};
	programData.ui.menuRenderer.toggleOptions(loc_Tonemapper(),
		"ACES|AgX|ZCAM|Uncharted|PBR neutral", &getShadingSettings().tonemapper,
		true, {(float)0x7F / 255.0f, (float)0x7F / 255.0f, (float)0x7F / 255.0f, 0.65}, nullptr, programData.ui.buttonTexture,
		{(float)0x7F / 255.0f, (float)0x7F / 255.0f, (float)0x7F / 255.0f, 0.65},
		"The tonemapper is the thing that displays the final color\n\
-Aces: a filmic look.\n-AgX: a more dull neutral look.\n-ZCAM a verey neutral and vanila look\n   preserves colors, slightly more expensive.\n-Unchrated :))");
	programData.renderer.defaultShader.shadingSettings.tonemapper = getShadingSettings().tonemapper;

	if (programData.ui.menuRenderer.Button(loc_ResetSettings(), {(float)0x7F / 255.0f, (float)0x7F / 255.0f, (float)0x7F / 255.0f, 0.65}, programData.ui.buttonTexture))
	{
		getShadingSettings().toneMapSaturation = 1;
		getShadingSettings().toneMapVibrance = 1;
		getShadingSettings().toneMapGamma = 1;
		getShadingSettings().toneMapShadowBoost = 0;
		getShadingSettings().toneMapHighlightBoost = 0;
		getShadingSettings().vignette = 0.15f;
		getShadingSettings().toneMapLift = glm::vec3(0.5);
		getShadingSettings().toneMapGain = glm::vec3(0.5);
	}

	programData.ui.menuRenderer.sliderFloat(loc_Vignette(), &getShadingSettings().vignette, 0, 1, DEFAULT_SLIDER_TRANSPARENT);
	programData.ui.menuRenderer.sliderFloat(loc_Saturation(), &getShadingSettings().toneMapSaturation, 0, 2, DEFAULT_SLIDER_TRANSPARENT);
	programData.ui.menuRenderer.sliderFloat(loc_Vibrance(), &getShadingSettings().toneMapVibrance, 0, 2, DEFAULT_SLIDER_TRANSPARENT);
	programData.ui.menuRenderer.sliderFloat(loc_Gamma(), &getShadingSettings().toneMapGamma, 0.1, 2, DEFAULT_SLIDER_TRANSPARENT);
	programData.ui.menuRenderer.sliderFloat(loc_ShadowBoost(), &getShadingSettings().toneMapShadowBoost, -1, 1, DEFAULT_SLIDER_TRANSPARENT);
	programData.ui.menuRenderer.sliderFloat(loc_HighlightBoost(), &getShadingSettings().toneMapHighlightBoost, -1, 1, DEFAULT_SLIDER_TRANSPARENT);
	

	programData.ui.menuRenderer.colorPicker(loc_Lift(), &getShadingSettings().toneMapLift[0], DEFAULT_COLOR_PICKER_TRANSPARENT);
	programData.ui.menuRenderer.colorPicker(loc_Gain(), &getShadingSettings().toneMapGain[0], DEFAULT_COLOR_PICKER_TRANSPARENT);

	//glUniform1f(applyToneMapper.u_saturation, shadingSettings.toneMapSaturation);
	//glUniform1f(applyToneMapper.u_vibrance, shadingSettings.toneMapVibrance);
	//glUniform1f(applyToneMapper.u_gamma, shadingSettings.toneMapGamma);
	//glUniform1f(applyToneMapper.u_shadowBoost, shadingSettings.toneMapShadowBoost);
	//glUniform1f(applyToneMapper.u_highlightBoost, shadingSettings.toneMapHighlightBoost);
	//glUniform3f(applyToneMapper.u_lift, shadingSettings.toneMapLift.x, shadingSettings.toneMapLift.y, shadingSettings.toneMapLift.z);
	//glUniform3f(applyToneMapper.u_gain, shadingSettings.toneMapGain.x, shadingSettings.toneMapGain.y, shadingSettings.toneMapGain.z);







	programData.ui.menuRenderer.EndMenu();

#pragma endregion






	programData.ui.menuRenderer.newColum(2);

	programData.ui.menuRenderer.Text("", {});
	
	programData.ui.menuRenderer.sliderInt(loc_ChunkBuildingThreads(), 
		&getShadingSettings().workerThreadsForBaking,
		0, 10, Colors_White, programData.ui.buttonTexture, Colors_Gray,
		programData.ui.buttonTexture, Colors_White);

	displayTexturePacksSettingsMenuButton(programData);

#pragma region SSR
	programData.ui.menuRenderer.BeginMenu("Screen Space Reflections", Colors_Gray, programData.ui.buttonTexture);
	programData.ui.menuRenderer.Text(loc_SSRSettings(), Colors_White);

	programData.ui.menuRenderer.toggleOptions("SSR: ", "OFF|ON", &getShadingSettings().SSR, true, Colors_White, 0, programData.ui.buttonTexture,
		Colors_Gray);

	if (getShadingSettings().SSR)
	{
		//... other SSR settings like quality stuff
	};

	programData.ui.menuRenderer.EndMenu();
#pragma endregion


	//programData.menuRenderer.BeginMenu("Volumetric", Colors_Gray, programData.buttonTexture);
	//programData.menuRenderer.Text("Volumetric Settings...", Colors_White);
	programData.ui.menuRenderer.sliderFloat(loc_FogGradient(),
		&getShadingSettings().fogGradient,
		0, 100, Colors_White, programData.ui.buttonTexture, Colors_Gray,
		programData.ui.buttonTexture, Colors_White);
	//programData.menuRenderer.EndMenu();


	static glm::vec4 colorsShadows[] = {{0.0,1,0.0,1}, {0.8,0.6,0.6,1}, {0.9,0.3,0.3,1}};
	programData.ui.menuRenderer.toggleOptions(loc_Shadows(), "Off|Hard|Soft",
		&getShadingSettings().shadows, true,
		Colors_White, colorsShadows, programData.ui.buttonTexture,
		Colors_Gray, "Shadows can affect the performance significantly."
	);


	programData.ui.menuRenderer.ToggleButton("FXAA", Colors_White, &getShadingSettings().FXAA, programData.ui.buttonTexture,
		Colors_Gray);

	programData.ui.menuRenderer.BeginMenu("Performance", Colors_Gray, programData.ui.buttonTexture);
	programData.ui.menuRenderer.Text("Frame Generation & VSync", Colors_White);
	{
		bool fg = getShadingSettings().frameGeneration;
		if(programData.ui.menuRenderer.ToggleButton("Frame Generation (2x FPS)", Colors_White, &fg, programData.ui.buttonTexture, Colors_Gray)){
			getShadingSettings().frameGeneration = fg;
			Performance::setFrameGeneration(fg, getShadingSettings().frameGenerationMode);
		}
		if(getShadingSettings().frameGeneration){
			programData.ui.menuRenderer.toggleOptions("FG Mode:", "Interp|Motion", &getShadingSettings().frameGenerationMode, true, Colors_White, 0, programData.ui.buttonTexture, Colors_Gray);
		}
	}
	{
		bool nb = getShadingSettings().nvidiaBoost;
		if(programData.ui.menuRenderer.ToggleButton("Nvidia Reflex Boost", Colors_White, &nb, programData.ui.buttonTexture, Colors_Gray)){
			getShadingSettings().nvidiaBoost=nb; Performance::setNvidiaBoost(nb);
		}
		bool al = getShadingSettings().amdAntiLag;
		if(programData.ui.menuRenderer.ToggleButton("AMD Anti-Lag", Colors_White, &al, programData.ui.buttonTexture, Colors_Gray)){
			getShadingSettings().amdAntiLag=al; Performance::setAmdAntiLag(al);
		}
	}
	programData.ui.menuRenderer.toggleOptions("VSync:", "Off|On|Adaptive (FreeSync/G-Sync)", &getShadingSettings().vsyncMode, true, Colors_White, 0, programData.ui.buttonTexture, Colors_Gray, "Adaptive = FreeSync/G-Sync, tear-free");
	if(programData.ui.menuRenderer.Button("Apply VSync", Colors_Gray, programData.ui.buttonTexture)){
		Performance::applyVSync((Performance::VSyncMode)getShadingSettings().vsyncMode);
	}
	programData.ui.menuRenderer.toggleOptions("MSAA:", "Off|2x|4x|8x", &getShadingSettings().msaa, true, Colors_White, 0, programData.ui.buttonTexture, Colors_Gray);
	if(programData.ui.menuRenderer.Button("Apply MSAA", Colors_Gray, programData.ui.buttonTexture)){
		int v=getShadingSettings().msaa; Performance::MSAA m=Performance::MSAA::Off; if(v==2)m=Performance::MSAA::X2; else if(v==4)m=Performance::MSAA::X4; else if(v==8)m=Performance::MSAA::X8; Performance::applyMSAA(m);
	}
	programData.ui.menuRenderer.sliderInt("Anisotropy", &getShadingSettings().anisotropy, 1, 16, Colors_White, programData.ui.buttonTexture, Colors_Gray, programData.ui.buttonTexture, Colors_White);
	if(programData.ui.menuRenderer.Button("Apply Anisotropy", Colors_Gray, programData.ui.buttonTexture)){
		Performance::applyAnisotropy(getShadingSettings().anisotropy);
	}
	programData.ui.menuRenderer.toggleOptions("FSR Upscaling:", "Off|Perf|Balanced|Quality|UltraQ", &getShadingSettings().fsr, true, Colors_White, 0, programData.ui.buttonTexture, Colors_Gray, "AMD FidelityFX Super Resolution - render low res, upscale");
	programData.ui.menuRenderer.Text(("GPU: "+Performance::getGpuVendor()).c_str(), glm::vec4(0.6f,0.8f,1.f,1.f));
	if(programData.ui.menuRenderer.Button("Auto Optimize", Colors_White, programData.ui.buttonTexture)){
		Performance::autoOptimize();
	}
	programData.ui.menuRenderer.Text(Performance::getPerformanceReport().c_str(), glm::vec4(0.7f,0.7f,0.7f,1.f));
	programData.ui.menuRenderer.EndMenu();

}

glm::ivec4 shrinkPercentage(glm::ivec4 dimensions, glm::vec2 p)
{
	glm::vec4 b = dimensions;

	b.x += (b.z * p.x) / 2.f;
	b.y += (b.w * p.y) / 2.f;

	b.z *= (1.f - p.x);
	b.w *= (1.f - p.y);

	dimensions = b;
	return dimensions;
}

bool texturePackDirty = 1;
int leftAdvance = 0;
int rightAdvance = 0;
std::vector<std::filesystem::path> loadedTexturePacks;
std::unordered_map<std::string, gl2d::Texture> logoTextures;

std::vector<std::filesystem::path> usedTexturePacks{"ourcraft"};

void displaySettingsMenuButton(ProgramData &programData)
{
	programData.ui.menuRenderer.BeginMenu("Settings", Colors_Gray, programData.ui.buttonTexture);

	displaySettingsMenu(programData);

	programData.ui.menuRenderer.EndMenu();
}

void displayLanguageMenuButton(ProgramData &programData)
{
	programData.ui.menuRenderer.BeginMenu("Language", Colors_Gray, programData.ui.buttonTexture);

	programData.ui.menuRenderer.Text(loc_LanguageName(), Colors_White);

	Language &lang = getCurrentLanguage();

	if (programData.ui.menuRenderer.Button(
		(lang == Language::English) ? "> English" : "English",
		Colors_Gray, programData.ui.buttonTexture))
	{
		setCurrentLanguage(Language::English);
		saveLanguageSettings();
	}

	if (programData.ui.menuRenderer.Button(
		(lang == Language::Portuguese_BR) ? "> Portugues (BR)" : "Portugues (BR)",
		Colors_Gray, programData.ui.buttonTexture))
	{
		setCurrentLanguage(Language::Portuguese_BR);
		saveLanguageSettings();
	}

	programData.ui.menuRenderer.EndMenu();
}

void displaySettingsMenu(ProgramData &programData)
{

	programData.ui.menuRenderer.Text(loc_Settings(), Colors_White);

	displayRenderSettingsMenuButton(programData);
	
	displayVolumeMenuButton(programData);

	displayLanguageMenuButton(programData);
}

bool shouldReloadTexturePacks()
{
	return texturePackDirty;
}

std::vector<std::filesystem::path> getUsedTexturePacksAndResetFlag()
{
	texturePackDirty = false;
	return usedTexturePacks;
}

bool isTexturePackUsed(std::string t)
{
	for (auto &t2 : usedTexturePacks)
	{
		if (t == t2) { return true; }
	}
	return false;
}

void useTexturePack(std::string t)
{
	if (!isTexturePackUsed(t))
	{
		usedTexturePacks.push_back(t);
		texturePackDirty = true;
	}
}

void unuseTexturePack(std::string t)
{
	auto f = std::find(usedTexturePacks.begin(), usedTexturePacks.end(), t);

	if (f != usedTexturePacks.end())
	{
		usedTexturePacks.erase(f);
		texturePackDirty = true;
	}
}

void displayTexturePacksSettingsMenuButton(ProgramData &programData)
{
	programData.ui.menuRenderer.BeginMenu("Textures Packs", Colors_Gray, programData.ui.buttonTexture);

	displayTexturePacksSettingsMenu(programData);

	programData.ui.menuRenderer.EndMenu();
}


void openFolder(const char *path)
{
	//TODO!
#ifdef PLATFORM_WINDOWS
	std::string command = "explorer ";
	command += path;
	system(command.c_str());
#elif defined(PLATFORM_MACOS)
	std::string command = "open ";
	command += path;
	system(command.c_str());
#elif defined(PLATFORM_LINUX)
	std::string command = "xdg-open ";
	command += path;
	system(command.c_str());
#endif
}

inline void stubErrorFunc(const char *msg, void *userDefinedData)
{
	
}


bool renderButton(gl2d::Renderer2D &renderer,
	gl2d::Texture buttonTexture,
	glm::ivec4 box, const std::string &text = "", gl2d::Font *f = 0)
{
	bool hovered = 0;
	bool held = 0;
	bool released = 0;
	auto cursorPos = platform::getRelMousePosition();

	if (glui::aabb(box, cursorPos))
	{
		hovered = 1;

		if (platform::isLMouseHeld())
		{
			held = 1;
		}
		else if (platform::isLMouseReleased())
		{
			released = 1;
		}
	}

	if (held)
	{
		box.y += 10;
	}

	if (buttonTexture.id)
	{

		auto color = Colors_Gray;

		if (hovered)
		{
			color += glm::vec4(0.1, 0.1, 0.1, 0);
		}

		renderer.render9Patch(box,
			20, color, {}, 0, buttonTexture,
			GL2D_DefaultTextureCoords, {0.2,0.8,0.8,0.2});
	}

	if (text.size() && f)
	{
		glui::renderText(renderer, text, *f, box, Colors_White, 0, true, true);
	}

	return released;
};


void displayTexturePacksSettingsMenu(ProgramData &programData)
{
	std::error_code err;


	programData.ui.menuRenderer.Text(loc_TexturesPacks(), Colors_White);

	//todo
	//if (programData.ui.menuRenderer.Button("Open Folder", Colors_Gray))
	//{
	//	if (!std::filesystem::exists(RESOURCES_PATH "texturePacks"))
	//	{
	//		std::filesystem::create_directories(RESOURCES_PATH "texturePacks", err);
	//	}
	//	
	//	openFolder(RESOURCES_PATH "texturePacks");
	//}


	glm::vec4 customWidgetTransform = {};
	programData.ui.menuRenderer.CustomWidget(169, &customWidgetTransform);



	//programData.ui.menuRenderer.cu
	auto &renderer = programData.ui.renderer2d;


	auto renderBox = [&](glm::vec4 c)
	{
		renderer.render9Patch(glui::Box().xCenter().yCenter().xDimensionPercentage(1).yDimensionPercentage(1.f)(),
			20, c, {}, 0, programData.ui.buttonTexture, GL2D_DefaultTextureCoords, {0.2,0.8,0.8,0.2});
	};

	if (programData.ui.menuRenderer.internal.allMenuStacks
		[programData.ui.menuRenderer.internal.currentId].size()
		&& programData.ui.menuRenderer.internal.allMenuStacks
		[programData.ui.menuRenderer.internal.currentId].back() == "Textures Packs"
		)
	{

		std::vector<std::filesystem::path> allTexturePacks;

		if (!std::filesystem::exists(RESOURCES_PATH "texturePacks"))
		{
			std::filesystem::create_directories(RESOURCES_PATH "texturePacks", err);
		}

		for (auto const &d : std::filesystem::directory_iterator{RESOURCES_PATH "texturePacks", err})
		{
			if (d.is_directory())
			{
				allTexturePacks.push_back(d.path().filename());
			}
		}

		//load logo textures
		gl2d::setErrorFuncCallback(stubErrorFunc);
		for (auto &pack : allTexturePacks)
		{
			auto file = pack;
			file = (RESOURCES_PATH "texturePacks") / file;
			file /= "logo.png";

			if (logoTextures.find(pack.string()) == logoTextures.end())
			{
				gl2d::Texture t;
				t.loadFromFile(file.string().c_str());

				if (!t.id)
				{
					file = pack;
					file = (RESOURCES_PATH "texturePacks") / file;
					file /= "pack.png";
					t.loadFromFile(file.string().c_str(), true);

				}

				logoTextures[pack.string()] = t;
			}
		}
		gl2d::setErrorFuncCallback(gl2d::defaultErrorFunc);
		//TODO delete unused entries


		auto listImplementation = [&](bool left)
		{
			std::vector<std::filesystem::path> *listPacks = 0;
			int *advance = 0;
			//buttons
			glm::ivec4 b;
			if (left)
			{
				b = glui::Box().xLeft().yTop().
					xDimensionPercentage(0.5).yDimensionPercentage(1).shrinkPercentage({0.05,0.05});
				listPacks = &allTexturePacks;
				advance = &leftAdvance;
			}
			else
			{
				b = glui::Box().xRight().yTop().
					xDimensionPercentage(0.5).yDimensionPercentage(1).shrinkPercentage({0.05,0.05});
				listPacks = &usedTexturePacks;
				advance = &rightAdvance;
			}

			glui::Frame f(b);

			float buttonSize = glui::Box().xLeft().yTop().xDimensionPercentage(0.1)().z;

			auto currentUpperBox = glui::Box().xCenter().yCenter().xDimensionPercentage(1).yDimensionPercentage(1.f)();

			//center
			{

				glui::Frame f(glui::Box().xCenter().yCenter().
					xDimensionPercentage(1).
					yDimensionPixels(currentUpperBox.w - buttonSize * 2)());

				auto currentBox = glui::Box().xCenter().yCenter().xDimensionPercentage(1).yDimensionPercentage(1.f)();

				//renderer.renderRectangle(,
				//	{0,1,0,0.2});

				renderBox({0.6,0.6,0.6,0.5});

				int height = currentBox.w - 10;

				int maxH = std::min(height, int(currentBox.z * 0.2));

				auto defaultB = glui::Box().xLeft().yTop().xDimensionPercentage(1).yDimensionPixels(maxH)();
				defaultB.y += 10;

				auto renderOneTeturePack = [&](glm::ivec4 box,
					std::string name)
				{
					box = shrinkPercentage(box, {0.1,0.05});

					renderer.render9Patch(box,
						20, {0.6,0.6,0.6,0.9}, {}, 0, programData.ui.buttonTexture,
						GL2D_DefaultTextureCoords, {0.2,0.8,0.8,0.2});

					{
						glui::Frame f(box);

						auto button = glui::Box().xRight().yTop().yDimensionPercentage(1.f).xDimensionPixels(buttonSize)();
						if (renderButton(renderer, programData.ui.buttonTexture, button))
						{
							if (left)
							{
								useTexturePack(name);
							}
							else
							{
								unuseTexturePack(name);
							}
						}

						auto icon = glui::Box().xLeft().yTop().yDimensionPercentage(1.f).xDimensionPixels(box.w)();

						if (logoTextures[name].id)
						{
							glui::renderTexture(renderer, shrinkPercentage(icon, {0.1,0.1}),
								logoTextures[name], Colors_White, GL2D_DefaultTextureCoords);
						}
						else
						{
							glui::renderTexture(renderer, shrinkPercentage(icon, {0.1,0.1}),
								programData.defaultCover, Colors_White, GL2D_DefaultTextureCoords);
						}


						auto text = icon;
						text.x += icon.z;
						text.z = button.x - text.x;

						glui::renderText(renderer, name, programData.ui.font, text, Colors_White, true);
					}


				};

				int canRenderCount = height / defaultB.w;

				int overflow = listPacks->size() - canRenderCount;
				if (overflow < 0) { overflow = 0; }
				if (*advance > overflow) { *advance = overflow; }

				for (int i = 0; i < listPacks->size(); i++)
				{
					if (i >= canRenderCount) { break; }

					renderOneTeturePack(defaultB, (*listPacks)[i + *advance].filename().string());
					defaultB.y += defaultB.w;
				}


			}

			//bottom button
			{
				auto currentDownBox = glui::Box().xLeft().yBottom().xDimensionPercentage(1).yDimensionPixels(buttonSize)();

				if (renderButton(renderer, programData.ui.buttonTexture, currentDownBox))
				{
					(*advance)++;
				}
			}

			//top button
			{
				auto currentUpperBox = glui::Box().xLeft().yTop().xDimensionPercentage(1).yDimensionPixels(buttonSize)();

				if (renderButton(renderer, programData.ui.buttonTexture, currentUpperBox))
				{
					(*advance)--;
				}
			}

			if (*advance < 0) { *advance = 0; }

		};
		

		glui::Frame f({0,0,renderer.windowW, renderer.windowH});

		{
			float ySize = renderer.windowH - customWidgetTransform.y - customWidgetTransform.w/2.f;

			glui::Frame f(glui::Box().xCenter().yTop(customWidgetTransform.y).
				yDimensionPixels(ySize).xDimensionPercentage(0.9)());

			//renderer.renderRectangle(glui::Box().xCenter().yCenter().xDimensionPercentage(1).yDimensionPercentage(1.f),
			//	{1,0,0,0.2});
			renderBox({0.2,0.2,0.2,0.4});

			//left
			{
				listImplementation(true);
			}

			//rioght
			{
				listImplementation(false);
			}

		}

	}
}

std::string currentSkinSelected = "";

gl2d::Texture currentTextureLoaded;
std::string currentTextureLoadedName = "";

void loadTexture()
{

	if (currentTextureLoadedName != currentSkinSelected || !currentTextureLoaded.id)
	{
		currentTextureLoaded.cleanup();
	}

	if(!currentTextureLoaded.id)
	{
		currentTextureLoadedName = currentSkinSelected;

		if (currentTextureLoadedName == "")
		{
			currentTextureLoaded
				= loadPlayerSkin(RESOURCES_PATH "assets/models/steve.png");
		}
		else
		{
			currentTextureLoaded = loadPlayerSkin((RESOURCES_PATH "skins/" + currentTextureLoadedName + ".png").c_str());
			if(!currentTextureLoaded.id) currentTextureLoaded = loadPlayerSkin(("./resources/skins/" + currentTextureLoadedName + ".png").c_str());
			if(!currentTextureLoaded.id) currentTextureLoaded = loadPlayerSkin(("resources/skins/" + currentTextureLoadedName + ".png").c_str());
		}
	}

}

std::string getSkinName()
{
	return currentSkinSelected;
}

ShadingSettings shadingSettings;
ShadingSettings shadingSettingsLastFrame;

ShadingSettings &getShadingSettings()
{
	return shadingSettings;
}

bool checkIfShadingSettingsChangedForShaderReloads()
{
	shadingSettings.normalize();
	if (
		shadingSettings.shadows != shadingSettingsLastFrame.shadows ||
		shadingSettings.PBR != shadingSettingsLastFrame.PBR ||
		shadingSettings.useLights != shadingSettingsLastFrame.useLights ||
		shadingSettings.SSR != shadingSettingsLastFrame.SSR 
		)
	{
		shadingSettingsLastFrame = shadingSettings;
		return true;
	}

	return false;
}

#define SET_INT(x) data.setInt( #x, shadingSettings. x )
#define SET_VEC3(x) data.setRawData( #x, &shadingSettings. x [0], sizeof(shadingSettings. x) )
#define SET_FLOAT(x) data.setFloat( #x, shadingSettings. x )


void saveShadingSettings()
{

	shadingSettings.normalize();

	sfs::SafeSafeKeyValueData data;
	data.setInt("Version", 1);

	SET_INT(viewDistance);
	SET_INT(lodStrength);
	SET_INT(workerThreadsForBaking);
	SET_INT(tonemapper);
	SET_INT(shadows);
	SET_INT(waterType);
	SET_INT(PBR);
	SET_INT(SSR);
	SET_INT(maxLights);
	SET_INT(useLights);
	SET_INT(bloom);
	SET_FLOAT(lightsStrength);
	SET_INT(frameGeneration);
	SET_INT(frameGenerationMode);
	SET_INT(nvidiaBoost);
	SET_INT(msaa);
	SET_INT(vsyncMode);
	SET_INT(fsr);
	SET_INT(amdAntiLag);
	SET_INT(anisotropy);

	SET_VEC3(waterColor);
	SET_VEC3(underWaterColor);

	SET_FLOAT(underwaterDarkenStrength);
	SET_FLOAT(underwaterDarkenDistance);
	SET_FLOAT(fogGradientUnderWater);
	SET_FLOAT(bloomTresshold);
	SET_FLOAT(bloomMultiplier);
	SET_FLOAT(exposure);
	SET_FLOAT(fogGradient);
	SET_FLOAT(parallaxStrength);


	SET_FLOAT(toneMapSaturation);
	SET_FLOAT(toneMapVibrance);
	SET_FLOAT(toneMapGamma);
	SET_FLOAT(toneMapShadowBoost);
	SET_FLOAT(toneMapHighlightBoost);
	SET_FLOAT(vignette);
	SET_VEC3(toneMapLift);
	SET_VEC3(toneMapGain);

	data.setBool("FXAA", shadingSettings.FXAA);



	sfs::safeSave(data, RESOURCES_PATH "../playerSettings/renderSettings", 0);

}

#undef SET_INT
#undef SET_VEC3
#undef SET_FLOAT



#define GET_INT(x) data.getInt( #x, shadingSettings. x )
#define GET_VEC3(x) data.setRawData( #x, &shadingSettings. x [0], sizeof(shadingSettings. x) )
#define GET_FLOAT(x) data.getFloat( #x, shadingSettings. x )
void loadShadingSettings()
{

	shadingSettings = ShadingSettings{};

	sfs::SafeSafeKeyValueData data;

	if (sfs::safeLoad(data, RESOURCES_PATH "../playerSettings/renderSettings", 0) == sfs::noError)
	{
		GET_INT(viewDistance);
		GET_INT(lodStrength);
		GET_INT(workerThreadsForBaking);
		GET_INT(tonemapper);
		GET_INT(shadows);
		GET_INT(waterType);
		GET_INT(PBR);
		GET_INT(SSR);
		GET_INT(maxLights);
		GET_INT(bloom);
		GET_INT(useLights);
		GET_FLOAT(lightsStrength);
		GET_INT(frameGeneration);
		GET_INT(frameGenerationMode);
		GET_INT(nvidiaBoost);
		GET_INT(msaa);
		GET_INT(vsyncMode);
		GET_INT(fsr);
		GET_INT(amdAntiLag);
		GET_INT(anisotropy);

		GET_FLOAT(toneMapSaturation);
		GET_FLOAT(toneMapVibrance);
		GET_FLOAT(toneMapGamma);
		GET_FLOAT(toneMapShadowBoost);
		GET_FLOAT(toneMapHighlightBoost);
		GET_FLOAT(vignette);


		void *rawData = 0;
		size_t dataSize = 0;
		data.getRawDataPointer("waterColor", rawData, dataSize);
		if (dataSize == sizeof(shadingSettings.waterColor))
			{ memcpy(&shadingSettings.waterColor[0], rawData, dataSize); }

		data.getRawDataPointer("underWaterColor", rawData, dataSize);
		if (dataSize == sizeof(shadingSettings.underWaterColor))
			{ memcpy(&shadingSettings.underWaterColor[0], rawData, dataSize); }

		data.getRawDataPointer("toneMapLift", rawData, dataSize);
		if (dataSize == sizeof(shadingSettings.toneMapLift))
			{ memcpy(&shadingSettings.toneMapLift[0], rawData, dataSize); }

		data.getRawDataPointer("toneMapGain", rawData, dataSize);
		if (dataSize == sizeof(shadingSettings.toneMapGain))
			{ memcpy(&shadingSettings.toneMapGain[0], rawData, dataSize); }

		data.getBool("FXAA", shadingSettings.FXAA);

		GET_FLOAT(underwaterDarkenStrength);
		GET_FLOAT(underwaterDarkenDistance);
		GET_FLOAT(fogGradientUnderWater);
		GET_FLOAT(exposure);

		GET_FLOAT(bloomTresshold);
		GET_FLOAT(bloomMultiplier);
		GET_FLOAT(fogGradient);
		GET_FLOAT(parallaxStrength);

	}

	shadingSettings.normalize();

	//todo apply

}

#undef GET_INT
#undef GET_VEC3
#undef GET_FLOAT


void displaySkinSelectorMenu(ProgramData &programData)
{
	std::error_code err;

	programData.ui.menuRenderer.Text(loc_ChangeSkin(), Colors_White);

	//todo
	//if (programData.ui.menuRenderer.Button("Open Folder", Colors_Gray))
	//{
	//	if (!std::filesystem::exists(RESOURCES_PATH "texturePacks"))
	//	{
	//		std::filesystem::create_directories(RESOURCES_PATH "texturePacks", err);
	//	}
	//	
	//	openFolder(RESOURCES_PATH "texturePacks");
	//}

	glm::vec4 customWidgetTransform = {};
	programData.ui.menuRenderer.CustomWidget(170, &customWidgetTransform);

	//programData.ui.menuRenderer.cu
	auto &renderer = programData.ui.renderer2d;
	
	auto renderBox = [&](glm::vec4 c)
	{
		renderer.render9Patch(glui::Box().xCenter().yCenter().xDimensionPercentage(1).yDimensionPercentage(1.f)(),
			20, c, {}, 0, programData.ui.buttonTexture, GL2D_DefaultTextureCoords, {0.2,0.8,0.8,0.2});
	};

	if (programData.ui.menuRenderer.internal.allMenuStacks
		[programData.ui.menuRenderer.internal.currentId].size()
		&& programData.ui.menuRenderer.internal.allMenuStacks
		[programData.ui.menuRenderer.internal.currentId].back() == "Change Skin"
		)
	{

		if (!std::filesystem::exists(RESOURCES_PATH "skins"))
		{
			std::filesystem::create_directories(RESOURCES_PATH "skins", err);
		}

		std::vector<std::string> skins;
		auto collectSkins = [&](const char* p){
			std::error_code e2;
			if(!std::filesystem::exists(p, e2)) return;
			for (auto const &d : std::filesystem::directory_iterator{p, e2})
			{
				if (!d.is_directory())
				{
					if (d.path().filename().extension() == ".png")
					{
						std::string name = d.path().filename().stem().string();
						if(std::find(skins.begin(), skins.end(), name)==skins.end())
							skins.push_back(name);
					}
				}
			}
		};
		collectSkins(RESOURCES_PATH "skins");
		collectSkins("./resources/skins");
		collectSkins("resources/skins");
		std::sort(skins.begin(), skins.end());

		int posInVect = -1;
		if (currentSkinSelected == "")
		{
			posInVect = -1;
		}
		else
		{
			int index = 0;
			for(auto &s : skins)
			{
				if (s == currentSkinSelected)
				{
					currentSkinSelected = s;
					posInVect = index;
					break;
				}
				index++;
			}
		}

		

		glui::Frame f({0,0,renderer.windowW, renderer.windowH});
		{
			float ySize = renderer.windowH - customWidgetTransform.y - customWidgetTransform.w / 2.f;

			glui::Frame f(glui::Box().xCenter().yTop(customWidgetTransform.y).
				yDimensionPixels(ySize).xDimensionPercentage(0.9)());

			//renderer.renderRectangle(glui::Box().xCenter().yCenter().xDimensionPercentage(1).yDimensionPercentage(1.f),
			//	{1,0,0,1.0});
			renderBox({0.4,0.4,0.4,0.5});


			auto textBox = glui::Box().xCenter().yBottom().xDimensionPercentage(1).yDimensionPercentage(0.2)();
			std::string label = currentSkinSelected == "" ? "Default" : currentSkinSelected;
			label += " (" + std::to_string(posInVect+2) + "/" + std::to_string(skins.size()+1) + ")";
			glui::renderText(renderer, label, programData.ui.font, textBox, Colors_White, true);

			auto center = glui::Box().xCenter().yCenter().yDimensionPercentage(0.5).xDimensionPercentage(0.5)();
			center.z = std::min(center.z, center.w);
			center = glui::Box().xCenter().yCenter().yDimensionPixels(center.z).xDimensionPixels(center.z)();

			loadTexture();
			if (currentTextureLoaded.id)
			{
				renderer.renderRectangle(center, currentTextureLoaded);
			}

			auto left = glui::Box().xLeft().yCenter().yDimensionPercentage(0.2).xAspectRatio(1.0)();
			auto right = glui::Box().xRight().yCenter().yDimensionPercentage(0.2).xAspectRatio(1.0)();

			//move cursor
			{
				if (renderButton(renderer, programData.ui.buttonTexture, left))
				{
					posInVect--;
				}

				if (renderButton(renderer, programData.ui.buttonTexture, right))
				{
					posInVect++;
				}

				if (posInVect < -1)
				{
					posInVect = skins.size() - 1;
				}

				if (posInVect >= skins.size())
				{
					posInVect = -1;
				}

				if (posInVect == -1)
				{
					currentSkinSelected = "";
				}
				else
				{
					currentSkinSelected = skins[posInVect];
				}
			}

		
		}
	}

}

void displaySkinSelectorMenuButton(ProgramData &programData)
{
	programData.ui.menuRenderer.BeginMenu("Change Skin", Colors_Gray, programData.ui.buttonTexture);

	displaySkinSelectorMenu(programData);

	programData.ui.menuRenderer.EndMenu();
}

void displayVolumeMenuButton(ProgramData &programData)
{
	programData.ui.menuRenderer.BeginMenu("Audio Settings", Colors_Gray, programData.ui.buttonTexture);

	displayVolumeMenu(programData);

	programData.ui.menuRenderer.EndMenu();
}

void displayVolumeMenu(ProgramData &programData)
{

	programData.ui.menuRenderer.Text(loc_AudioSettings(), Colors_White);

	programData.ui.menuRenderer.sliderFloat(loc_MasterVolume(), &AudioEngine::getMasterVolume(), 0, 1, Colors_White, programData.ui.buttonTexture, Colors_Gray, programData.ui.buttonTexture, Colors_White);
	programData.ui.menuRenderer.newLine();
	programData.ui.menuRenderer.sliderFloat(loc_MusicVolume(), &AudioEngine::getMusicVolume(), 0, 1, Colors_White, programData.ui.buttonTexture, Colors_Gray, programData.ui.buttonTexture, Colors_White);
	programData.ui.menuRenderer.sliderFloat(loc_UIVolume(), &AudioEngine::getUIVolume(), 0, 1, Colors_White, programData.ui.buttonTexture, Colors_Gray, programData.ui.buttonTexture, Colors_White);
	programData.ui.menuRenderer.sliderFloat(loc_SoundsVolume(), &AudioEngine::getSoundsVolume(), 0, 1, Colors_White, programData.ui.buttonTexture, Colors_Gray, programData.ui.buttonTexture, Colors_White);

}


void displayWorldSelectorMenuButton(ProgramData &programData)
{
	programData.ui.menuRenderer.BeginMenu("Play", Colors_Gray, programData.ui.buttonTexture);

	displayWorldSelectorMenu(programData);

	programData.ui.menuRenderer.EndMenu();
}


void displayWorldSelectorMenu(ProgramData &programData)
{
	auto &renderer = programData.ui.renderer2d;



	programData.ui.menuRenderer.Text(loc_SelectWorld(), Colors_White);

	//programData.ui.menuRenderer.Button("Create new world", Colors_Gray, programData.ui.buttonTexture);
	programData.ui.menuRenderer.Text("", Colors_White);

	glm::vec4 customWidgetTransform = {};
	programData.ui.menuRenderer.CustomWidget(171, &customWidgetTransform);

	glui::Frame f({0,0,renderer.windowW, renderer.windowH});


	auto drawButton = [&](glm::vec4 transform, glm::vec4 color,
		const std::string &s)
	{
		return glui::drawButton(renderer, transform, color, s, programData.ui.font, programData.ui.buttonTexture,
			platform::getRelMousePosition(), platform::isLMouseHeld(), platform::isLMouseReleased());
	};

	auto drawBackground = [&]()
	{
		float rezolution = 256;
		glm::vec2 size{renderer.windowW, renderer.windowH};
		size /= 256.f;

		renderer.renderRectangle({0,0, renderer.windowW, renderer.windowH},
			programData.blocksLoader.backgroundTexture, {0.6,0.6,0.6,1}, {}, 0,
			{0,size.y, size.x, 0}
		);

	};

	static std::string selected = "";

	if (programData.ui.menuRenderer.internal.allMenuStacks
		[programData.ui.menuRenderer.internal.currentId].size()
		&& programData.ui.menuRenderer.internal.allMenuStacks
		[programData.ui.menuRenderer.internal.currentId].back() == "Play"
		)
	{

		drawBackground();

		//folder logic

		if (!std::filesystem::exists(RESOURCES_PATH "worlds"))
		{
			std::filesystem::create_directories(RESOURCES_PATH "worlds");
		}
			
		std::vector<std::filesystem::path> allWorlds;
		for (auto const &d : std::filesystem::directory_iterator{RESOURCES_PATH "worlds"})
		{
			if (d.is_directory())
			{
				allWorlds.push_back(d.path().filename());
			}
		}


		//center
		{
			float ySize = renderer.windowH - customWidgetTransform.y * 2;
			
			if (ySize > 50)
			{
				static int advance = 0;

				glui::Frame f(glui::Box().xCenter().yTop(customWidgetTransform.y).
					yDimensionPixels(ySize).xDimensionPercentage(1)());
				auto fullBox = glui::Box().xLeft().yTop().xDimensionPercentage(1.f).yDimensionPercentage(1.f)();

				float buttonH = customWidgetTransform.w;
				int maxElements = fullBox.w / buttonH;

				advance = std::max(advance, 0);
				int overflow = allWorlds.size() - maxElements;
				if (overflow < 0) { overflow = 0; }
				advance = std::min(advance, overflow);

				//background
				{
					float rezolution = 256;
					glm::vec2 size{fullBox.z, fullBox.w};
					size /= 256.f;

					float padding = advance * 0.4;

					renderer.renderRectangle(fullBox,
						programData.blocksLoader.backgroundTexture, {0.4,0.4,0.4,1}, {}, 0,
						{padding,size.y, size.x + padding, 0}
					);
				}

				//renderer.renderRectangle(glui::Box().xCenter().yCenter().xDimensionPercentage(1).yDimensionPercentage(1.f),
				//	{1,0,0,1.0});

				glm::vec4 worldBox = fullBox;
				worldBox.w = buttonH;
				worldBox.z -= buttonH;

				for (int i = 0; i < maxElements; i++)
				{
					if (allWorlds.size() <= i + advance) { break; }

					auto s = allWorlds[i + advance].string();

					if (s == selected)
					{
						renderer.render9Patch(worldBox,
							20, {0.3,0.3,0.3,0.7}, {}, 0, programData.ui.buttonTexture,
							GL2D_DefaultTextureCoords, {0.2,0.8,0.8,0.2});
					}

					if (renderButton(renderer, {}, worldBox, s, &programData.ui.font))
					{
						selected = s;
					}

					worldBox.y += buttonH;
				}

				auto topButton = glui::Box().yTop().xRight().yDimensionPixels(buttonH).xDimensionPixels(buttonH)();
				auto bottomButton = glui::Box().yBottom().xRight().yDimensionPixels(buttonH).xDimensionPixels(buttonH)();

				if (renderButton(renderer, programData.ui.buttonTexture, topButton)) { advance--; }
				if (renderButton(renderer, programData.ui.buttonTexture, bottomButton)) { advance++; }

			}

		}

		//bottom
		{
			float ySize = customWidgetTransform.y;
			glui::Frame f(glui::Box().xCenter().yBottom().
				yDimensionPixels(ySize).xDimensionPercentage(1)());

			//renderer.renderRectangle(glui::Box().xCenter().yCenter().xDimensionPercentage(1).yDimensionPercentage(1.f),
			//	{0,1,0,1.0});


			//top
			{
				glui::Frame f(glui::Box().xCenter().yTop().
					yDimensionPercentage(0.5).xDimensionPercentage(1)());

				{
					auto leftButton = glui::Box().xLeft().yCenter().xDimensionPercentage(0.5).yDimensionPercentage(1)();
					if (drawButton(shrinkPercentage(leftButton, {0.1,0.05}), Colors_Gray, "Play Selected World"))
					{
						if (selected.size())
						{
							hostServer(selected);
						}

					}

					auto rightButton = glui::Box().xRight().yCenter().xDimensionPercentage(0.5).yDimensionPercentage(1)();
					if (drawButton(shrinkPercentage(rightButton, {0.1,0.05}), Colors_Gray, loc_Settings()))
					{
						programData.ui.menuRenderer.ExitCurrentMenu();
					}


				}

			}

			//bottom
			{
				glui::Frame f(glui::Box().xCenter().yBottom().
					yDimensionPercentage(0.5).xDimensionPercentage(1)());

				{
					auto leftButton = glui::Box().xLeft().yCenter().xDimensionPercentage(0.5).yDimensionPercentage(1)();
					if (drawButton(shrinkPercentage(leftButton, {0.1,0.05}), Colors_Gray, loc_CreateNewWorld()))
					{
						programData.ui.menuRenderer.StartManualMenu("Create world");
					}

					auto rightButton = glui::Box().xRight().yCenter().xDimensionPercentage(0.5).yDimensionPercentage(1)();
					if (drawButton(shrinkPercentage(rightButton, {0.1,0.05}), Colors_Gray, "Delete world"))
					{
						if (selected.size())
						{
							programData.ui.menuRenderer.StartManualMenu("Delete world");
						}
					}
				}


			}

			if (selected.size())
			{
				if (programData.ui.menuRenderer.Button("Configurar mundo", Colors_Gray, programData.ui.buttonTexture))
				{
					programData.ui.menuRenderer.StartManualMenu("World Config");
				}
			}
			displayWorldConfigMenu(programData, selected);
		}

	}

	static char seed[12] = {};
	static char name[20] = {};
	static int currentIndex = 0; //0 normal, 1 super flat
	static int createDifficulty = 2; //0 peaceful 1 easy 2 normal 3 hard
	static int createGamemode = 0; //0 survival 1 creative
	static int createKeepInventory = 0; //0 off 1 on
	static int createAllowCheats = 0; //0 off 1 on
	static int createPvp = 0;
	static WorldGeneratorSettings settings;
	static gl2d::Texture worldPreviewTexture;
	static WorldGenerator wg;
	if (wg.regionsHeightNoise == 0)
	{
		wg.init();
	}



	programData.ui.menuRenderer.BeginManualMenu("Delete world");

	if (programData.ui.menuRenderer.internal.allMenuStacks
		[programData.ui.menuRenderer.internal.currentId].size()
		&& programData.ui.menuRenderer.internal.allMenuStacks
		[programData.ui.menuRenderer.internal.currentId].back() == "Delete world"
		)
	{

		if (selected.size())
		{
			drawBackground();

			programData.ui.menuRenderer.Text("Are you sure you want to delete:", Colors_White);
			programData.ui.menuRenderer.Text(selected.c_str(), Colors_White);


			if (programData.ui.menuRenderer.Button("Delete", Colors_Gray, programData.ui.buttonTexture))
			{
				std::string deletePath = RESOURCES_PATH "worlds/";
				deletePath += selected;

				std::error_code error;
				std::filesystem::remove_all(deletePath, error);

				selected = {};

				programData.ui.menuRenderer.ExitCurrentMenu();
			}

			if (programData.ui.menuRenderer.Button("Cancel", Colors_Gray, programData.ui.buttonTexture))
			{
				programData.ui.menuRenderer.ExitCurrentMenu();
			}
		}
		else
		{
			programData.ui.menuRenderer.ExitCurrentMenu();
		}

		
	}

	programData.ui.menuRenderer.EndMenu();



	programData.ui.menuRenderer.BeginManualMenu("Create world");

	
	if (programData.ui.menuRenderer.internal.allMenuStacks
		[programData.ui.menuRenderer.internal.currentId].size()
		&& programData.ui.menuRenderer.internal.allMenuStacks
		[programData.ui.menuRenderer.internal.currentId].back() == "Create world"
		)
	{

		programData.ui.menuRenderer.temporalViewPort
			= glm::vec4(0, 0, programData.ui.renderer2d.windowW / 2.6f, programData.ui.renderer2d.windowH);

		programData.ui.menuRenderer.Text(loc_CreateNewWorld(), Colors_White);
		

		drawBackground();

		programData.ui.menuRenderer.InputText("Name:", name, sizeof(name),
			Colors_Gray, programData.ui.buttonTexture);

		programData.ui.menuRenderer.InputText("Seed:", seed, sizeof(seed),
			Colors_Gray, programData.ui.buttonTexture);
		
		programData.ui.menuRenderer.toggleOptions("Dificuldade: ", "Paz|Facil|Normal|Dificil", &createDifficulty, true, Colors_White, 0, programData.ui.buttonTexture, Colors_Gray);
		programData.ui.menuRenderer.toggleOptions("Modo: ", "Sobrevivencia|Criativo", &createGamemode, true, Colors_White, 0, programData.ui.buttonTexture, Colors_Gray);
		programData.ui.menuRenderer.toggleOptions("KeepInventory: ", "OFF|ON", &createKeepInventory, true, Colors_White, 0, programData.ui.buttonTexture, Colors_Gray);
		programData.ui.menuRenderer.toggleOptions("Cheats: ", "OFF|ON", &createAllowCheats, true, Colors_White, 0, programData.ui.buttonTexture, Colors_Gray);
		programData.ui.menuRenderer.toggleOptions("PvP: ", "OFF|ON", &createPvp, true, Colors_White, 0, programData.ui.buttonTexture, Colors_Gray);

		std::string finalName = RESOURCES_PATH "worlds/";
		finalName += name;

		if (name[0] == '\0')
		{
			programData.ui.menuRenderer.Button("Please enter a name!", {0.6,0.4,0.4,1},
				programData.ui.buttonTexture);
		}
		else if (std::filesystem::exists(finalName))
		{
			programData.ui.menuRenderer.Button("Name already exists!", {0.6,0.4,0.4,1},
				programData.ui.buttonTexture);
		}
		else
		{
			bool create = 0;
			bool createAndPlay = 0;


			if (programData.ui.menuRenderer.Button("Create!", Colors_Gray,
				programData.ui.buttonTexture))
			{
				create = true;
			}

			if (programData.ui.menuRenderer.Button("Create and play", Colors_Gray,
				programData.ui.buttonTexture))
			{
				createAndPlay = true;
			}

			if (create || createAndPlay)
			{
				std::error_code err;
				bool err2 = 0;
				std::filesystem::create_directory(finalName, err);


				{
					std::ifstream f(RESOURCES_PATH "gameData/worldGenerator/default.wgenerator");
					if (f.is_open())
					{
						std::stringstream buffer;
						buffer << f.rdbuf();
						if (!settings.loadSettings(buffer.str().c_str()))
						{
							err2 = true;
						}
					}

				}

				if (!err && !err2)
				{
					int finalSeed = 0;
					{
						bool isNumeric = true;
						bool hasContent = false;
						for(int i=0;i<(int)sizeof(seed);i++) if(seed[i]!=0){ hasContent=true; if(!isdigit((unsigned char)seed[i])) isNumeric=false; }
						long long computedSeed = 0;
						if(!hasContent){
							computedSeed = time(0);
						}else if(isNumeric){
							long long pow = 1;
							for (int i = sizeof(seed) - 1; i >= 0; i--)
							{
								if (seed[i] != 0)
								{
									computedSeed += (seed[i] - '0') * pow;
									pow *= 10;
								}
							}
							if(computedSeed==0) computedSeed = time(0);
						}else{
							int h = 0;
							for(int i=0;i<(int)sizeof(seed);i++) if(seed[i]!=0) h = h*31 + (unsigned char)seed[i];
							computedSeed = h;
						}
						finalSeed = (int)computedSeed;
						if (finalSeed < 0) { finalSeed = -finalSeed; }
						if (finalSeed == 0) { finalSeed = 1; }
					};

					{
						std::ofstream f(finalName + "/worldGenSettings.wgenerator");
						settings.seed = finalSeed;
						settings.isSuperFlat = (currentIndex == 1);

						settings.sanitize();
						f << settings.saveSettings();

					}
					{
						ServerSettings defCfg;
						defCfg.worldName = name;
						defCfg.allowCheats = createAllowCheats;
						defCfg.pvpEnabled = createPvp;
						defCfg.keepInventory = createKeepInventory;
						if (createDifficulty==0) defCfg.difficulty="peaceful";
						else if (createDifficulty==1) defCfg.difficulty="easy";
						else if (createDifficulty==2) defCfg.difficulty="normal";
						else defCfg.difficulty="hard";
						defCfg.defaultGamemode = createGamemode ? "creative" : "survival";
						auto &launcher = getLauncherState();
						defCfg.worldOwner = launcher.currentUUID.empty()? launcher.currentUsername : launcher.currentUUID;
						saveWorldConfig(name, defCfg);
					}

					if (createAndPlay)
					{
						hostServer(name);
					}
				}

				programData.ui.menuRenderer.ExitCurrentMenu();
			}

		}
		
		
		//programData.ui.menuRenderer.newColum(11);
		//programData.ui.menuRenderer.newColum(12);

		{
			auto &renderer = programData.ui.renderer2d;
			glui::Frame f({0,0, renderer.windowW, renderer.windowH});

			{
				glui::Frame f(glui::Box().xLeftPerc(0.35).yTopPerc(0.1).yDimensionPercentage(0.8).xDimensionPercentage(0.6)());

				{
					auto fullBox = glui::Box().xLeft().yTop().xDimensionPercentage(1.f).yDimensionPercentage(1.f)();
					

					auto buttonBox = glui::Box().xLeft().yBottom().xDimensionPercentage(1.f).yDimensionPixels(150.f)();
					glui::toggleOptions(renderer, buttonBox, "World type: ", Colors_White,
						"Normal|Super Flat", &currentIndex, true, programData.ui.font,
						programData.ui.buttonTexture, Colors_Gray, platform::getRelMousePosition(),
						platform::isLMouseHeld(), platform::isLMouseReleased());

					auto mapBox = glui::Box().xLeft().yTop().xDimensionPercentage(1.f).yDimensionPercentage(1.f)();
					mapBox.w -= buttonBox.w;

					if (mapBox.w > 0)
					{
						renderer.render9Patch(mapBox, 20,
							Colors_Gray, {}, 0.f, programData.ui.buttonTexture, 
							GL2D_DefaultTextureCoords, {0.2,0.8,0.8,0.2});

						//auto textureBox = mapBox;
						//textureBox = shrinkPercentage(textureBox, {0.6f, 0.6f});
						//
						//worldPreviewTexture.cleanup();
						//wg.applySettings(settings);
						//	
						//wg.generateChunkPreview(worldPreviewTexture, {textureBox.z,textureBox.w}, {});
						//
						//renderer.renderRectangle(textureBox, worldPreviewTexture);

					}

				}


			}


		}


	}
	else
	{
		memset(seed, 0, sizeof(seed));
		memset(name, 0, sizeof(name));
		currentIndex = 0;
		createDifficulty = 2;
		createGamemode = 0;
		createKeepInventory = 0;
		createAllowCheats = 0;
		createPvp = 0;
		settings = {};
	}
	
	programData.ui.menuRenderer.EndMenu();


}

void displayWorldConfigMenu(ProgramData &programData, std::string &selectedWorld)
{
	programData.ui.menuRenderer.BeginManualMenu("World Config");
	if (programData.ui.menuRenderer.internal.allMenuStacks[programData.ui.menuRenderer.internal.currentId].size()
		&& programData.ui.menuRenderer.internal.allMenuStacks[programData.ui.menuRenderer.internal.currentId].back() == "World Config")
	{
		if (selectedWorld.empty())
		{
			programData.ui.menuRenderer.Text("Nenhum mundo selecionado", Colors_White);
			if (programData.ui.menuRenderer.Button("Voltar", Colors_Gray, programData.ui.buttonTexture))
				programData.ui.menuRenderer.ExitCurrentMenu();
		}
		else
		{
			static ServerSettings cfg;
			static std::string loadedFor;
			if (loadedFor != selectedWorld)
			{
				cfg = loadWorldConfig(selectedWorld);
				loadedFor = selectedWorld;
			}
			programData.ui.menuRenderer.Text(("Mundo: " + selectedWorld).c_str(), Colors_White);
			programData.ui.menuRenderer.Text("Configuracoes do mundo", Colors_White);
			{
				int cheats = cfg.allowCheats ? 1 : 0;
				programData.ui.menuRenderer.toggleOptions("Permitir cheats: ", "NAO|SIM", &cheats, true, Colors_White, 0, programData.ui.buttonTexture, Colors_Gray);
				cfg.allowCheats = cheats;
				std::string label = cfg.allowCheats ? "Cheats: ATIVADO" : "Cheats: DESATIVADO";
				programData.ui.menuRenderer.Text(label.c_str(), cfg.allowCheats ? glm::vec4(0.3f,1,0.4f,1) : glm::vec4(1,0.4f,0.4f,1));
				programData.ui.menuRenderer.Text(cfg.allowCheats ? "Comandos liberados para todos" : "Apenas OP pode usar comandos", glm::vec4(0.7f,0.7f,0.7f,1));
			}
			int diffIdx = 0;
			if (cfg.difficulty == "peaceful") diffIdx = 0;
			else if (cfg.difficulty == "easy") diffIdx = 1;
			else if (cfg.difficulty == "normal") diffIdx = 2;
			else if (cfg.difficulty == "hard") diffIdx = 3;
			programData.ui.menuRenderer.toggleOptions("Dificuldade: ", "Paz|Facil|Normal|Dificil", &diffIdx, true, Colors_White, 0, programData.ui.buttonTexture, Colors_Gray);
			if (diffIdx == 0) cfg.difficulty = "peaceful";
			else if (diffIdx == 1) cfg.difficulty = "easy";
			else if (diffIdx == 2) cfg.difficulty = "normal";
			else cfg.difficulty = "hard";

			int gmIdx = (cfg.defaultGamemode == "creative") ? 1 : 0;
			programData.ui.menuRenderer.toggleOptions("Modo padrao: ", "Sobrevivencia|Criativo", &gmIdx, true, Colors_White, 0, programData.ui.buttonTexture, Colors_Gray);
			cfg.defaultGamemode = gmIdx ? "creative" : "survival";

			int pvp = cfg.pvpEnabled ? 1 : 0;
			programData.ui.menuRenderer.toggleOptions("PvP: ", "OFF|ON", &pvp, true, Colors_White, 0, programData.ui.buttonTexture, Colors_Gray);
			cfg.pvpEnabled = pvp;

			int keep = cfg.keepInventory ? 1 : 0;
			programData.ui.menuRenderer.toggleOptions("Keep Inventory: ", "OFF|ON", &keep, true, Colors_White, 0, programData.ui.buttonTexture, Colors_Gray);
			cfg.keepInventory = keep;

			int hunger = cfg.hungerEnabled ? 1 : 0;
			programData.ui.menuRenderer.toggleOptions("Fome: ", "OFF|ON", &hunger, true, Colors_White, 0, programData.ui.buttonTexture, Colors_Gray);
			cfg.hungerEnabled = hunger;

			if (programData.ui.menuRenderer.Button("Salvar", Colors_Gray, programData.ui.buttonTexture))
			{
				cfg.worldName = selectedWorld;
				saveWorldConfig(selectedWorld, cfg);
				if (isServerRunning() && getServerSettingsReff().worldName == selectedWorld)
				{
					auto cur = getServerSettingsReff();
					cur.allowCheats = cfg.allowCheats;
					cur.difficulty = cfg.difficulty;
					cur.defaultGamemode = cfg.defaultGamemode;
					cur.pvpEnabled = cfg.pvpEnabled;
					cur.keepInventory = cfg.keepInventory;
					cur.hungerEnabled = cfg.hungerEnabled;
					setServerSettings(cur);
				}
				programData.ui.menuRenderer.ExitCurrentMenu();
			}
			if (programData.ui.menuRenderer.Button("Cancelar", Colors_Gray, programData.ui.buttonTexture))
			{
				loadedFor.clear();
				programData.ui.menuRenderer.ExitCurrentMenu();
			}
			if (programData.ui.menuRenderer.Button("Voltar", Colors_Gray, programData.ui.buttonTexture))
			{
				programData.ui.menuRenderer.ExitCurrentMenu();
			}
		}
	}
	programData.ui.menuRenderer.EndMenu();
}

void displayWorldConfigMenuButton(ProgramData &programData, std::string &selectedWorld)
{
	if (programData.ui.menuRenderer.Button("Configurar mundo", Colors_Gray, programData.ui.buttonTexture))
	{
		if (!selectedWorld.empty())
			programData.ui.menuRenderer.StartManualMenu("World Config");
	}
	displayWorldConfigMenu(programData, selectedWorld);
}

void displayPlayerRolesMenu(ProgramData &programData)
{
	programData.ui.menuRenderer.Text("Gerenciar jogadores", Colors_White);
	programData.ui.menuRenderer.Text("Lista de jogadores online", glm::vec4(0.7f,0.7f,0.9f,1));

	auto getRoleName = [](int lvl)->std::string{
		if (lvl >= 3) return "Operator";
		if (lvl >= 2) return "Moderador";
		return "Jogador";
	};
	auto getRoleColor = [](int lvl)->glm::vec4{
		if (lvl >= 3) return glm::vec4(1,0.84f,0,1);
		if (lvl >= 2) return glm::vec4(0.3f,0.6f,1,1);
		return glm::vec4(0.8f,0.8f,0.8f,1);
	};
	auto getRoleIcon = [](int lvl)->const char*{
		if (lvl >= 3) return "[OP]";
		if (lvl >= 2) return "[MOD]";
		return "[PLY]";
	};
	auto getRoleDesc = [](int lvl)->const char*{
		if (lvl >= 3) return "Operator: acesso total, pode gerenciar mundo, cheats e cargos";
		if (lvl >= 2) return "Moderador: pode moderar jogadores, usar comandos de moderacao";
		return "Jogador: acesso basico, sem comandos admin";
	};

	int localLevel = 1;
	bool isOwner = false;
	if (isServerRunning())
	{
		auto &clients = getAllClientsReff();
		if (!clients.empty())
		{
			localLevel = clients.begin()->second.playerData.otherPlayerSettings.commandPermisionLevel;
			isOwner = (localLevel >= 3);
		}
	}
	else
	{
		localLevel = 3;
		isOwner = true;
	}

	if (isServerRunning())
	{
		auto &clients = getAllClientsReff();
		if (clients.empty())
		{
			programData.ui.menuRenderer.Text("Nenhum jogador online", glm::vec4(0.8f,0.8f,0.8f,1));
			auto &launcher = getLauncherState();
			glm::vec4 roleCol = getRoleColor(3);
			programData.ui.menuRenderer.Text((std::string(getRoleIcon(3)) + " " + launcher.currentUsername + " - " + getRoleName(3)).c_str(), roleCol);
			programData.ui.menuRenderer.Text(getRoleDesc(3), glm::vec4(0.6f,0.6f,0.6f,1));
		}
		for (auto &kv : clients)
		{
			std::uint64_t cid = kv.first;
			Client &cl = kv.second;
			int lvl = cl.playerData.otherPlayerSettings.commandPermisionLevel;
			std::string name = "Player " + std::to_string(cid);
			if (cid == clients.begin()->first)
			{
				auto &launcher = getLauncherState();
				if (!launcher.currentUsername.empty()) name = launcher.currentUsername;
			}
			glm::vec4 col = getRoleColor(lvl);
			std::string line = std::string(getRoleIcon(lvl)) + " " + name + " - " + getRoleName(lvl);
			programData.ui.menuRenderer.Text(line.c_str(), col);
			programData.ui.menuRenderer.Text(getRoleDesc(lvl), glm::vec4(0.55f,0.55f,0.55f,1));
			{
				glm::vec4 widgetPos = {};
				bool hovered = false, clicked = false;
				if (programData.ui.menuRenderer.CustomWidget((int)cid, &widgetPos, &hovered, &clicked))
				{
					if (hovered)
					{
						programData.ui.renderer2d.renderRectangle(widgetPos, {1,1,0.2f,0.15f});
					}
				}
				if (hovered)
				{
					programData.ui.menuRenderer.Text(("Hover: " + std::string(getRoleDesc(lvl))).c_str(), glm::vec4(1,1,0.5f,1));
				}
			}
			if (isOwner)
			{
				std::string btnOp = "Tornar OP##" + std::to_string(cid);
				std::string btnMod = "Tornar MOD##" + std::to_string(cid);
				std::string btnPly = "Tornar Jogador##" + std::to_string(cid);
				if (lvl != 3)
				{
					if (programData.ui.menuRenderer.Button(btnOp.c_str(), glm::vec4(1,0.84f,0,1), programData.ui.buttonTexture))
					{
						cl.playerData.otherPlayerSettings.commandPermisionLevel = 3;
						executeServerCommand(cid, "op");
					}
				}
				if (lvl != 2)
				{
					if (programData.ui.menuRenderer.Button(btnMod.c_str(), glm::vec4(0.3f,0.6f,1,1), programData.ui.buttonTexture))
					{
						cl.playerData.otherPlayerSettings.commandPermisionLevel = 2;
					}
				}
				if (lvl != 1)
				{
					if (programData.ui.menuRenderer.Button(btnPly.c_str(), Colors_Gray, programData.ui.buttonTexture))
					{
						cl.playerData.otherPlayerSettings.commandPermisionLevel = 1;
					}
				}
			}
			programData.ui.menuRenderer.Text("", Colors_White);
		}
	}
	else
	{
		auto &launcher = getLauncherState();
		std::string name = launcher.currentUsername.empty() ? "Player" : launcher.currentUsername;
		programData.ui.menuRenderer.Text((std::string("[OP] ") + name + " - Operator (Singleplayer)").c_str(), glm::vec4(1,0.84f,0,1));
		programData.ui.menuRenderer.Text("Dono do mundo singleplayer tem todas as permissoes", glm::vec4(0.6f,0.6f,0.6f,1));
	}
	if (!isOwner)
	{
		programData.ui.menuRenderer.Text("Apenas Operator pode gerenciar cargos", glm::vec4(1,0.5f,0.5f,1));
	}
}

void displayPlayerRolesMenuButton(ProgramData &programData)
{
	programData.ui.menuRenderer.BeginMenu("Cargos", Colors_Gray, programData.ui.buttonTexture);
	displayPlayerRolesMenu(programData);
	programData.ui.menuRenderer.EndMenu();
}

void displayWorldSettingsMenu(ProgramData &programData)
{
	programData.ui.menuRenderer.Text("Configuracoes do mundo (Owner)", Colors_White);
	if (!isServerRunning())
	{
		programData.ui.menuRenderer.Text("Mundo nao esta rodando", glm::vec4(0.8f,0.3f,0.3f,1));
		return;
	}
	auto &s = getServerSettingsReff();
	bool isOwner = false;
	{
		auto &clients = getAllClientsReff();
		if (!clients.empty())
		{
			int lvl = clients.begin()->second.playerData.otherPlayerSettings.commandPermisionLevel;
			isOwner = (lvl >= 3);
		}
		else isOwner = true;
	}
	if (!isOwner)
	{
		programData.ui.menuRenderer.Text("Apenas o dono (Operator) pode alterar", glm::vec4(1,0.5f,0.5f,1));
		programData.ui.menuRenderer.Text(("Cheats: " + std::string(s.allowCheats?"ATIVADO":"DESATIVADO")).c_str(), s.allowCheats? glm::vec4(0.3f,1,0.4f,1): glm::vec4(1,0.4f,0.4f,1));
		return;
	}
	{
		int v = s.allowCheats ? 1 : 0;
		programData.ui.menuRenderer.toggleOptions("Cheats: ", "OFF|ON", &v, true, s.allowCheats? glm::vec4(0.3f,1,0.4f,1): glm::vec4(1,0.4f,0.4f,1), 0, programData.ui.buttonTexture, Colors_Gray);
		bool newVal = v;
		if (newVal != s.allowCheats)
		{
			s.allowCheats = newVal;
			saveWorldConfig(s.worldName, s);
		}
		programData.ui.menuRenderer.Text(s.allowCheats ? "Cheats liberados: qualquer jogador pode usar comandos" : "Cheats bloqueados: apenas OP", glm::vec4(0.7f,0.7f,0.7f,1));
	}
	{
		int pvp = s.pvpEnabled ? 1 : 0;
		programData.ui.menuRenderer.toggleOptions("PvP: ", "OFF|ON", &pvp, true, Colors_White, 0, programData.ui.buttonTexture, Colors_Gray);
		if ((bool)pvp != s.pvpEnabled){ s.pvpEnabled = pvp; saveWorldConfig(s.worldName, s);}
	}
	{
		int keep = s.keepInventory ? 1 : 0;
		programData.ui.menuRenderer.toggleOptions("Keep Inventory: ", "OFF|ON", &keep, true, Colors_White, 0, programData.ui.buttonTexture, Colors_Gray);
		if ((bool)keep != s.keepInventory){ s.keepInventory = keep; saveWorldConfig(s.worldName, s);}
	}
	{
		int hunger = s.hungerEnabled ? 1 : 0;
		programData.ui.menuRenderer.toggleOptions("Fome: ", "OFF|ON", &hunger, true, Colors_White, 0, programData.ui.buttonTexture, Colors_Gray);
		if ((bool)hunger != s.hungerEnabled){ s.hungerEnabled = hunger; saveWorldConfig(s.worldName, s);}
	}
	{
		int diffIdx = 0;
		if (s.difficulty == "peaceful") diffIdx=0; else if (s.difficulty=="easy") diffIdx=1; else if (s.difficulty=="normal") diffIdx=2; else diffIdx=3;
		programData.ui.menuRenderer.toggleOptions("Dificuldade: ", "Paz|Facil|Normal|Dificil", &diffIdx, true, Colors_White, 0, programData.ui.buttonTexture, Colors_Gray);
		std::string ndiff = s.difficulty;
		if (diffIdx==0) ndiff="peaceful"; else if (diffIdx==1) ndiff="easy"; else if (diffIdx==2) ndiff="normal"; else ndiff="hard";
		if (ndiff != s.difficulty){ s.difficulty = ndiff; saveWorldConfig(s.worldName, s);}
	}
	programData.ui.menuRenderer.Text(("Mundo: " + s.worldName).c_str(), glm::vec4(0.7f,0.7f,0.9f,1));
}

void displayWorldSettingsMenuButton(ProgramData &programData)
{
	programData.ui.menuRenderer.BeginMenu("Mundo", Colors_Gray, programData.ui.buttonTexture);
	displayWorldSettingsMenu(programData);
	programData.ui.menuRenderer.EndMenu();
}

void ShadingSettings::normalize()
{

	viewDistance = glm::clamp(viewDistance, 1, 32);
	tonemapper = glm::clamp(tonemapper, 0, 4);
	shadows = glm::clamp(shadows, 0, 2);
	waterType = glm::clamp(waterType, 0, 1);

	waterColor = glm::clamp(waterColor, glm::vec3(0.f), glm::vec3(2.f, 2.f, 2.f));
	underWaterColor = glm::clamp(underWaterColor, glm::vec3(0.f), glm::vec3(2.f, 2.f, 2.f));

	underwaterDarkenStrength = glm::clamp(underwaterDarkenStrength, 0.f, 1.f);
	underwaterDarkenDistance = glm::clamp(underwaterDarkenDistance, 0.f, 40.f);
	fogGradientUnderWater = glm::clamp(fogGradientUnderWater, 0.f, 32.f);
	workerThreadsForBaking = glm::clamp(workerThreadsForBaking, 0, 10);
	lodStrength = glm::clamp(lodStrength, 0, 5);

	bloomTresshold = glm::clamp(bloomTresshold, 0.1f, 1.f);
	bloomMultiplier = glm::clamp(bloomMultiplier, 0.0f, 1.f);
	
	PBR = glm::clamp(PBR, 0, 1);
	bloom = glm::clamp(bloom, 0, 1);
	SSR = glm::clamp(SSR, 0, 1);
	useLights = glm::clamp(useLights, 0, 1);
	lightsStrength = glm::clamp(lightsStrength, 0.1f, 2.f);

	maxLights = glm::clamp(maxLights, 1,100);

	exposure = glm::clamp(exposure, -2.f, 2.f);
	fogGradient = glm::clamp(fogGradient, 0.f, 100.f);


	toneMapSaturation = glm::clamp(toneMapSaturation, 0.f, 2.f);
	toneMapVibrance = glm::clamp(toneMapVibrance, 0.f, 2.f);
	toneMapGamma = glm::clamp(toneMapGamma, 0.1f, 2.f);
	toneMapShadowBoost = glm::clamp(toneMapShadowBoost, -1.f, 1.f);
	toneMapHighlightBoost = glm::clamp(toneMapHighlightBoost, -1.f, 1.f);
	vignette = glm::clamp(vignette, 0.f, 1.f);
	glm::vec3 toneMapLift = glm::clamp(toneMapLift ,glm::vec3(0.f), glm::vec3(1));
	glm::vec3 toneMapGain = glm::clamp(toneMapGain, glm::vec3(0.f), glm::vec3(1));

	frameGeneration = glm::clamp(frameGeneration, 0, 1);
	frameGenerationMode = glm::clamp(frameGenerationMode, 0, 1);
	nvidiaBoost = glm::clamp(nvidiaBoost, 0, 1);
	msaa = glm::clamp(msaa, 0, 8);
	if(msaa!=0 && msaa!=2 && msaa!=4 && msaa!=8) msaa=0;
	vsyncMode = glm::clamp(vsyncMode, 0, 2);
	fsr = glm::clamp(fsr, 0, 4);
	amdAntiLag = glm::clamp(amdAntiLag, 0, 1);
	anisotropy = glm::clamp(anisotropy, 1, 16);

}

#define GET_STR(x) "c_" #x

#define ADD_TO_RESULT(x) result += ("#define " GET_STR(x) " ") + std::to_string(x) + "\n"
#define ADD_TO_RESULT_VEC3(x) result += ("#define " GET_STR(x) " vec3(") + std::to_string(x.r) + "," + std::to_string(x.g) + "," + std::to_string(x.b) + ")\n"


std::string ShadingSettings::formatIntoGLSLcode()
{
	normalize();

	std::string result;
	result.reserve(500);


	ADD_TO_RESULT(shadows);
	ADD_TO_RESULT(PBR);
	ADD_TO_RESULT(SSR);
	ADD_TO_RESULT(useLights);



	return result;
}
