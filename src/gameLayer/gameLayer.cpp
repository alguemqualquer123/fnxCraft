#include "gameLayer.h"
#include "gl2d/gl2d.h"
#include "platformInput.h"
#include <iostream>
#include <sstream>
#include "rendering/camera.h"
#include "errorReporting.h"
#include "rendering/renderer.h"
#include "chunkSystem.h"
#include "threadStuff.h"
#include <thread>
#include <ctime>
#include "multyPlayer/server.h"
#include "multyPlayer/createConnection.h"
#include "multyPlayer/enetServerFunction.h"
#include <enet/enet.h>
#include <filesystem>
#include <fstream>
#include "scripting/EventBus.h"
#include "rendering/UiEngine.h"
#include "glui/glui.h"
#include "gamePlayLogic.h"
#include "multyPlayer/splitUpdatesLogic.h"
#include <rendering/renderSettings.h>
#include <ourJson.h>
#include <filesystem>
#include <audioEngine.h>
#include <gameplay/loot.h>
#include <localization.h>

#include <platformTools.h>
#include "scripting/ScriptingIntegration.h"
#include <gameLayer/SplashScreen.h>
#include <gameLayer/Launcher.h>
#include <gameLayer/persistence/SaveSystem.h>
#include "gameLayerHelpers.h"

#if REMOVE_IMGUI == 0
#include "imgui.h"
#endif


ProgramData programData;


inline void stubErrorFunc(const char *msg, void *userDefinedData)
{}

void clearOtherTextures()
{
	programData.numbersTexture.cleanup();
	programData.dudv.cleanup();
	programData.causticsTexture.cleanup();
	programData.dudvNormal.cleanup();
	programData.aoTexture.cleanup();
	programData.brdfTexture.cleanup();
	programData.crackTexture.cleanup();
	programData.lensDirtTexture.cleanup();
	programData.hitDirtTexture.cleanup();
	programData.waterDirtTexture.cleanup();
	programData.heartsTexture.cleanup();
	programData.heartsAtlas = {};
	programData.hungerTexture.cleanup();
	programData.hungerAtlas = {};
	programData.thirstTexture.cleanup();
	programData.thirstAtlas = {};
	programData.armorTexture.cleanup();
	programData.armorAtlas = {};

	for (auto &t : programData.lensFlare)
	{
		t.cleanup();
	}
	programData.lensFlare = {};

}

void clearAllTexturePacks()
{
	clearOtherTextures();
	programData.skyBoxLoaderAndDrawer.clearOnlyTextures();
	programData.ui.clearOnlyTextures();
	programData.modelsManager.clearAllModels();
	programData.blocksLoader.clearAllTextures();
}

void loadOtherTextures(const char *basePath)
{


	std::string p(basePath);

	if (!programData.numbersTexture.id)
	{
		programData.numbersTexture.loadFromFile((p + "numbers.png").c_str(), true, true);
		//programData.dudv.loadFromFile(RESOURCES_PATH "assets/otherTextures/test.jpg", true, true);
	};

	if (!programData.dudv.id)
	{
		programData.dudv.loadFromFile((p + "waterDUDV.png").c_str(), false, true);

		if (!programData.dudv.id)
		{
			programData.dudv.loadFromFile((p + "waterDUDV.jpg").c_str(), false, true);
		}

		//programData.dudv.loadFromFile(RESOURCES_PATH "assets/otherTextures/wdudv.jpg", false, true);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	};

	if (!programData.causticsTexture.id)
	{
		programData.causticsTexture.loadFromFile((p + "caustics.png").c_str(), false, true);

		if (!programData.causticsTexture.id)
		{
			programData.causticsTexture.loadFromFile((p + "caustics.jpg").c_str(), false, true);
		}

		//programData.causticsTexture.loadFromFile(RESOURCES_PATH "assets/otherTextures/caustics3.png", false, true);
		//programData.causticsTexture.loadFromFile(RESOURCES_PATH "assets/otherTextures/test.jpg", false, true);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	};

	if (!programData.dudvNormal.id)
	{
		//programData.dudvNormal.loadFromFile(RESOURCES_PATH "assets/otherTextures/normal.png", false, true);
		//programData.dudvNormal.loadFromFile(RESOURCES_PATH "assets/otherTextures/normal2.png", false, true);
		//programData.dudvNormal.loadFromFile(RESOURCES_PATH "assets/otherTextures/normal.jpg", false, true);
		//programData.dudvNormal.loadFromFile(RESOURCES_PATH "assets/otherTextures/normal2.jpg", false, true);
		programData.dudvNormal.loadFromFile((p+"normal.png").c_str(), false, true); //best

		if (!programData.dudvNormal.id)
		{
			programData.dudvNormal.loadFromFile((p + "normal.jpg").c_str(), false, true); //best
		}


		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	};


	if (!programData.aoTexture.id)
	{
		programData.aoTexture.loadFromFile((p+"ao.png").c_str(), false, true);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	};

	if (!programData.brdfTexture.id)
	{
		programData.brdfTexture.loadFromFile((p + "brdf.png").c_str(), false, false);
	}

	if (!programData.crackTexture.id)
	{
		programData.crackTexture.loadFromFile((p + "crack.png").c_str(), true, true);
	}

	if (!programData.lensDirtTexture.id)
	{
		programData.lensDirtTexture.loadFromFile((p + "lensDirt.png").c_str(), true, true);
	}

	if (!programData.hitDirtTexture.id)
	{
		programData.hitDirtTexture.loadFromFile((p + "hitLensDirt.png").c_str(), true, true);
	}

	if (!programData.waterDirtTexture.id)
	{
		programData.waterDirtTexture.loadFromFile((p + "waterLensDirt.png").c_str(), true, true);
	}

	if (!programData.heartsTexture.id)
	{
		//todo better api here
		programData.heartsTexture.loadFromFileWithPixelPadding((p + "hearts.png").c_str(), 
			9, true, false);
		auto s = programData.heartsTexture.GetSize();
		programData.heartsAtlas = gl2d::TextureAtlasPadding(5, 1, s.x, s.y);
	}
	if (!programData.hungerTexture.id)
	{
		programData.hungerTexture.loadFromFileWithPixelPadding((p + "hunger.png").c_str(), 9, true, false);
		auto s = programData.hungerTexture.GetSize();
		programData.hungerAtlas = gl2d::TextureAtlasPadding(5, 1, s.x, s.y);
		if(!programData.hungerTexture.id){
			programData.hungerTexture = programData.heartsTexture;
			programData.hungerAtlas = programData.heartsAtlas;
		}
	}
	if (!programData.thirstTexture.id)
	{
		programData.thirstTexture.loadFromFileWithPixelPadding((p + "thirst.png").c_str(), 9, true, false);
		auto s = programData.thirstTexture.GetSize();
		programData.thirstAtlas = gl2d::TextureAtlasPadding(5, 1, s.x, s.y);
		if(!programData.thirstTexture.id){
			programData.thirstTexture = programData.heartsTexture;
			programData.thirstAtlas = programData.heartsAtlas;
		}
	}
	if (!programData.armorTexture.id)
	{
		programData.armorTexture.loadFromFileWithPixelPadding((p + "armor.png").c_str(), 9, true, false);
		auto s = programData.armorTexture.GetSize();
		programData.armorAtlas = gl2d::TextureAtlasPadding(5, 1, s.x, s.y);
		if(!programData.armorTexture.id){
			programData.armorTexture = programData.heartsTexture;
			programData.armorAtlas = programData.heartsAtlas;
		}
	}

	if (programData.lensFlare.empty())
	{

		auto path = RESOURCES_PATH "assets/otherTextures/lensFlare";
		programData.maxFlareSize = 0;
		
		if (std::filesystem::exists(path))
		{

			for (auto &f : std::filesystem::directory_iterator(path))
			{
				gl2d::Texture t = {};
				t.loadFromFile(f.path().string().c_str());

				if (t.id)
				{
					programData.lensFlare.push_back(t);

					programData.maxFlareSize = std::max(programData.maxFlareSize, (float)t.GetSize().x);
				}


			}

		}

	}

}

void loadAllDefaultTexturePacks()
{
	loadOtherTextures(RESOURCES_PATH "assets/otherTextures/"); //load the defaults

	programData.skyBoxLoaderAndDrawer.loadAllTextures(RESOURCES_PATH "assets/sky/");

	programData.ui.loadTextures(RESOURCES_PATH "assets/ui/");

	programData.modelsManager.loadAllModels(RESOURCES_PATH "assets/models/", false);

	programData.blocksLoader.loadAllTextures(RESOURCES_PATH "assets/", true);
	programData.blocksLoader.loadAllItemsGeometry();
	programData.blocksLoader.setupAllColors();

	programData.renderer.recreateBlocksTexturesBuffer(programData.blocksLoader);

	//todo remove? repeating? it seems like i also call it down
	programData.renderer.renderAllBlocksUiTextures(programData.blocksLoader, programData.modelsManager);

	

	//programData.blocksLoader.clearAllTextures();
	//
	//programData.blocksLoader.loadAllTextures(RESOURCES_PATH "assets/");
	//programData.renderer.recreateBlocksTexturesBuffer(programData.blocksLoader);



}

bool loadTexturePack(const char *basePath)
{

	std::filesystem::path root(basePath);

	if (std::filesystem::exists(root) &&
		std::filesystem::is_directory(root))
	{
		gl2d::setErrorFuncCallback(stubErrorFunc);


		std::filesystem::path otherTexturesPath = root;
		otherTexturesPath /= "otherTextures/";

		if (std::filesystem::exists(otherTexturesPath) &&
			std::filesystem::is_directory(otherTexturesPath))
		{

			loadOtherTextures(otherTexturesPath.string().c_str());
		}


		std::filesystem::path skyPath = root;
		skyPath /= "sky/";

		if (std::filesystem::exists(skyPath) &&
			std::filesystem::is_directory(skyPath))
		{
			programData.skyBoxLoaderAndDrawer.loadAllTextures(skyPath.string().c_str());
		}

		std::filesystem::path uiPath = root;
		uiPath /= "ui/";

		if (std::filesystem::exists(uiPath) &&
			std::filesystem::is_directory(uiPath))
		{
			programData.ui.loadTextures(uiPath.string().c_str());
		}

		std::filesystem::path modelsPath = root;
		modelsPath /= "models/";

		if (std::filesystem::exists(modelsPath) &&
			std::filesystem::is_directory(modelsPath))
		{
			programData.modelsManager.loadAllModels(modelsPath.string().c_str(), false);
		}


		std::filesystem::path blocksPath = root / "blocks/";
		std::filesystem::path items = root / "items/";


		if (std::filesystem::is_directory(blocksPath) || std::filesystem::is_directory(items))
		{
			programData.blocksLoader.loadAllTextures(root.string() + "/", false);
		}


		gl2d::setErrorFuncCallback(gl2d::defaultErrorFunc);


	}
	else
	{
		return 0;
	}

	return true;
}

static ShadingSettings shadingSettingsCopy;


bool initGame() //main server and title screen stuff
{
	SplashScreen::draw(0.02f, "Inicializando...", "Preparando arquivos");

	srand(time(0));
	createErrorFile();
	ensureAllDataDirectories();
	SaveSystem::get().init("world");
	// Launcher state must be initialized before use

	std::filesystem::create_directory(RESOURCES_PATH "../playerSettings/");


	programData.GPUProfiler.initGPUProfiler();
	SplashScreen::draw(0.08f, "Inicializando GPU...", "Profiler e VSync");

	gl2d::setVsync(false);

	AudioEngine::init();
	AudioEngine::loadSettingsOrSetToDefaultIfFail();
	SplashScreen::draw(0.15f, "Carregando audio...", "Engine de som");

	loadLanguageSettings();
	SplashScreen::draw(0.20f, "Idioma...", "Carregando traducoes");

	loadShadingSettings();
	{
		auto &s = programData.renderer.defaultShader.shadingSettings;
		auto &loadedS = getShadingSettings();

		s.exposure = loadedS.exposure;
		s.fogGradientUnderWater = loadedS.fogGradientUnderWater;
		s.tonemapper = loadedS.tonemapper;
		s.underWaterColor = loadedS.underWaterColor;
		s.underwaterDarkenDistance = loadedS.underwaterDarkenDistance;
		s.underwaterDarkenStrength = loadedS.underwaterDarkenStrength;
		s.waterColor = loadedS.waterColor;

	}
	shadingSettingsCopy = getShadingSettings();
	SplashScreen::draw(0.28f, "Interface...", "Preparando UI");

	programData.ui.init();
	SplashScreen::draw(0.35f, "Renderizador...", "Gyzmos e Skybox");

	programData.gyzmosRenderer.create();
	programData.pointDebugRenderer.create();
	programData.skyBoxLoaderAndDrawer.createGpuData();
	programData.sunRenderer.create();
	programData.weatherRenderer.init();
	programData.weatherRenderer.loadShaders();
	SplashScreen::draw(0.45f, "Texturas...", "Pacotes e blocos");

	loadAllDefaultTexturePacks();
	programData.renderer.create(programData.modelsManager);
	programData.renderer.renderAllBlocksUiTextures(programData.blocksLoader, programData.modelsManager);

	{
		int missingBlocks = 0, missingItems = 0;
		if(programData.blocksLoader.texturesIds.size() >= (size_t)BlocksCount*4 && !programData.blocksLoader.texturesIds.empty()){
			for (int i = 0; i < BlocksCount; i++)
				if (programData.blocksLoader.texturesIds[i*4] == programData.blocksLoader.texturesIds[0]) missingBlocks++;
		}else{
			std::cout<<"[AssetValidator] texturesIds size "<<programData.blocksLoader.texturesIds.size()<<" < "<<BlocksCount*4<<" - skipping check\n";
		}
		if(programData.blocksLoader.texturesIdsItems.size() >= (size_t)(ItemTypes::lastItem - ItemsStartPoint) && !programData.blocksLoader.texturesIdsItems.empty()){
			for (int i = 0; i < (int)(ItemTypes::lastItem - ItemsStartPoint); i++)
				if (programData.blocksLoader.texturesIdsItems[i] == programData.blocksLoader.texturesIds[0]) missingItems++;
		}
		if (missingBlocks > 1 || missingItems > 0)
			std::cout << "[AssetValidator] Missing textures — blocks: " << missingBlocks << " (incl. air), items: " << missingItems << " (checker fallback)\n";
	}
	SplashScreen::draw(0.70f, "Modelos 3D...", "Carregando assets");


	AudioEngine::loadAllMusic();
	SplashScreen::draw(0.80f, "Musicas...", "Trilha sonora");
	AudioEngine::playTitleMusic();

	programData.defaultCover.loadFromFile(RESOURCES_PATH "defaultCover.png");
	SplashScreen::draw(0.88f, "Rede...", "Inicializando ENet");


	if (enet_initialize() != 0)
	{
		reportError("problem starting ENET");
		return false;
	}

	SplashScreen::draw(0.92f, "Scripts Lua...", "Carregando resources");
	if(!Scripting::init()){
		std::cout<<"[Lua] Aviso: scripting desabilitado\n";
	}
	
	//programData.facesCount = blockData.size() / 4;

	//glNamedBufferData(programData.renderer.vertexBuffer, 0, 0, GL_DYNAMIC_DRAW);

	//glEnable(GL_LINE_WIDTH);
	glLineWidth(4);
	SplashScreen::draw(0.98f, "Quase la...", "Finalizando");


	//KeyValuePair settings;
	//settings.loadElementsFromFile(RESOURCES_PATH "test.txt");
	//settings.printAll();
	//settings.writeIntoFile(RESOURCES_PATH "test2.txt");


#pragma region checks
	//todo option to remove in a production build!
	{

		for (int i = 1; i < BlocksCount; i++)
		{

			getBlockBaseMineDuration(i);

		}

	}
#pragma endregion


#pragma region tests

	//loot tests
	if(0)
	{
		std::minstd_rand rng;

		LootEntry test;
		test.item = itemCreator(cloth);

		std::vector<LootEntry> loot;
		loot.push_back(test);
		//loot.push_back(test);
		//loot.push_back(test);

		test.item = itemCreator(fang);
		test.chance = 10;
		loot.push_back(test);

		int tries = 1000;
		int success = 0;
		for (int i = 0; i < tries; i++)
		{

			auto item = drawLoot(loot, rng, 100.f);

			if (item.type == fang)
			{
				success++;
			}

		}

		std::cout << "!!!!!!!Percent chance: " << (float(success) / tries) * 100 << "%\n";
		std::cout << "!!!!!!!Get one every: " << (float(tries) / success) << "tries\n";

		long long rezult = 0;
		long long smal = 0;
		long long big = 0;
		for (int i = 0; i < tries; i++)
		{

			rezult += getRandomLootNumber(0, 100, rng, 0);
			smal += getRandomLootNumber(0, 100, rng, -50);
			big += getRandomLootNumber(0, 100, rng, 50);

		}

		std::cout << "!!!!!!!getRandomLootNumber avg: " << (float(rezult) / tries) << "\n";
		std::cout << "!!!!!!!getRandomLootNumber avg small: " << (float(smal) / tries) << "\n";
		std::cout << "!!!!!!!getRandomLootNumber avg big: " << (float(big) / tries) << "\n";

	}


#pragma endregion

	return true;
}

static bool gameStarted = 0;
void renderLauncherUI(ProgramData &pd)
{
	auto &launcher = getLauncherState();
	static char usernameBuf[64] = "Player";
	static bool init = false;
	if (!init)
	{
		if (!launcher.currentUsername.empty())
			snprintf(usernameBuf, sizeof(usernameBuf), "%s", launcher.currentUsername.c_str());
		init = true;
	}
	pd.ui.menuRenderer.Text("Bem-vindo ao fnxCraft!", Colors_White);
	pd.ui.menuRenderer.Text("Digite seu nome para entrar:", Colors_White);
	pd.ui.menuRenderer.InputText("Nome", usernameBuf, sizeof(usernameBuf), Colors_Gray, pd.ui.buttonTexture);
	if (pd.ui.menuRenderer.Button("Entrar", Colors_Gray, pd.ui.buttonTexture))
	{
		std::string name = usernameBuf;
		size_t s = name.find_first_not_of(" \t\r\n");
		size_t e = name.find_last_not_of(" \t\r\n");
		if (s == std::string::npos) name.clear();
		else name = name.substr(s, e - s + 1);
		if (name.empty()) name = "Player";
		launcher.currentUsername = name;
		launcher.currentUUID = name + "-offline";
		launcher.loggedIn = true;
		launcher.showLauncher = false;
	}
	if (pd.ui.menuRenderer.Button("Jogar como Convidado", Colors_Gray, pd.ui.buttonTexture))
	{
		launcher.currentUsername = "Player";
		launcher.currentUUID = "offline-player";
		launcher.loggedIn = true;
		launcher.showLauncher = false;
	}
}

static std::string lastError = "";
static ConfirmationModal exitModal;
bool hostServer(const std::string &path)
{
	SplashScreen::draw(0.5f, "Conectando ao servidor local...", "Preparando mundo");

	if (!startServer(path))
	{
		lastError = "Problem starting server";
	}
	else
	{
		gameStarted = true;
		SplashScreen::draw(0.7f, "Conectando ao servidor local...", "Entrando no mundo");
		if (!initGameplay(programData, nullptr))
		{
			closeServer();
			lastError = "Coultn't join, closing server";
			gameStarted = false;
		}
		else
		{
			return true;
		}
	}

	return false;
}

bool gameLogic(float deltaTime)
{
	Scripting::update(deltaTime);

#pragma region init stuff
	int w = 0; int h = 0;
	w = platform::getWindowSizeX();
	h = platform::getWindowSizeY();

	programData.ui.renderer2d.updateWindowMetrics(w, h);


	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

#pragma endregion

#pragma region fps
	static float timeCounter = 0;
	static int frameCounter = 0;

	timeCounter += deltaTime;
	if(timeCounter >= 1.f)
	{
		timeCounter -= 1;
		programData.currentFps = frameCounter;
		frameCounter = 0;
	}
	frameCounter++;
	static float autosaveTimer=0;
	autosaveTimer+=deltaTime;
	if(autosaveTimer>10.f){
		autosaveTimer=0;
		SaveSystem::get().autoSave();
		if(isServerRunning()){
			auto &s = getServerSettingsReff();
			saveWorldConfig(s.worldName, s);
			SaveSystem::get().saveWorld(s.worldName);
			auto &clients = getAllClientsReff();
			for(auto &kv: clients){
				std::string pid = std::to_string(kv.first);
				std::string pdir = std::string(RESOURCES_PATH) + "worlds/" + s.worldName + "/players";
				std::filesystem::create_directories(pdir);
				std::string pfile = pdir + "/" + pid + ".json";
				std::ofstream f(pfile);
				if(f.is_open()){
					auto &pos = kv.second.playerData.entity.position;
					f << "{\n  \"uuid\":\"" << pid << "\",\n  \"x\":" << pos.x << ",\n  \"y\":" << pos.y << ",\n  \"z\":" << pos.z << ",\n  \"skin\":\"" << getSkinName() << "\"\n}\n";
				}
				std::string invFile = pdir + "/" + pid + ".inv";
				std::vector<unsigned char> data;
				kv.second.playerData.inventory.formatIntoData(data);
				std::ofstream invF(invFile, std::ios::binary);
				if(invF.is_open() && !data.empty()) invF.write((char*)data.data(), data.size());
			}
		}
		auto &ls = getLauncherState();
		if(ls.loggedIn){
			PlayerSaveData pd; pd.username=ls.currentUsername; pd.uuid=ls.currentUUID;
			SaveSystem::get().savePlayer(pd.uuid, pd);
		}
		std::cout << "[AutoSave] mundo, inventario, posicoes, mobs e blocos salvos (10s)\n";
	}

#pragma endregion
	
#pragma region music
	AudioEngine::update();
#pragma endregion

	if (shouldReloadTexturePacks())
	{
		clearAllTexturePacks();

		auto tp = getUsedTexturePacksAndResetFlag();

		for (auto &t : tp)
		{
			std::string path = RESOURCES_PATH;
			path += "/texturePacks/";
			path += t.filename().string();
			loadTexturePack(path.c_str());
		}

		loadAllDefaultTexturePacks();
	}

	if (platform::isKeyPressedOn(platform::Button::F11))
	{
		platform::setFullScreen(!platform::isFullScreen());
	}
	if (platform::isKeyPressedOn(platform::Button::F12))
	{
		bool html = gHtmlUi.toggle();
		std::cout << "[UI] Toggle para " << (html ? "HTML/CSS/JS" : "ImGui/glui") << " (F12)\n";
	}


	static char ipString[50] = {};

	if (!gameStarted)
	{
		programData.ui.menuRenderer.Begin(1);
		programData.ui.menuRenderer.SetAlignModeFixedSizeWidgets({0,150});

		//if (programData.ui.menuRenderer.internal.allMenuStacks[programData.ui.menuRenderer.internal.currentId].size() == 0)
		{
			programData.ui.renderer2d.renderRectangle({0,0,
				programData.ui.renderer2d.windowW, programData.ui.renderer2d.windowH},
				programData.ui.background, Colors_White, {}, 0,
				glui::calculateInnerTextureCoords(
				{programData.ui.renderer2d.windowW, programData.ui.renderer2d.windowH}, programData.ui.background)
				);

			programData.ui.renderer2d.renderRectangle({0,0,
			programData.ui.renderer2d.windowW, programData.ui.renderer2d.windowH},
				programData.ui.vignete, {1,1,1,0.8}, {}, 0,
				glui::calculateInnerTextureCoords(
				{programData.ui.renderer2d.windowW, programData.ui.renderer2d.windowH}, programData.ui.vignete)
			);

		}

		//if (programData.ui.menuRenderer.Button("Host game", Colors_Gray, programData.ui.buttonTexture))
		//{
		
		//}

		auto &launcher = getLauncherState();
		if(launcher.showLauncher || !launcher.loggedIn){
			renderLauncherUI(programData);
		}else{
			programData.ui.menuRenderer.Text(("Logado como: "+launcher.currentUsername).c_str(), glm::vec4(0.4f,1,0.7f,1));
			if(programData.ui.menuRenderer.Button("Trocar conta", Colors_Gray, programData.ui.buttonTexture)){
				launcher.logout();
			}
			displayWorldSelectorMenuButton(programData);
			if (programData.ui.menuRenderer.Button(loc_JoinGame(), Colors_Gray,
				programData.ui.buttonTexture))
			{
				std::string trimmed = ipString;
				trimmed.erase(0, trimmed.find_first_not_of(" \t\r\n"));
				trimmed.erase(trimmed.find_last_not_of(" \t\r\n") + 1);
				if (!trimmed.empty() && trimmed.find_first_not_of("0123456789.: \t\r\nabcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ-_") != std::string::npos)
				{
					lastError = "IP invalido / Invalid IP";
				}
				else
				{
					std::string connectMsg = trimmed.empty() ? "Conectando ao servidor local..." : ("Conectando a " + trimmed + "...");
					SplashScreen::draw(0.5f, connectMsg.c_str(), "Entrando no mundo");
					if (initGameplay(programData, ipString))
					{
						gameStarted = true;
						lastError.clear();
					}
					else
					{
						if (trimmed.empty())
							lastError = std::string(loc_CouldntJoinServer()) + " (servidor local nao iniciou / local server failed)";
						else
							lastError = std::string(loc_CouldntJoinServer()) + " - Verifique IP:porta e firewall UDP 7771 / Check IP:port & UDP 7771 firewall";
					}
				}
			}
			programData.ui.menuRenderer.InputText(loc_IP(), ipString, sizeof(ipString),
				Colors_Gray, programData.ui.buttonTexture, false);
			programData.ui.menuRenderer.Text("Ex: 192.168.1.10  ou  192.168.1.10:7771  ou  203.0.113.5:7771", glm::vec4(0.85f, 0.85f, 0.85f, 0.9f));
			programData.ui.menuRenderer.Text("Dica: host precisa liberar UDP 7771 no roteador/firewall", glm::vec4(0.7f, 0.7f, 0.9f, 0.9f));
			displaySettingsMenuButton(programData);
			displaySkinSelectorMenuButton(programData);
		}

	if (programData.ui.menuRenderer.Button(loc_Exit(), Colors_Gray, programData.ui.buttonTexture))
	{
		exitModal.open(loc_Exit(), loc_AreYouSureExit());
	}

	if (!lastError.empty())
	{
		programData.ui.menuRenderer.Text(lastError, glm::vec4(1, 0, 0, 1));
	}

		programData.ui.menuRenderer.End();

	}
	else
	{
		if (isServerRunning())
		{

		#if REMOVE_IMGUI == 0

			if (programData.showImgui)
			{

				auto s = getServerSettingsCopy();
				auto p = getServerProfilerCopy();
				auto tickProfiler = getServerTickProfilerCopy();
				static Profiler profilerCopy;
				static Profiler tickProfilerCopy;
				if (!profilerCopy.pause) { profilerCopy = p; }
				if (!tickProfilerCopy.pause) { tickProfilerCopy = tickProfiler; }


				ImGui::PushStyleColor(ImGuiCol_WindowBg, {26 / 255.f,26 / 255.f,26 / 255.f,0.5f});
				ImGui::Begin("Server window");

				ImGui::Text("Server Chunk Capacity: %d", getChunkCapacity());
				ImGui::Text("Server Ticke per seccond: %d", getServerTicksPerSeccond());
				ImGui::Text("Server Worker tick threads: %d", getThredPoolSize());

				ImGui::Text("Server Pending count: %d", getServerPendingReliableCount());
				ImGui::Text("Server Pending size bytes: %d", (int)getServerTotalPendingSize());

				ImGui::Separator();
				auto &gs = getServerSettingsReff();
				ImGui::Text("Game Rules:");
				ImGui::Checkbox("Hunger System", &gs.hungerEnabled);
				ImGui::Checkbox("Thirst System", &gs.thirstEnabled);
				ImGui::Checkbox("PvP Enabled", &gs.pvpEnabled);
				ImGui::Separator();
				ImGui::Text("Use /gamerule <hunger|thirst|pvp> <on|off> in chat");

				profilerCopy.displayPlot("Server Profiler", 52);
				ImGui::Separator();
				tickProfilerCopy.displayPlot("Tick Profiler for region 0", 52);

				for (auto &c : s.perClientSettings)
				{
					ImGui::PushID(c.first);
					ImGui::Text("%d", c.first);
					ImGui::Text("Position: %lf %lf %lf", c.second.outPlayerPos.x,
						c.second.outPlayerPos.y, c.second.outPlayerPos.z);

					ImGui::Checkbox("Resend inventory", &c.second.resendInventory);

					if (ImGui::Button("Set Survival"))
					{
						changePlayerGameMode(c.first, OtherPlayerSettings::SURVIVAL);
					} ImGui::SameLine();
					if (ImGui::Button("Set Crative"))
					{
						changePlayerGameMode(c.first, OtherPlayerSettings::CREATIVE);
					}

					if (ImGui::Button("Damage"))
					{
						c.second.damage = true;
					}

					if (ImGui::Button("Heal"))
					{
						c.second.heal = true;
					}

					if (ImGui::Button("Kill a pig"))
					{
						c.second.killApig = true;
					}

					if (ImGui::Button("Generate structure!"))
					{
						c.second.generateStructure = true;
					}

					ImGui::Separator();
					ImGui::PopID();
				}


				ImGui::End();
				ImGui::PopStyleColor();

				setServerSettings(s);
			};

		#endif

		}

		if (!gameplayFrame(deltaTime, w, h, programData))
		{
			if(isServerRunning()){
				auto &s = getServerSettingsReff();
				saveWorldConfig(s.worldName, s);
				SaveSystem::get().saveWorld(s.worldName);
				auto &clients = getAllClientsReff();
				for(auto &kv: clients){
					std::string pid = std::to_string(kv.first);
					std::string pdir = std::string(RESOURCES_PATH) + "worlds/" + s.worldName + "/players";
					std::filesystem::create_directories(pdir);
					std::ofstream f(pdir + "/" + pid + ".json");
					if(f.is_open()){
						auto &pos = kv.second.playerData.entity.position;
						f << "{\"uuid\":\"" << pid << "\",\"x\":" << pos.x << ",\"y\":" << pos.y << ",\"z\":" << pos.z << ",\"skin\":\"" << getSkinName() << "\"}\n";
					}
					std::string invFile = pdir + "/" + pid + ".inv";
					std::vector<unsigned char> data;
					kv.second.playerData.inventory.formatIntoData(data);
					std::ofstream invF(invFile, std::ios::binary);
					if(invF.is_open() && !data.empty()) invF.write((char*)data.data(), data.size());
				}
				auto &ls = getLauncherState();
				if(ls.loggedIn){
					PlayerSaveData pd; pd.username=ls.currentUsername; pd.uuid=ls.currentUUID;
					SaveSystem::get().savePlayer(pd.uuid, pd);
				}
				std::cout << "[Save] quit world: inventario, pos, mobs, blocos salvos\n";
			}
			EventBus::instance().trigger("onWorldLeave");
			EventBus::instance().trigger("onPlayerLeave");
			EventBus::instance().trigger("playerDisconnected");
			EventBus::instance().trigger("client:disconnected");
			closeGameLogic();
			closeConnection();
			closeServer();
			gameStarted = false;
			platform::showMouse(true);
			AudioEngine::playTitleMusic();
		}

	}



#pragma region set finishing stuff
	gl2d::enableNecessaryGLFeatures();

	bool anyButtonPressed = 0;
	bool backPressed = 0;
	bool anyCustomWidgetPressed = 0;
	bool anyToggleToggeled = 0;
	bool anyToggleDetoggeled = 0;
	bool andSliderDragged = 0;

	bool isModal = exitModal.show;
	bool mPressed = isModal ? false : platform::isLMousePressed();
	bool mHeld = isModal ? false : platform::isLMouseHeld();
	bool mReleased = isModal ? false : platform::isLMouseReleased();
	bool escReleased = isModal ? false : platform::isKeyReleased(platform::Button::Escape);
	programData.ui.menuRenderer.renderFrame(programData.ui.renderer2d, programData.ui.font, platform::getRelMousePosition(),
		mPressed, mHeld, mReleased,
		escReleased, platform::getTypedInput(), deltaTime, &anyButtonPressed, &backPressed,
		&anyCustomWidgetPressed, &anyToggleToggeled, &anyToggleDetoggeled, &andSliderDragged);
	if(isModal){
		if(platform::isKeyReleased(platform::Button::Escape)){
			exitModal.show=false;
			exitModal.result=false;
			exitModal.resultReady=true;
			AudioEngine::playSound(AudioEngine::uiButtonBack, UI_SOUND_VOLUME);
		}
	}


	if (anyToggleToggeled)
	{
		AudioEngine::playSound(AudioEngine::uiCheckBoxOn, UI_SOUND_VOLUME);
	}
	if(anyToggleDetoggeled)
	{
		AudioEngine::playSound(AudioEngine::uiCheckBoxOff, UI_SOUND_VOLUME);
	}
	if (anyButtonPressed)
	{
		AudioEngine::playSound(AudioEngine::uiButtonPress, UI_SOUND_VOLUME);
	}
	if(backPressed)
	{
		AudioEngine::playSound(AudioEngine::uiButtonBack, UI_SOUND_VOLUME);
	}
	if (andSliderDragged)
	{
		AudioEngine::playSound(AudioEngine::uiSlider, UI_SOUND_VOLUME);
	}

	if(exitModal.show){
		glm::vec2 screenSize = {programData.ui.renderer2d.windowW, programData.ui.renderer2d.windowH};
		if(exitModal.render(programData.ui.renderer2d, programData.ui.font, screenSize)){
			if(exitModal.result) return false;
		}
	}

	if (shadingSettingsCopy != getShadingSettings())
	{
		shadingSettingsCopy = getShadingSettings();

		saveShadingSettings();
	}


	programData.ui.renderer2d.flush();

	return true;
#pragma endregion

}

void closeGame()
{
	static bool closed = false;
	if (closed) return;
	closed = true;
	if(isServerRunning()){
		auto &s = getServerSettingsReff();
		saveWorldConfig(s.worldName, s);
		SaveSystem::get().saveWorld(s.worldName);
		auto &clients = getAllClientsReff();
		for(auto &kv: clients){
			std::string pid = std::to_string(kv.first);
			std::string pdir = std::string(RESOURCES_PATH) + "worlds/" + s.worldName + "/players";
			std::filesystem::create_directories(pdir);
			std::ofstream f(pdir + "/" + pid + ".json");
			if(f.is_open()){
				auto &pos = kv.second.playerData.entity.position;
				f << "{\"uuid\":\"" << pid << "\",\"x\":" << pos.x << ",\"y\":" << pos.y << ",\"z\":" << pos.z << ",\"skin\":\"" << getSkinName() << "\"}\n";
			}
			std::string invFile = pdir + "/" + pid + ".inv";
			std::vector<unsigned char> data;
			kv.second.playerData.inventory.formatIntoData(data);
			std::ofstream invF(invFile, std::ios::binary);
			if(invF.is_open() && !data.empty()) invF.write((char*)data.data(), data.size());
		}
	}else SaveSystem::get().saveWorld("world");
	auto &launcher = getLauncherState();
	if(launcher.loggedIn){
		PlayerSaveData pd;
		pd.username = launcher.currentUsername;
		pd.uuid = launcher.currentUUID;
		SaveSystem::get().savePlayer(pd.uuid, pd);
	}
	std::cout << "[Save] fechar game: tudo salvo\n";
	EventBus::instance().trigger("onGameClose");
	EventBus::instance().trigger("game:close");
	EventBus::instance().trigger("onClientExit");
	EventBus::instance().trigger("onResourceStop");
	Scripting::shutdown();
	closeGameLogic();
	closeConnection();
	closeServer();
	enet_deinitialize();
}
