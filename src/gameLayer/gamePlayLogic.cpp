#include "gamePlayLogic.h"
#include "rendering/camera.h"
#include "platformInput.h"
#include "errorReporting.h"
#include "rendering/renderer.h"
#include "chunkSystem.h"
#include "threadStuff.h"
#include <thread>
#include <ctime>
#include "multyPlayer/server.h"
#include "multyPlayer/createConnection.h"
#include <enet/enet.h>
#include "rendering/UiEngine.h"
#include "glui/glui.h"
#include <gameplay/fishing.h>
#include <limits>
#include <platformTools.h>
#include <platform/platformDetection.h>
#include <localization.h>

#if REMOVE_IMGUI == 0
#include <imgui.h>
#endif
#include <iostream>
#include "multyPlayer/undoQueue.h"
#include <lightSystem.h>
#include <structure.h>
#include <safeSave.h>
#include <rendering/sunShadow.h>
#include <multiPlot.h>
#include <profiler.h>
#include <thread>
#include <gameplay/physics.h>
#include <gameplay/entityManagerClient.h>
#include <gameplay/items.h>

#ifdef PLATFORM_WINDOWS
#include <Windows.h>
#include <commdlg.h>
#else
#ifndef MAXSHORT
#define MAXSHORT 32767
#endif
#endif
#include <gameplay/crafting.h>
#include <rendering/renderSettings.h>
#include <audioEngine.h>
#include <gameplay/mapEngine.h>
#include <gameplay/battleUI.h>
#include <gameplay/food.h>
#include <cameraShaker.h>
#include <blockUpdates.h>
#include <weather.h>
#include "scripting/EventBus.h"

struct GameData
{
	Camera c;
	int cameraMode = 0;
	float thirdPersonDistance = 4.f;
	ChunkSystem chunkSystem;
	bool escapePressed = 0;
	bool showLightLevels = 0;
	UndoQueue undoQueue;
	LightSystem lightSystem;
	int skyLightIntensity = 15;

	SunShadow sunShadow;

	Profiler gameplayFrameProfiler;
	CameraShaker cameraShaker;

	MapEngine mapEngine;
	bool showF3Debug = false;
	bool showChunkBorders = false;
	float bowCharge = 0;
	bool bowCharging = false;
	FishingManager fishingManager;
	struct BreakParticle { glm::dvec3 pos; glm::vec3 vel; float life=1.f; float maxLife=1.f; glm::vec3 color={1,1,1}; };
	std::vector<BreakParticle> breakParticles;
	bool isInsideMapView = 0;
	bool isInsideChat = 0;
	char chatBuffer[250] = {};
	int chatBufferPosition = 0;
	int chatCursor = 0;
	std::deque<std::string> chat;
	std::vector<std::string> chatHistory;
	int chatHistoryIndex = -1;
	float chatStayOnTimer = 0;
	std::vector<std::string> commandSuggestions = {};
	std::string lastRequestedCommandPrefix = "";
	int chatSuggestionSelected = 0;

	//debug stuff
	glm::ivec3 point = {};
	glm::ivec3 pointSize = {};
	glm::dvec3 entityTest = {-4, 113, 3};
	bool renderBox = 1;
	bool renderPlayerPos = 0;
	bool renderColliders = 0;
	
	bool colidable = 1;

	ClientEntityManager entityManager;
	std::unordered_map<std::uint64_t, PlayerConnectionData> playersConnectionData;

	std::uint64_t serverTimer = 0; //this is in MS 
	float serverTimerCounter = 0;

	Player lastSendPlayerData = {};

	int currentItemSelected = 0;
	
	InteractionData interaction;
	unsigned char currentBlockInteractionRevisionNumber = 0;

	AdaptiveExposure adaptiveExposure;

	bool insideInventoryMenu = 0;
	int currentInventoryTab = 0;
	bool justDamaged = 0;
	float hitLensDirt = 0;

	struct
	{
		bool breaking = 0;
		glm::ivec3 pos = {};
		float timer = 0;
		float totalTime = 0;
		int tool = 0;
	}currentBlockBreaking;

	std::string currentSkinName = "";
	gl2d::Texture currentSkinTexture = {};
	GLuint64 currentSkinBindlessTexture = 0;
	BattleUI battleUI;
	 
	BoneTransform playerFOVHandTransform{
		glm::vec3{glm::radians(120.f),0.f,0.f},
		glm::vec3{0.2,-2.0,-0.5}
	};

	bool handHit = 0;
	bool killed = 0;

	//water drops blur
	float dropsStrength = 0;
	bool lastFrameInWater = 0;

	WeatherState weather;

	int craftingSlider = 0;
	bool showUI = 1;

	ConfirmationModal exitWorldModal;
	ConfirmationModal exitGameModal;

	std::minstd_rand rng;

	void clearData()
	{
		for (auto &c : playersConnectionData)
		{
			c.second.cleanup();
		}

		*this = GameData{};
	}

}gameData;

//the /time command changes this, it controls the sun position
float globalDayTime = 0.25f;

void setDayTimeGlobally(float dayTime)
{
	globalDayTime = dayTime;
}

// Global weather state
static int globalWeatherType = 0; // Weather_Clear
static float globalWetness = 0.f;
static WeatherState *globalWeatherStatePtr = nullptr;

void setWeatherGlobally(int type)
{
	globalWeatherType = type;
	std::cerr << "[weather-debug] setWeatherGlobally type=" << type << " globalWeatherType=" << globalWeatherType << std::endl;
}


int getWeatherGlobally()
{
	return globalWeatherType;
}

float getWetnessGlobally()
{
	return globalWetness;
}

void setWetnessGlobally(float w)
{
	globalWetness = w;
}

WeatherState *getGlobalWeatherState()
{
	return globalWeatherStatePtr;
}

ThreadPool threadPoolForChunkBaking;

float &getHitLensDirt()
{
	return gameData.hitLensDirt;
}

void loadCurrentSkin()
{

	if (gameData.currentSkinTexture.id)
	{
		gameData.currentSkinTexture.cleanup();
	}

	gameData.currentSkinName = getSkinName();
	if (gameData.currentSkinName == "")
	{

	}
	else
	{
		gameData.currentSkinTexture = loadPlayerSkin((RESOURCES_PATH "skins/" + gameData.currentSkinName + ".png").c_str());
		if(!gameData.currentSkinTexture.id) gameData.currentSkinTexture = loadPlayerSkin(("./resources/skins/" + gameData.currentSkinName + ".png").c_str());
		if(!gameData.currentSkinTexture.id) gameData.currentSkinTexture = loadPlayerSkin(("resources/skins/" + gameData.currentSkinName + ".png").c_str());
	}

	if (!gameData.currentSkinTexture.id)
	{
		gameData.currentSkinTexture
			= loadPlayerSkin(RESOURCES_PATH "assets/models/steve.png");
	}

	//todo repeating code
	gameData.currentSkinTexture.bind();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 6.f);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_LOD, 2.f);

	glGenerateMipmap(GL_TEXTURE_2D);


	gameData.currentSkinBindlessTexture = glGetTextureHandleARB(gameData.currentSkinTexture.id);
	glMakeTextureHandleResidentARB(gameData.currentSkinBindlessTexture);

	sendPlayerSkinPacket(getServer(), getConnectionData().cid, gameData.currentSkinTexture);

}

#ifdef PLATFORM_WINDOWS
#ifdef _WIN32
bool ShowOpenFileDialog(HWND hwnd, char *filePath, DWORD filePathSize, const char *initialDir,
	const char *filter)
{
	// Initialize the OPENFILENAME structure
	OPENFILENAMEA ofn;
	ZeroMemory(&ofn, sizeof(ofn));

	// Zero out the file path buffer
	ZeroMemory((void*)filePath, filePathSize);


	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFile = filePath;
	ofn.nMaxFile = filePathSize;
	ofn.lpstrInitialDir = initialDir;
	ofn.lpstrFilter = filter;
	ofn.nFilterIndex = 1;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;

	return GetOpenFileNameA(&ofn);
}
#else
bool ShowOpenFileDialog(void* hwnd, char *filePath, unsigned long filePathSize, const char *initialDir, const char *filter){ return false; }
#endif
#else
bool ShowOpenFileDialog(void* hwnd, char *filePath, int filePathSize, const char *initialDir,
	const char *filter)
{
	// Linux stub - file dialog not implemented
	return false;
}
#endif

bool initGameplay(ProgramData &programData, const char *c) //GAME STUFF!
{

	Packet_ReceiveCIDAndData playerData;

	if (!createConnection(playerData, c))
	{
		reportError("Problem joining server");
		return false;
	}

	gameData.clearData();
	//threadPoolForChunkBaking.setThreadsNumber(2, bakeWorkerThread);
	gameData.c.position = glm::vec3(0, 65, 0);

	gameData.entityManager.localPlayer.entity = playerData.entity;
	gameData.entityManager.localPlayer.entityId = playerData.yourPlayerEntityId;
	gameData.entityManager.localPlayer.otherPlayerSettings = playerData.otherSettings;

	gameData.rng.seed(time(0));

	//todo restant timer here ...
	//playerData.timer;


	gameData.chunkSystem.init(getShadingSettings().viewDistance * 2);

	//TODO, MOVE TO PROGRAM DATA!!!!!!!!!!!
	gameData.sunShadow.init();

	loadCurrentSkin();

	//-5359
	//6348
	//todo clear history stuff here
	//programData.GPUProfiler.;

	//gameData.inventory.heldInMouse = Item(BlockTypes::glass);


	//we started the game!
	AudioEngine::stopAllMusicAndSounds();

	AudioEngine::playRandomNightMusic();

	EventBus::instance().trigger("onPlayerJoin");
	EventBus::instance().trigger("playerSpawn");
	EventBus::instance().trigger("onWorldJoin");
	EventBus::instance().trigger("playerJoining");
	EventBus::instance().trigger("onClientConnected");

	return true;
}

//todo: bug: use the same revision for inventory and block placement and also resend block pos or something

void playSoundAndShakeForPlayerTakingDamage()
{
	AudioEngine::playHurtSound();
	gameData.justDamaged = true;
	gameData.hitLensDirt = 1;
	gameData.cameraShaker.triggerHitShake(1.f);
}

void dealDamageToLocalPlayer(int damage)
{
	if (damage < 0) { return; }
	if (damage == 0) { AudioEngine::playHurtSound(); return; }
	damage = std::min(damage, (int)std::numeric_limits<short>::max() - 10);

	auto &p = gameData.entityManager.localPlayer;

	Packet_ClientDamageLocally packetData;
	packetData.damage = damage;

	p.life.life -= damage;
	if (p.life.life <= 0) 
	{
		p.life.life = 0; //kill
		gameData.killed = true;

		sendPacket(getServer(), formatPacket(headerClientDamageLocallyAndDied),
			0, 0,
			true, channelChunksAndBlocks);
	}
	else
	{
		sendPacket(getServer(), formatPacket(headerClientDamageLocally),
			(char *)&packetData, sizeof(Packet_ClientDamageLocally),
			true, channelChunksAndBlocks);

		playSoundAndShakeForPlayerTakingDamage();
	}

}

void paintBlock(int color, Block &b, Chunk &c, glm::ivec3 pos, std::vector<unsigned char> data)
{
	Block oldBlock = b;
	b.setColor(color);
	c.setDirty(true);
	gameData.undoQueue.addPlaceBlockEvent(pos, oldBlock, b, data);
}

void exitInventoryMenu()
{

	if (gameData.interaction.blockInteractionType)
	{

		Packet_RecieveExitBlockInteraction packetData;
		packetData.revisionNumber = gameData.currentBlockInteractionRevisionNumber;

		sendPacket(getServer(), formatPacket(headerRecieveExitBlockInteraction),
			(char *)&packetData, sizeof(Packet_RecieveExitBlockInteraction),
			true, channelChunksAndBlocks);
	}

	gameData.interaction = {};

	gameData.insideInventoryMenu = false;

}



bool gameplayFrame(float deltaTime, int w, int h, ProgramData &programData)
{
	gameData.gameplayFrameProfiler.endSubProfile("swap chain and others");

	if (h != 0)
	{
		gameData.c.aspectRatio = (float)w / h;
	}
	glViewport(0, 0, w, h);

	auto &player = gameData.entityManager.localPlayer;

#pragma region server stuff
	{
		//todo when the server invalidates a block action it should also send you that block state back just
		//in case.
		gameData.gameplayFrameProfiler.startSubProfile("server messages");

		EventCounter validateEvent = 0;
		RevisionNumber inValidateRevision = 0;
		bool disconnect = 0;

		if (!gameData.undoQueue.events.empty())
		{
			auto time = gameData.undoQueue.events[0].createTime;

			//todo Request the server for a hard reset rather than a timeout?
			//todo set networking problem effect when this happens
			if ((getTimer() - time) > 20'000)
			{
				std::cout << "Client timeouted because of validate events!\n";
				return 0;
			}
		}

		bool shouldExitBlockInteraction = 0;
		bool respawned = 0;

		clientMessageLoop(validateEvent, inValidateRevision,
			from3DPointToBlock(gameData.c.position), gameData.chunkSystem.squareSize,
			gameData.entityManager, gameData.undoQueue,
			gameData.chunkSystem, gameData.lightSystem,
			gameData.serverTimer, disconnect,
			gameData.currentBlockInteractionRevisionNumber, shouldExitBlockInteraction,
			gameData.killed, respawned, gameData.chat, gameData.chatStayOnTimer,
			gameData.interaction, gameData.playersConnectionData,
			gameData.commandSuggestions);

		if (disconnect) { return 0; }

		if (respawned)
		{
			gameData.killed = false;
		}

		if (shouldExitBlockInteraction)
		{
			//exit block interaction
			gameData.interaction = {};
			gameData.insideInventoryMenu = 0;
		}

		if (validateEvent)
		{
			while (!gameData.undoQueue.events.empty())
			{
				if (validateEvent >= gameData.undoQueue.events[0].eventId.counter)
				{
					gameData.undoQueue.events.pop_front();
				}
				else
				{
					break;
				}
			}
		}

		//undo stuff
		if (inValidateRevision)
		{
			if (gameData.undoQueue.events.empty())
			{
				permaAssert(0); // undo queue is empty but I revieved an undo message.
			}

			RevisionNumber currentRevisionNumber = gameData.undoQueue.events[0].eventId.revision;
			for (auto &i : gameData.undoQueue.events)
			{
				if (i.eventId.revision != currentRevisionNumber)
				{
					permaAssert(0); // undo queue has inconsistent revisions
				}
			}

			if (inValidateRevision != gameData.undoQueue.currentEventId.revision)
			{
				permaAssert(0 && "inconsistency between the server's revision and mine"); //inconsistency between the server's revision and mine
			}

			for (int i = gameData.undoQueue.events.size() - 1; i >= 0; i--)
			{

				auto &e = gameData.undoQueue.events[i];

				if (e.type == UndoQueueEvent::iPlacedBlock)
				{

					gameData.chunkSystem.placeBlockNoClient(e.blockPos, e.originalBlock, gameData.lightSystem,
						&e.blockData, gameData.interaction, gameData.entityManager);

					if (e.blockPos == gameData.currentBlockBreaking.pos)
					{
						gameData.currentBlockBreaking = {};
					}

				}
				else if (e.type == UndoQueueEvent::iDroppedItemFromInventory)
				{
					gameData.entityManager.removeDroppedItem(e.entityId);
				}
				else if (e.type == UndoQueueEvent::changedBlockData)
				{

					if (e.originalBlock.getType() == BlockTypes::structureBase)
					{

						BaseBlock block;
						size_t _ = 0;
						if (block.readFromBuffer(e.blockData.data(), e.blockData.size(), _))
						{

							auto *c = gameData.chunkSystem.getChunkSafeFromBlockPos(e.blockPos.x, e.blockPos.z);

							if (c)
							{

								auto blockData = c->blockData.getBaseBlock(modBlockToChunk(e.blockPos.x), e.blockPos.y,
									modBlockToChunk(e.blockPos.z));

								if (blockData)
								{
									*blockData = block;
								}

							}

						}

					}
					else if (isChest(e.originalBlock.getType()))
					{

						ChestBlock block;
						size_t _ = 0;
						if (block.readFromBuffer(e.blockData.data(), e.blockData.size(), _))
						{
							auto *c = gameData.chunkSystem.getChunkSafeFromBlockPos(e.blockPos.x, e.blockPos.z);

							if (c)
							{
								auto blockData = c->blockData.getChestBlock(modBlockToChunk(e.blockPos.x), e.blockPos.y,
									modBlockToChunk(e.blockPos.z));

								if (blockData)
								{
									*blockData = block;
								}

							}
						}


					}




				}


			}

			gameData.undoQueue.events.clear();

			gameData.undoQueue.currentEventId.revision++;
		}

		//player sends updates to server
		{

			static float timer = 0.020;

			///todo a common method to check if data was modified
			if (gameData.entityManager.localPlayer.entity != gameData.lastSendPlayerData)
			{
				timer -= deltaTime;
			}
			else
			{
				timer -= deltaTime * 0.1f;
			}

			if (timer <= 0)
			{
				timer = 0.020;
				//timer = 0.316;

				Packer_SendPlayerData data;
				data.timer = gameData.serverTimer;

				//todo SET THIS ALSO SOMEWHERE ELSE LOL!!!!
				gameData.entityManager.localPlayer.entity.chunkDistance = gameData.chunkSystem.squareSize;
				data.playerData = gameData.entityManager.localPlayer.entity;

				sendPacket(getServer(),
					formatPacket(headerSendPlayerData), (char *)&data, sizeof(data), 0,
					channelPlayerPositions);

				gameData.lastSendPlayerData = gameData.entityManager.localPlayer.entity;
			}



		}

		gameData.gameplayFrameProfiler.endSubProfile("server messages");
	}
#pragma endregion

#pragma region timer
	{
		gameData.serverTimerCounter += deltaTime;
		while (gameData.serverTimerCounter > 0.001)
		{
			gameData.serverTimerCounter -= 0.001;
			gameData.serverTimer++;
		}
		//std::cout << gameData.serverTimer << "\n";
	}
#pragma endregion

#pragma region music

	if (!AudioEngine::isMusicPlaying())
	{
		AudioEngine::playRandomNightMusic();
	}

#pragma endregion


#pragma region reload skin

	if (gameData.currentSkinName != getSkinName())
	{
		loadCurrentSkin();
		//todo signal to others
	}

#pragma endregion


#pragma region input


	if (player.otherPlayerSettings.gameMode != OtherPlayerSettings::SURVIVAL)
	{
		gameData.hitLensDirt = 0;
	}
	else
	{
		gameData.hitLensDirt -= deltaTime * 1.2;

		gameData.hitLensDirt = std::max(gameData.hitLensDirt, 0.f);

		if (player.life.life / (float)player.life.maxLife < 0.25)
		{
			gameData.hitLensDirt = std::max(gameData.hitLensDirt, 0.2f);
		}

		if (player.life.life / (float)player.life.maxLife < 0.15)
		{
			gameData.hitLensDirt = std::max(gameData.hitLensDirt, 0.3f);
		}
	}


	bool stopMainInput = gameData.escapePressed || gameData.killed || gameData.insideInventoryMenu ||
		gameData.isInsideMapView || gameData.isInsideChat || gameData.interaction.blockInteractionType != 0;

	static float moveSpeed = 7.f;
	float isPlayerMovingSpeed = 0;


	if (player.otherPlayerSettings.gameMode == OtherPlayerSettings::SURVIVAL)
	{
		player.entity.fly = 0;
	}

	if (gameData.killed)
	{
		gameData.insideInventoryMenu = false;
		gameData.interaction = {};
	}

	if (platform::isKeyReleased(platform::Button::F9))
	{
		programData.showImgui = !programData.showImgui;
	}

	if (platform::isKeyReleased(platform::Button::F10))
	{
		gameData.showUI = !gameData.showUI;
	}

	if (!stopMainInput && platform::isKeyReleased(platform::Button::F5))
	{
		gameData.cameraMode = (gameData.cameraMode + 1) % 3;
	}

	if (platform::isKeyReleased(platform::Button::F3))
	{
		gameData.showF3Debug = !gameData.showF3Debug;
	}

	if (platform::isKeyHeld(platform::Button::F3) && platform::isKeyReleased(platform::Button::G))
	{
		gameData.showChunkBorders = !gameData.showChunkBorders;
	}
	if (platform::isKeyReleased(platform::Button::F3) && platform::isKeyHeld(platform::Button::G))
	{
		gameData.showChunkBorders = !gameData.showChunkBorders;
	}

	if ((!stopMainInput || gameData.insideInventoryMenu) && !programData.ui.isItemSearchFocused())
		if (platform::isKeyReleased(platform::Button::E))
		{
			if (gameData.insideInventoryMenu)
			{
				programData.ui.setItemSearchFocused(false);
				exitInventoryMenu();
			}
			else
			{
				gameData.insideInventoryMenu = true;
				gameData.interaction = {};
			}
		}

	//inventory and menu stuff

	gameData.c.fovRadians = glm::radians(70.f);

	glm::vec3 movementForCameraShake = {};
	if (!stopMainInput)
	{

		if (platform::isKeyHeld(platform::Button::C))
		{
			gameData.c.fovRadians = glm::radians(30.f);
		}

		//move
		{
			auto prelucrateControllerMovement = [&](float x, float y)
			{
				auto rez = glm::vec2(x, y);
				float len = glm::length(rez);

				if (len <= 0.1) { return glm::vec2(); }

				rez /= len;
				len -= 0.1;
				len /= 0.8;
				if (len > 1.f) { len = 1; }
				rez *= len;

				return rez;
			};

			auto prelucrateControllerMovementPower = [&](float x, float y)
			{
				return prelucrateControllerMovement(x * std::abs(x), y * std::abs(y));
			};

			glm::vec3 moveDir = {};
			if (platform::isKeyHeld(platform::Button::Up)
				|| platform::isKeyHeld(platform::Button::W)
				|| platform::getControllerButtons().buttons[platform::ControllerButtons::Up].held
				)
			{
				moveDir.z -= 1;
			}
			if (platform::isKeyHeld(platform::Button::Down)
				|| platform::isKeyHeld(platform::Button::S)
				|| platform::getControllerButtons().buttons[platform::ControllerButtons::Down].held
				)
			{
				moveDir.z += 1;
			}
			if (platform::isKeyHeld(platform::Button::Left)
				|| platform::isKeyHeld(platform::Button::A)
				|| platform::getControllerButtons().buttons[platform::ControllerButtons::Left].held
				)
			{
				moveDir.x -= 1;

			}
			if (platform::isKeyHeld(platform::Button::Right)
				|| platform::isKeyHeld(platform::Button::D)
				|| platform::getControllerButtons().buttons[platform::ControllerButtons::Right].held
				)
			{
				moveDir.x += 1;
			}

			if (player.entity.fly)
			{
				if (platform::isKeyHeld(platform::Button::LeftShift)
					
					)
				{
					moveDir.y -= 1;
				}
				if (platform::isKeyHeld(platform::Button::Space)
					|| platform::getControllerButtons().buttons[platform::ControllerButtons::Rthumb].held
					)
				{
					moveDir.y += 1;
				}
			}
			else
			{
				if (platform::isKeyPressedOn(platform::Button::Space)
					|| platform::getControllerButtons().buttons[platform::ControllerButtons::Rthumb].held
					)
				{
					bool inW = player.isInWater || gameData.entityManager.localPlayer.isInWater;
					if (inW)
					{
						gameData.entityManager.localPlayer.entity.swimUp(WATER_SWIM_IMPULSE * 1.35f);
						if(platform::isKeyHeld(platform::Button::Space))
							gameData.entityManager.localPlayer.entity.forces.velocity.y = glm::max(gameData.entityManager.localPlayer.entity.forces.velocity.y, 2.8f);
					}
					else
					{
						gameData.entityManager.localPlayer.entity.jump();
					}
				}
			}

			//apply controller move
			glm::vec2 controllerMove = prelucrateControllerMovement(platform::getControllerButtons().LStick.x, 
				platform::getControllerButtons().LStick.y);
			moveDir.x += controllerMove.x;
			moveDir.z += controllerMove.y;

			{
				auto &ent = gameData.entityManager.localPlayer.entity;
				bool inWater = player.isInWater;
				bool wantCrouch = !ent.fly && !inWater && (platform::isKeyHeld(platform::Button::LeftCtrl) || platform::isKeyHeld(platform::Button::C));
				bool wantProne = !ent.fly && !inWater && platform::isKeyHeld(platform::Button::Z);
				bool wantSprint = !ent.fly && !inWater && !wantCrouch && !wantProne && platform::isKeyHeld(platform::Button::LeftShift) && glm::length(glm::vec2(moveDir.x,moveDir.z))>0.1f;
				ent.isCrouching = wantCrouch && !wantProne;
				ent.isProne = wantProne;
				ent.isRunning = wantSprint;
				ent.isSwimmingAnim = inWater;
				if(ent.isProne){ ent.crouchTransition = glm::mix(ent.crouchTransition,1.f, deltaTime*8.f); } else if(ent.isCrouching){ ent.crouchTransition = glm::mix(ent.crouchTransition, 0.6f, deltaTime*8.f);} else { ent.crouchTransition = glm::mix(ent.crouchTransition, 0.f, deltaTime*8.f);}
				float l = glm::length(glm::vec2(moveDir.x, moveDir.z));
				if (l != 0)
				{
					moveDir.x /= l;
					moveDir.z /= l;
				}

				float speed = moveSpeed;
				bool flyingFast = ent.fly && platform::isKeyHeld(platform::Button::LeftShift);
				if(ent.isProne) speed *= 0.3f;
				else if(ent.isCrouching) speed *= 0.55f;
				else if(ent.isSwimmingAnim) speed *= 0.7f;
				else if(ent.isRunning) speed *= 1.75f;
				else if(flyingFast) speed *= 3.0f;
				speed *= player.effects.getSpeedMultiplier();
				moveDir *= speed;
				float animSpeed = 0;
				if(glm::length(glm::vec2(moveDir.x,moveDir.z))>0.1f || glm::length(moveDir)>0.1f){
					if(ent.isSwimmingAnim) animSpeed = 1.1f;
					else if(ent.isProne) animSpeed = 0.6f;
					else if(ent.isCrouching) animSpeed = 0.8f;
					else if(ent.isRunning) animSpeed = 1.9f;
					else if(flyingFast) animSpeed = 2.5f;
					else animSpeed = 1.0f;
				}
				ent.movementSpeedForLegsAnimations = animSpeed;
				if(animSpeed>0.1f) ent.animationStateServer.runningTime = 0.6f; else ent.animationStateServer.runningTime = 0;
				if(inWater) ent.animationStateServer.runningTime = 0.6f;
				bool wantAttack = !stopMainInput && (platform::isLMouseHeld() || platform::isLMousePressed());
				ent.animationStateServer.attacked = wantAttack;
			}

			static float jumpTimer = 0;
			if (platform::isKeyPressedOn(platform::Button::Space)
				|| platform::getControllerButtons().buttons[platform::ControllerButtons::Rthumb].pressed)
			{
				if (player.otherPlayerSettings.gameMode == OtherPlayerSettings::CREATIVE)
				{
					if (jumpTimer > 0)
					{
						gameData.entityManager.localPlayer.entity.fly =
							!gameData.entityManager.localPlayer.entity.fly;

						gameData.entityManager.localPlayer.entity.forces = {};
					}
					else
					{
						jumpTimer = 0.2;
					}
				}
			}
			jumpTimer -= deltaTime;
			jumpTimer = std::max(jumpTimer, 0.f);

			if (player.entity.fly)
			{
				gameData.entityManager.localPlayer.entity.flyFPS(moveDir * deltaTime, gameData.c.viewDirection);
			}
			else
			{
				gameData.entityManager.localPlayer.entity.moveFPS(moveDir, gameData.c.viewDirection, deltaTime);
			}

			//gameData.c.moveFPS(moveDir);

			setBodyAndLookOrientation(gameData.entityManager.localPlayer.entity.bodyOrientation,
				gameData.entityManager.localPlayer.entity.lookDirectionAnimation, moveDir, gameData.c.viewDirection);

			//gameData.entityManager.localPlayer.bodyOrientation = 
			//gameData.entityManager.localPlayer.lookDirection = 

			bool rotate = !gameData.escapePressed;
			gameData.c.rotateFPS(platform::getRelMousePosition(), 0.22f * 0.02f, rotate);

			if (rotate) //controller
			{
				gameData.c.rotateFPSController(
					-prelucrateControllerMovementPower(platform::getControllerButtons().RStick.x, platform::getControllerButtons().RStick.y),
					11.0f * deltaTime
				);
			}

		{
			auto &ent = gameData.entityManager.localPlayer.entity;
			float moveSpeed = glm::length(glm::vec2(ent.forces.velocity.x, ent.forces.velocity.z));
			if (moveSpeed > 0.5f && ent.forces.colidesBottom() && !ent.fly)
			{
				static float localBobPhase = 0.f;
				float bobFreq = ent.isRunning ? 14.f : 10.f;
				float bobAmp = ent.isRunning ? 0.04f : 0.02f;
				localBobPhase += deltaTime * bobFreq;
				float bobY = sin(localBobPhase) * bobAmp;
				float bobX = cos(localBobPhase * 0.5f) * bobAmp * 0.3f;
				gameData.c.position += glm::dvec3(0, bobY, 0);
			}
			}

			if (!gameData.escapePressed)
			{
				platform::setRelMousePosition(w / 2, h / 2);
				gameData.c.lastMousePos = {w / 2, h / 2};
			}

			if (glm::length(glm::vec2{moveDir.x, moveDir.z}))
			{
				isPlayerMovingSpeed = 1;
			}

			movementForCameraShake = moveDir;

			auto &held = *player.inventory.getItemFromIndex(gameData.currentItemSelected, nullptr);
			bool hasArrow = false;
			for(int i=PlayerInventory::ARROWS_START_INDEX;i<PlayerInventory::ARROWS_START_INDEX+4;i++){ Item *a = player.inventory.getItemFromIndex(i,nullptr); if(a && a->isArrow() && a->counter>0){ hasArrow=true; break; } }
			if(held.isBow() && hasArrow && platform::isRMouseHeld() && !stopMainInput)
			{
				gameData.bowCharging = true;
				gameData.bowCharge = std::min(gameData.bowCharge + deltaTime, 1.1f);
			}
			else if(gameData.bowCharging && platform::isRMouseReleased())
			{
				if(held.isBow() && hasArrow && gameData.bowCharge > 0.15f)
				{
					float power = glm::mix(9.f, 26.f, std::clamp(gameData.bowCharge/0.9f, 0.f, 1.f));
					int arrowSlot = -1;
					for(int i=PlayerInventory::ARROWS_START_INDEX;i<PlayerInventory::ARROWS_START_INDEX+4;i++){ Item *a = player.inventory.getItemFromIndex(i,nullptr); if(a && a->isArrow() && a->counter>0){ arrowSlot=i; break; } }
					if(arrowSlot!=-1){
						glm::vec3 shootDir = glm::normalize(gameData.c.viewDirection);
						gameData.entityManager.dropItemByClient(player.entity.position + glm::dvec3(0,0.5,0), arrowSlot, gameData.undoQueue, shootDir * power, gameData.serverTimer, player.inventory, 1);
					}
				}
				gameData.bowCharging = false;
				gameData.bowCharge = 0;
			}
			else if(!platform::isRMouseHeld())
			{
				gameData.bowCharging = false;
				gameData.bowCharge = 0;
			}
		}
		{
			auto &heldFish = *player.inventory.getItemFromIndex(gameData.currentItemSelected, nullptr);
			if(heldFish.type == ItemTypes::fishingRod){
				if(platform::isRMousePressed() && !stopMainInput){
					if(!gameData.fishingManager.bobber.active){
						glm::vec3 dir = glm::normalize(gameData.c.viewDirection);
						gameData.fishingManager.startCast(player.entity.position + glm::dvec3(0,1.4,0), dir, 1.0f);
						AudioEngine::playSound(AudioEngine::waterSplash, UI_SOUND_VOLUME);
					}else if(gameData.fishingManager.canCatch()){
						gameData.fishingManager.reel(true);
						Item fish = itemCreator(ItemTypes::rawFish); fish.counter=1;
						player.inventory.tryPickupItem(fish);
						AudioEngine::playSound(AudioEngine::uiButtonPress, UI_SOUND_VOLUME);
					}else{
						if(gameData.fishingManager.bobber.stateTimer > 0.5f) gameData.fishingManager.reset();
					}
				}
				auto bpos = gameData.fishingManager.bobber.pos;
				auto ch = gameData.chunkSystem.getChunkSafeFromBlockPos((int)bpos.x, (int)bpos.z);
				bool inWater = false;
				if(ch){
					auto bp = fromBlockPosToBlockPosInChunk(from3DPointToBlock(bpos));
					auto blk = ch->unsafeGet(bp.x, bp.y, bp.z);
					inWater = blk.getType() == BlockTypes::water;
				}
				gameData.fishingManager.update(deltaTime, player.entity.position, inWater);
				if(gameData.fishingManager.bobber.active){
					glm::dvec3 handPos = player.entity.position + glm::dvec3(0,1.3,0) + glm::dvec3(gameData.c.viewDirection)*0.4;
					programData.gyzmosRenderer.drawLine(handPos, gameData.fishingManager.bobber.pos);
					programData.gyzmosRenderer.drawCube(from3DPointToBlock(gameData.fishingManager.bobber.pos), glm::vec3(0), glm::vec3(0.25f));
				}
			}else{
				if(gameData.fishingManager.bobber.active) gameData.fishingManager.reset();
			}
		}


		//keyPad
		{

			for (int i = 0; i < 9; i++)
			{
				if (platform::isKeyPressedOn(platform::Button::NR1 + i))
				{
					if (gameData.currentItemSelected != i)
					{
						AudioEngine::playSound(AudioEngine::sounds::uiSlider, UI_SOUND_VOLUME);
					}

					gameData.currentItemSelected = i;


				}
			}

			auto scroll = platform::getScroll();
			if (scroll < -0.5 || platform::getControllerButtons().buttons[platform::ControllerButtons::RBumper].pressed)
			{
				if (gameData.currentItemSelected < 8)
				{
					AudioEngine::playSound(AudioEngine::sounds::uiSlider, UI_SOUND_VOLUME);
					gameData.currentItemSelected++;
				}

			}
			else if (scroll > 0.5 || platform::getControllerButtons().buttons[platform::ControllerButtons::LBumper].pressed)
			{
				if (gameData.currentItemSelected > 0)
				{
					AudioEngine::playSound(AudioEngine::sounds::uiSlider, UI_SOUND_VOLUME);
					gameData.currentItemSelected--;
				}
			}

			gameData.currentItemSelected = std::clamp(gameData.currentItemSelected, 0, 8);

			if (gameData.cameraMode != 0)
			{
				auto s = platform::getScroll();
				if (s > 0.5f) gameData.thirdPersonDistance = std::clamp(gameData.thirdPersonDistance - 0.5f, 2.f, 10.f);
				else if (s < -0.5f) gameData.thirdPersonDistance = std::clamp(gameData.thirdPersonDistance + 0.5f, 2.f, 10.f);
			}

		}


	#pragma region drop items

		if (platform::isKeyPressedOn(platform::Button::Q))
		{
			auto *it = player.inventory.getItemFromIndex(gameData.currentItemSelected,nullptr);
			bool hadItem = it && it->type;
			gameData.entityManager.dropItemByClient(
				gameData.entityManager.localPlayer.entity.position,
				gameData.currentItemSelected, gameData.undoQueue, gameData.c.viewDirection * 5.f,
				gameData.serverTimer, player.inventory, !platform::isKeyHeld(platform::Button::LeftCtrl));
			if(hadItem) AudioEngine::playSound(AudioEngine::grass, 0.6f);

			gameData.currentBlockBreaking = {};
		}

	#pragma endregion
	}



#pragma endregion

#pragma region reload shaders
	{

		if (checkIfShadingSettingsChangedForShaderReloads() ||
			(!stopMainInput && platform::isKeyPressedOn(platform::Button::R)))
		//if(!stopMainInput && platform::isKeyPressedOn(platform::Button::R))
		{

			programData.renderer.reloadShaders();
			programData.skyBoxLoaderAndDrawer.clearOnlyGPUdata();
			programData.skyBoxLoaderAndDrawer.createGpuData();
			programData.skyBoxLoaderAndDrawer.createSkyTextures();

			programData.sunRenderer.clear();
			programData.sunRenderer.create();

		}


	}
#pragma endregion



#pragma region block collisions and entity updates!
	{
		auto playerPosLastFrame = player.entity.lastPosition;

		auto chunkGetter = [](glm::ivec2 pos) -> ChunkData*
		{
			auto c = gameData.chunkSystem.getChunkSafeFromChunkPos(pos.x, pos.y);
			if (c)
			{
				return &c->data;
			}
			else
			{
				return nullptr;
			}
		};

		
		if (gameData.killed)
		{
			gameData.entityManager.localPlayer.entity.forces = {};
			gameData.entityManager.localPlayer.entity.updatePositions();
		}
		else
		{

			if (gameData.colidable)
			{
				auto forcesBackup = gameData.entityManager.localPlayer.entity.forces.velocity;

				gameData.entityManager.localPlayer.entity.update(deltaTime, chunkGetter);

				if (gameData.entityManager.localPlayer.entity.forces.colidesBottom())
				{
					gameData.entityManager.localPlayer.entity.fly = false;
				}

				auto newForces = gameData.entityManager.localPlayer.entity.forces.velocity;

				//todo no spawn damage!!!!
				{
					//fall damage
					float rez = glm::length(forcesBackup) - glm::length(newForces);

					//if (rez > 0.2)
					//{
					//	std::cout << "rez: " << rez << "\n";
					//}

					//auto b = 
					auto blockPos = from3DPointToBlock(player.entity.position - glm::dvec3(0, 0.1, 0));
					auto block = gameData.chunkSystem.getBlockSafe(blockPos.x, blockPos.y, blockPos.z);
					int sound = 0;
					if (block)
					{
						sound = getSoundForBlockStepping(block->getType());
					}

					if (rez > 17)
					{
						//high impact
						AudioEngine::playSound(AudioEngine::fallHigh, FALL_SOUND_VOLUME);

						if (sound)
						{
							AudioEngine::playSound(sound, 1);
						}
					}
					if (rez > 14)
					{
						//medium impact
						AudioEngine::playSound(AudioEngine::fallMedium, FALL_SOUND_VOLUME);

						if (sound)
						{
							AudioEngine::playSound(sound, 1);
						}
					}else
					if (rez > 10)
					{
						//low impact
						AudioEngine::playSound(AudioEngine::fallLow, FALL_SOUND_VOLUME);

						if (sound)
						{
							AudioEngine::playSound(sound, 1);
						}
					}
					else if (rez > 8)
					{
						if (sound)
						{
							AudioEngine::playSound(sound, 1);
						}
					}


					if (player.otherPlayerSettings.gameMode ==
						OtherPlayerSettings::SURVIVAL && rez > 13.2)
					{
						int fallDamage = (rez - 12.2);
						fallDamage *= 10;
						//std::cout << "fallDamage: " << fallDamage << "\n";

						dealDamageToLocalPlayer(fallDamage);
					}
				};

			}
			else
			{
				gameData.entityManager.localPlayer.entity.updateForces(deltaTime, !player.entity.fly);

				gameData.entityManager.localPlayer.entity.updatePositions();
			}
		}
		

		{
			auto &e = gameData.entityManager.localPlayer.entity;
			float eyeH = 1.5f;
			if(e.isProne) eyeH = 0.4f + e.crouchTransition*0.2f;
			else if(e.isCrouching) eyeH = 1.0f;
			else if(e.isSwimmingAnim) eyeH = 0.9f;
			else if(e.fly) eyeH = 1.5f;
			gameData.c.position = e.position + glm::dvec3(0,eyeH,0);
		}

		if (gameData.cameraMode != 0)
		{
			glm::dvec3 eye = gameData.c.position;
			glm::vec3 dir = glm::normalize(gameData.c.viewDirection);
			float dist = gameData.thirdPersonDistance;
			glm::vec3 offsetDir = (gameData.cameraMode == 1) ? -dir : dir;
			glm::dvec3 target = eye + glm::dvec3(offsetDir) * (double)dist;
			glm::ivec3 hitPos; std::optional<glm::ivec3> prev; float hitDist;
			Block *hit = gameData.chunkSystem.rayCast(eye, offsetDir, hitPos, dist, prev, hitDist);
			if (hit)
			{
				float safe = std::max(0.3f, hitDist - 0.4f);
				target = eye + glm::dvec3(offsetDir) * (double)safe;
			}
			gameData.c.position = target;
		}

		gameData.entityManager.doAllUpdates(deltaTime, chunkGetter, gameData.serverTimer);

		for(auto &p : gameData.breakParticles){ p.vel.y -= 9.8f * deltaTime * 0.7f; p.pos += glm::dvec3(p.vel) * (double)deltaTime; p.life -= deltaTime; }
		gameData.breakParticles.erase(std::remove_if(gameData.breakParticles.begin(), gameData.breakParticles.end(), [](auto &p){ return p.life<=0; }), gameData.breakParticles.end());

		gameData.cameraShaker.updateCameraShake(deltaTime, movementForCameraShake, 
			glm::vec3(player.entity.position - playerPosLastFrame) * deltaTime,
			gameData.justDamaged);


		gameData.cameraShaker.applyCameraShake(gameData.c);


	}
#pragma endregion


#pragma region blockNieghbourChangeUpdate

	auto computeNeighbourChangeUpdateOneBLock = [&](Block &in, 
		int x, int y, int z)
	{

		if (hasBlockNeighbourChangeUpdate(in.getType()))
		{
			
			auto up = gameData.chunkSystem.getBlockSafe(x, y + 1, z);
			auto down = gameData.chunkSystem.getBlockSafe(x, y - 1, z);
			auto left = gameData.chunkSystem.getBlockSafe(x-1, y, z);
			auto right = gameData.chunkSystem.getBlockSafe(x+1, y, z);
			auto front = gameData.chunkSystem.getBlockSafe(x, y, z + 1);
			auto back = gameData.chunkSystem.getBlockSafe(x, y, z - 1);


			auto rez = blockNieghbourChangeUpdate(in,
				front ? std::optional<Block>(*front) : std::nullopt,
				back ? std::optional<Block>(*back) : std::nullopt,
				up ? std::optional<Block>(*up) : std::nullopt,
				down ? std::optional<Block>(*down) : std::nullopt,
				left ? std::optional<Block>(*left) : std::nullopt,
				right ? std::optional<Block>(*right) : std::nullopt);
			
			if (rez.newBlockType.typeAndFlags != in.typeAndFlags)
			{

				//todo drop block
				if (rez.shouldDropCurrentBlock)
				{

				}
				
				std::cout << "Yes!\n";

				gameData.chunkSystem.placeBlockNoClient({x,y,z}, rez.newBlockType, gameData.lightSystem, 0, 
					gameData.interaction, gameData.entityManager);


			}



		}



	};

	auto computeNeighbourChangeUpdate = [&](
		int x, int y, int z)
	{
		auto up = gameData.chunkSystem.getBlockSafe(x, y + 1, z);
		auto down = gameData.chunkSystem.getBlockSafe(x, y - 1, z);
		auto left = gameData.chunkSystem.getBlockSafe(x - 1, y, z);
		auto right = gameData.chunkSystem.getBlockSafe(x + 1, y, z);
		auto front = gameData.chunkSystem.getBlockSafe(x, y, z + 1);
		auto back = gameData.chunkSystem.getBlockSafe(x, y, z - 1);

		if (up) { computeNeighbourChangeUpdateOneBLock(*up, x, y + 1, z); }
		if (down) { computeNeighbourChangeUpdateOneBLock(*down, x, y - 1, z); }
		if (left) { computeNeighbourChangeUpdateOneBLock(*left, x - 1, y, z); }
		if (right) { computeNeighbourChangeUpdateOneBLock(*right, x + 1, y, z); }
		if (front) { computeNeighbourChangeUpdateOneBLock(*front, x, y, z + 1); }
		if (back) { computeNeighbourChangeUpdateOneBLock(*back, x, y, z - 1); }

	};

#pragma endregion




#pragma region place blocks

	glm::ivec3 rayCastPos = {};
	std::optional<glm::ivec3> blockToPlace = std::nullopt;
	glm::dvec3 cameraRayPos = gameData.c.position;
	Block *raycastBlock = 0;
	glm::uint64 targetedEntity = 0;
	float entityHitDistance = 0;
	float raycastDist = 0;
	bool topPartForSlabs = 0;

	int facingDirection = gameData.c.getViewDirectionRotation();
	//std::cout << facingDirection << "\n";

	if (!stopMainInput)
	{

		if (platform::isLMouseHeld() || platform::isRMousePressed())
		{
			gameData.handHit = true;
		}

		constexpr static float TARGET_DIST = 20;
		float dist = TARGET_DIST;


		raycastBlock = gameData.chunkSystem.rayCast(cameraRayPos, gameData.c.viewDirection,
			rayCastPos, TARGET_DIST, blockToPlace, raycastDist);
		{
			auto *heldWater = player.inventory.getItemFromIndex(gameData.currentItemSelected,nullptr);
			bool isBucket = heldWater && (heldWater->type==ItemTypes::waterBottle || heldWater->type==ItemTypes::wateringCan || heldWater->type==ItemTypes::lighter);
			bool inW = player.isInWater;
			int maxSkip = inW ? 14 : 3;
			int skip=0;
			while(raycastBlock && raycastBlock->getType()==BlockTypes::water && !isBucket && skip<maxSkip){
				glm::dvec3 newStart = glm::dvec3(rayCastPos) + glm::dvec3(gameData.c.viewDirection)*0.6;
				float used = glm::distance(cameraRayPos, glm::dvec3(rayCastPos)) + 0.6f;
				float remain = TARGET_DIST - used;
				if(remain<=0){ raycastBlock=nullptr; blockToPlace=std::nullopt; break; }
				float newDist=0; std::optional<glm::ivec3> newPlace;
				auto *secondHit = gameData.chunkSystem.rayCast(newStart, gameData.c.viewDirection, rayCastPos, remain, newPlace, newDist);
				raycastDist = used + newDist;
				blockToPlace = newPlace;
				raycastBlock = secondHit;
				skip++;
				if(raycastBlock && raycastBlock->getType()!=BlockTypes::water) break;
			}
			if(raycastBlock && raycastBlock->getType()==BlockTypes::water && !isBucket) { raycastBlock=nullptr; blockToPlace=std::nullopt; }
		}

		if (raycastBlock)
		{

			glm::dvec3 intersectPos = {};
			float intersectDist = 0;
			int intersectFace = 0;

			if (lineIntersectBoxGetPos(cameraRayPos, gameData.c.viewDirection, glm::dvec3(rayCastPos) -
				glm::dvec3(0, 0.5, 0), glm::dvec3(1.f), intersectPos, intersectDist, intersectFace))
			{

				//const char * names[] = {"front", "back", "top", "bottom", "left", "right"};
				//std::cout << names[intersectFace] << "\n";

				//top
				if (intersectFace == 2)
				{
					topPartForSlabs = 0;
				}
				else if (intersectFace == 3) //bottom
				{
					topPartForSlabs = 1;
				}
				else if (intersectPos.y - int(intersectPos.y) < 0.5)
				{
					topPartForSlabs = 1;
				}else
				{
					topPartForSlabs = 0;
				}
				//std::cout << (topPartForSlabs ? "top\n" : "bottom\n");

			}




			dist = raycastDist - 0.1;
		}

		//current selected item
		auto &item = player.inventory.items[gameData.currentItemSelected];
		auto weaponStats = item.getWeaponStats();

		targetedEntity = gameData.entityManager.intersectAllAttackableEntities(cameraRayPos,
			gameData.c.viewDirection, std::min(dist, weaponStats.range) , entityHitDistance,
			weaponStats.getAccuracyAdjusted());

		//std::cout << targetedEntity << "\n";

		if (targetedEntity)
		{
			raycastBlock = nullptr;
			blockToPlace = {};
		}

		if (raycastBlock)
		{
			//todo special function here
			if (raycastBlock->getType() != BlockTypes::water)
			{
				auto collider = raycastBlock->getCollider();
				collider.offset.y -= (1.f - collider.size.y) / 2.f;
				programData.gyzmosRenderer.drawCube(rayCastPos, collider.offset, collider.size);

				if (raycastBlock->hasSecondCollider())
				{
					auto collider = raycastBlock->getSecondCollider();
					collider.offset.y -= (1.f - collider.size.y) / 2.f;
					programData.gyzmosRenderer.drawCube(rayCastPos, collider.offset, collider.size);
				}

			}
			else
			{
				gameData.currentBlockBreaking = {};
			}
		}
		else
		{
			gameData.currentBlockBreaking = {};
		}

		//place blocks
		//if (!gameData.escapePressed)
		{


			if (item.isBlock())
			{
				if (platform::isKeyReleased(platform::Button::Z)) { item.type--; }
				if (platform::isKeyReleased(platform::Button::X)) { item.type++; }

				item.type = glm::clamp(item.type, (unsigned short)1u,
					(unsigned short)(BlocksCount - 1u));
			}


			if (platform::isKeyHeld(platform::Button::LeftCtrl)
				&& (platform::isLMousePressed() || platform::isRMousePressed())
				)
			{
				if (player.otherPlayerSettings.gameMode == OtherPlayerSettings::CREATIVE
					&&
					blockToPlace)
				{
					auto b = gameData.chunkSystem.getBlockSafe(rayCastPos);
					if (b)
					{
						Item newItem(b->getType());

						item = newItem;

						forceOverWriteItem(player.inventory, 0, gameData.currentItemSelected, item);

					}
				}

			}
			else
				if (platform::isKeyHeld(platform::Button::LeftAlt))
				{
					if (platform::isRMouseReleased())
					{
						if (blockToPlace)
						{
							gameData.point = *blockToPlace;
						}
					}
					else if (platform::isLMouseReleased())
					{
						if (blockToPlace)
						{
							gameData.pointSize = *blockToPlace - gameData.point;
						}
					}
				}
				else if (!platform::isKeyHeld(platform::Button::LeftCtrl))
				{

					static float soundTimer = 0;
					if (platform::isLMousePressed())
					{
						soundTimer = 0;
					}

					if (platform::isRMousePressed())
					{
						bool didAction = 0;
						Chunk *c = 0;
						std::vector<unsigned char> blockData;
						auto b = gameData.chunkSystem.getBlockAndData(rayCastPos, blockData, c);
						if(b)
						{
							
							auto actionType = isInteractable(b->getType());


							if (player.inventory.getItemFromIndex(gameData.currentItemSelected, nullptr)->isPaint())
							{
								auto &item = *player.inventory.getItemFromIndex(gameData.currentItemSelected, nullptr);

								int paintType = item.type - soap;

								paintBlock(paintType, *b, *c, rayCastPos, blockData);

								


							}else
							if (actionType)
							{
								didAction = true;


								gameData.interaction = {};
								gameData.currentBlockInteractionRevisionNumber++;
								gameData.interaction.blockInteractionType = actionType;
								gameData.interaction.block = b->getType();
								gameData.interaction.blockInteractionPosition = rayCastPos;

								if (actionType == InteractionTypes::structureBaseBlock)
								{
									auto baseBlock = c->blockData.getBaseBlock(
										modBlockToChunk(rayCastPos.x), rayCastPos.y, modBlockToChunk(rayCastPos.z));
									if (baseBlock)
									{
										gameData.interaction.baseBlockHolder = *baseBlock;
									}
								}

								sendBlockInteractionMessage(player.entityId, rayCastPos,
									b->getType(), gameData.currentBlockInteractionRevisionNumber);
								
								gameData.insideInventoryMenu = false;
								gameData.currentInventoryTab = 0;

								if (actionType >= InteractionTypes::craftingTable &&
									actionType < InteractionTypes::structureBaseBlock
									)
								{
									gameData.insideInventoryMenu = true;
									gameData.currentInventoryTab = INVENTORY_TAB_CRAFTING;
								}

								if (actionType == InteractionTypes::chestInteraction)
								{
									gameData.insideInventoryMenu = true;
									gameData.currentInventoryTab = INVENTORY_TAB_CHEST;
								}

								//reset crafting table
								// TODO!!! exit menu thingy here
								//gameData.craftingTableInventory = {};
							}

						}

						if (!didAction)
						{

							//TODO
							//also send the time so the server can know from when to simulate that.
							if (item.isEatable())
							{

								bool allowed = true;
								{
									auto effects = getItemEffects(item, player.inventory);
									int healing = getItemHealing(item, player.inventory);

									//can't eat if satiety doesn't allow it
									if (effects.allEffects[Effects::Saturated].timerMs > 0 &&
										player.effects.allEffects[Effects::Saturated].timerMs > 0
										)
									{
										allowed = 0;
									}
									else
									{
										player.life.life += healing;
										player.effects.applyEffects(effects);
										player.life.sanitize();
									}
								}

								if (allowed)
								{
									Packet_ClientUsedItem data;
									data.from = gameData.currentItemSelected;
									data.itemType = item.type;
									data.revisionNumber = player.inventory.revisionNumber;

									sendPacket(getServer(), headerClientUsedItem, player.entityId,
										&data, sizeof(data), true, channelChunksAndBlocks);

									if (item.isConsumedAfterUse() && player.otherPlayerSettings.gameMode ==
										OtherPlayerSettings::SURVIVAL)
									{
										item.counter--;
										if (item.counter <= 0)
										{
											item = {};
										}
									}
								};

							}else
							if (item.isItemThatCanBeUsed())
							{

								Packet_ClientUsedItem data;
								data.from = gameData.currentItemSelected;
								data.itemType = item.type;
								data.revisionNumber = player.inventory.revisionNumber;
								bool good = true;

								if (item.isBow())
								{
									good = false;
								}
								else if (item.isSeed() && blockToPlace)
								{
									Block *below = gameData.chunkSystem.getBlockSafe(blockToPlace->x, blockToPlace->y -1, blockToPlace->z);
									Block *at = gameData.chunkSystem.getBlockSafe(blockToPlace->x, blockToPlace->y, blockToPlace->z);
									if(at && at->getType()==BlockTypes::air && below && (below->getType()==BlockTypes::dirt || below->getType()==BlockTypes::grassBlock || below->getType()==BlockTypes::coarseDirt))
									{
										bool hasWater=false;
										for(int dx=-4;dx<=4 && !hasWater;dx++) for(int dz=-4;dz<=4 && !hasWater;dz++){
											auto *wb = gameData.chunkSystem.getBlockSafe(blockToPlace->x+dx, blockToPlace->y-1, blockToPlace->z+dz);
											if(wb && wb->getType()==BlockTypes::water) hasWater=true;
										}
										if(hasWater || player.otherPlayerSettings.gameMode==OtherPlayerSettings::CREATIVE){
											Block crop; 
											if(item.type==ItemTypes::potatoSeeds) crop.setType(BlockTypes::potatoCrop);
											else if(item.type==ItemTypes::cornSeeds) crop.setType(BlockTypes::cornCrop);
											else if(item.type==ItemTypes::carrotSeeds) crop.setType(BlockTypes::carrotCrop);
											else crop.setType(BlockTypes::wheatCrop);
											crop.setCropStage(0);
											gameData.chunkSystem.placeBlockNoClient(*blockToPlace, crop, gameData.lightSystem, nullptr, gameData.interaction, gameData.entityManager);
											AudioEngine::playSound(AudioEngine::dirt, PLACED_BLOCK_SOUND_VOLUME);
											if(player.otherPlayerSettings.gameMode==OtherPlayerSettings::SURVIVAL){ item.counter--; if(item.counter<=0) item={}; }
										}
									}
									good = false;
								}
								else if ((item.isBoneMealItem() || item.isFertilizerItem() || item.type==ItemTypes::compost) && raycastBlock && raycastBlock->isCrop())
								{
									int stage = raycastBlock->getCropStage();
									if(stage < 7){
										int inc = item.isFertilizerItem() ? 3 : item.type==ItemTypes::compost ? 2 : 1;
										int ns = std::min(7, stage + inc);												Block nb = *raycastBlock; nb.setCropStage(ns);
												gameData.chunkSystem.placeBlockNoClient(rayCastPos, nb, gameData.lightSystem, nullptr, gameData.interaction, gameData.entityManager);
												AudioEngine::playSound(AudioEngine::grass, PLACED_BLOCK_SOUND_VOLUME);
												//green growth particles
												{ glm::vec3 cp = glm::vec3(rayCastPos) + glm::vec3(0.5f, 0.25f, 0.5f); for(int i=0;i<10;i++){ GameData::BreakParticle bp; bp.pos = glm::dvec3(cp); bp.vel = glm::vec3((rand()%100)/100.f-0.5f, 0.4f + (rand()%100)/100.f*0.8f, (rand()%100)/100.f-0.5f)*1.6f; bp.life=0.5f; bp.maxLife=0.5f; bp.color=glm::vec3(0.3f,1.0f,0.25f); gameData.breakParticles.push_back(bp); } }
												if(player.otherPlayerSettings.gameMode==OtherPlayerSettings::SURVIVAL){ item.counter--; if(item.counter<=0) item={}; }
									}
									good = false;
								}
								else if (item.type==ItemTypes::wateringCan && raycastBlock && raycastBlock->isCrop())
								{
									int stage = raycastBlock->getCropStage();
									if(stage < 7 && (rand()%2==0)){
										Block nb = *raycastBlock; nb.setCropStage(stage+1);
										gameData.chunkSystem.placeBlockNoClient(rayCastPos, nb, gameData.lightSystem, nullptr, gameData.interaction, gameData.entityManager);
									}
									AudioEngine::playSound(AudioEngine::waterSplash, PLACED_BLOCK_SOUND_VOLUME);
									good = false;
								}
								else if (item.isLighter() && raycastBlock && raycastBlock->getType()==BlockTypes::torchUnlit)
								{
									Block nb; nb.typeAndFlags = raycastBlock->typeAndFlags; nb.setType(BlockTypes::torch); nb.setLightLevel(15);
									gameData.chunkSystem.placeBlockNoClient(rayCastPos, nb, gameData.lightSystem, nullptr, gameData.interaction, gameData.entityManager);
									AudioEngine::playSound(AudioEngine::metal, PLACED_BLOCK_SOUND_VOLUME);
									{ glm::vec3 p = glm::vec3(rayCastPos) + glm::vec3(0.5f); for(int i=0;i<6;i++){ GameData::BreakParticle bp; bp.pos = glm::dvec3(p) + glm::dvec3((rand()%100)/100.f-0.5f)*0.4; bp.vel = glm::vec3((rand()%100)/100.f-0.5f, 0.8f+ (rand()%100)/200.f, (rand()%100)/100.f-0.5f)*1.2f; bp.life=0.35f; bp.maxLife=0.35f; bp.color=glm::vec3(1.0f,0.6f,0.15f); gameData.breakParticles.push_back(bp); } }
									if(player.otherPlayerSettings.gameMode==OtherPlayerSettings::SURVIVAL){ int d = item.getLighterDurability(); d--; item.setLighterDurability(d); if(d<=0) item={}; }
									data.position = rayCastPos;
								}
								else if (item.isLighter() && blockToPlace)
								{
									auto at = gameData.chunkSystem.getBlockSafe(blockToPlace->x, blockToPlace->y, blockToPlace->z);
									auto below = gameData.chunkSystem.getBlockSafe(blockToPlace->x, blockToPlace->y-1, blockToPlace->z);
									bool canPlaceFire = at && at->getType()==BlockTypes::air;
									bool hasFuel = false;
									const int dirs[6][3]={{1,0,0},{-1,0,0},{0,1,0},{0,-1,0},{0,0,1},{0,0,-1}};
									for(auto &d: dirs){ auto nb2 = gameData.chunkSystem.getBlockSafe(blockToPlace->x+d[0], blockToPlace->y+d[1], blockToPlace->z+d[2]); if(nb2 && isFlammable(nb2->getType())) hasFuel=true; }
									float fuelScore = 0.f;
									for(auto &d: dirs){ auto nb2 = gameData.chunkSystem.getBlockSafe(blockToPlace->x+d[0], blockToPlace->y+d[1], blockToPlace->z+d[2]); if(nb2 && isFlammable(nb2->getType())) fuelScore += getFlammability(nb2->getType()); }
									if(below && isFlammable(below->getType())) fuelScore += getFlammability(below->getType())*1.2f;
									float wet = programData.weatherState.wetness;
									if(wet>0.6f) fuelScore *= (1.f - wet*0.55f);
									bool canIgnite = canPlaceFire && (fuelScore > 0.12f);
									if(canIgnite){
										Block fire; fire.setType(BlockTypes::fire); fire.setLightLevel(13);
										gameData.chunkSystem.placeBlockNoClient(*blockToPlace, fire, gameData.lightSystem, nullptr, gameData.interaction, gameData.entityManager);
										AudioEngine::playSound(AudioEngine::metal, PLACED_BLOCK_SOUND_VOLUME);
										{ glm::vec3 p = glm::vec3(*blockToPlace) + glm::vec3(0.5f); for(int i=0;i<7;i++){ GameData::BreakParticle bp; bp.pos = glm::dvec3(p) + glm::dvec3((rand()%100)/100.f-0.5f)*0.35; bp.vel = glm::vec3((rand()%100)/100.f-0.5f, 0.9f+ (rand()%100)/180.f, (rand()%100)/100.f-0.5f)*1.4f; bp.life=0.42f; bp.maxLife=0.42f; bp.color=glm::vec3(1.0f,0.55f,0.12f); gameData.breakParticles.push_back(bp); } }
										if(player.otherPlayerSettings.gameMode==OtherPlayerSettings::SURVIVAL){ int dd = item.getLighterDurability(); dd--; item.setLighterDurability(dd); if(dd<=0) item={}; }
										data.position = *blockToPlace;
									} else {
										if(fuelScore <= 0.12f){ AudioEngine::playSound(AudioEngine::metal, 0.35f); }
										// tenta acender bloco alvo se for inflamável
										if(raycastBlock && isFlammable(raycastBlock->getType()) && at && at->getType()==BlockTypes::air){
											Block fire; fire.setType(BlockTypes::fire); fire.setLightLevel(13);
											gameData.chunkSystem.placeBlockNoClient(*blockToPlace, fire, gameData.lightSystem, nullptr, gameData.interaction, gameData.entityManager);
											AudioEngine::playSound(AudioEngine::metal, PLACED_BLOCK_SOUND_VOLUME);
											{ glm::vec3 p = glm::vec3(*blockToPlace) + glm::vec3(0.5f); for(int i=0;i<7;i++){ GameData::BreakParticle bp; bp.pos = glm::dvec3(p) + glm::dvec3((rand()%100)/100.f-0.5f)*0.35; bp.vel = glm::vec3((rand()%100)/100.f-0.5f, 0.9f+ (rand()%100)/180.f, (rand()%100)/100.f-0.5f)*1.4f; bp.life=0.42f; bp.maxLife=0.42f; bp.color=glm::vec3(1.0f,0.55f,0.12f); gameData.breakParticles.push_back(bp); } }
											if(player.otherPlayerSettings.gameMode==OtherPlayerSettings::SURVIVAL){ int dd = item.getLighterDurability(); dd--; item.setLighterDurability(dd); if(dd<=0) item={}; }
											data.position = *blockToPlace;
										}
									}
								}
								else if (item.isPaint())
								{
									data.position = rayCastPos;
								}
								else if(item.type == ItemTypes::fishSpawnEgg && blockToPlace)
								{
									data.position = *blockToPlace;
								}
								else
								{
									// No handler matched - don't send packet or consume
									good = false;
								}

								data.eventId = gameData.undoQueue.currentEventId;

								if (good)
								{
									sendPacket(getServer(), headerClientUsedItem, player.entityId,
										&data, sizeof(data), true, channelChunksAndBlocks);

									// Play sound for equipment items
									if (item.isEquipement())
									{
										AudioEngine::playSound(AudioEngine::wood, UI_SOUND_VOLUME);
									}

									if (item.isConsumedAfterUse() && player.otherPlayerSettings.gameMode ==
										OtherPlayerSettings::SURVIVAL)
									{
										item.counter--;
										if (item.counter <= 0)
										{
											item = {};
									}
									}
								}
								

							}
							else if (blockToPlace && item.isBlock())
							{
								bool dontPlace = false;

								//todo intersect other entities
								if (isColidable(item.type))
								{
									if (boxColideBlock(
										player.entity.position,
										player.entity.getColliderSize(),
										*blockToPlace
										))
									{
										dontPlace = true;
									}
								};

								int faceDirection = facingDirection;
								int isOnWall = 0;

								if (!dontPlace &&
									(isWallMountedBlock(item.type) ||
									isWallMountedOrStangingBlock(item.type))
									)
								{

									if (!blockToPlace)
									{
										dontPlace = true;
									}
									else
									{
										bool skipNextStep = 0;
										if (isWallMountedOrStangingBlock(item.type))
										{
											glm::ivec3 placeDiff = *blockToPlace - rayCastPos;

											if (placeDiff == glm::ivec3(0, 1, 0))
											{
												//good
												skipNextStep = true;
											}
											else if (placeDiff == glm::ivec3(0, -1, 0))
											{
												//bad 
												dontPlace = true;
												skipNextStep = true;
											}


										}

										if (!skipNextStep)
										{
											glm::ivec3 placeDiff = *blockToPlace - rayCastPos;

											if (placeDiff == glm::ivec3(1, 0, 0) ||
												placeDiff == glm::ivec3(-1, 0, 0) ||
												placeDiff == glm::ivec3(0, 0, 1) ||
												placeDiff == glm::ivec3(0, 0, -1))
											{
												//todo walls as well
												if (raycastBlock &&
													raycastBlock->canWallMountedBlocksBePlacedOn())
												{
													isOnWall = true;
													//good
													//we place ladders only on blocks

													if (placeDiff == glm::ivec3(1, 0, 0))
													{
														faceDirection = 1;
													}
													else if (placeDiff == glm::ivec3(-1, 0, 0))
													{
														faceDirection = 3;
													}
													else if (placeDiff == glm::ivec3(0, 0, 1))
													{
														faceDirection = 0;
													}
													else if (placeDiff == glm::ivec3(0, 0, -1))
													{
														faceDirection = 2;
													}

												}
												else
												{
													dontPlace = true;
												}
											}
											else
											{
												dontPlace = true;
											}

											
										}

									}

									
								}

								if (!dontPlace)
								{
									//place block
									gameData.chunkSystem.placeBlockByClient(*blockToPlace,
										gameData.currentItemSelected,
										gameData.undoQueue,
										gameData.entityManager.localPlayer.entity.position,
										gameData.lightSystem,
										player.inventory,
										player.otherPlayerSettings.gameMode == OtherPlayerSettings::SURVIVAL,
										faceDirection, topPartForSlabs, isOnWall, gameData.entityManager
									);


									std::vector<unsigned char> blockData;
									auto b = gameData.chunkSystem.getBlockAndData(*blockToPlace,
										blockData, c);


									auto itemLast = player.inventory.getItemFromIndex(8, nullptr);
									if (itemLast && itemLast->isPaint())
									{


										int paintType = itemLast->type - soap;
										paintBlock(paintType, *b, *c, *blockToPlace, blockData);

										Packet_ClientUsedItem data;
										data.from = 8;
										data.itemType = itemLast->type;
										data.revisionNumber = player.inventory.revisionNumber;

										data.position = *blockToPlace;
										data.eventId = gameData.undoQueue.currentEventId;

										sendPacket(getServer(), headerClientUsedItem, player.entityId,
											&data, sizeof(data), true, channelChunksAndBlocks);
									}

									AudioEngine::playSound(getSoundForBlockStepping(item.type),
										PLACED_BLOCK_SOUND_VOLUME);

									computeNeighbourChangeUpdate(blockToPlace->x, blockToPlace->y, blockToPlace->z);


								}
								
							}
						};

							
					}
					else if (platform::isLMouseHeld() && raycastBlock
						&& !item.isWeapon()
						)
					{
						if (gameData.currentBlockBreaking.breaking &&
							gameData.currentBlockBreaking.tool != gameData.currentItemSelected)
						{
							gameData.currentBlockBreaking.breaking = false;
						}

						if (!gameData.currentBlockBreaking.breaking)
						{
							gameData.currentBlockBreaking.breaking = true;
							gameData.currentBlockBreaking.pos = rayCastPos;
							gameData.currentBlockBreaking.tool = gameData.currentItemSelected;

							if (player.otherPlayerSettings.gameMode == OtherPlayerSettings::SURVIVAL)
							{
								gameData.currentBlockBreaking.timer = computeMineDurationTime(raycastBlock->getType(),
									*player.inventory.getItemFromIndex(gameData.currentItemSelected, nullptr));
								gameData.currentBlockBreaking.totalTime = gameData.currentBlockBreaking.timer;
							}
							else
							{
								gameData.currentBlockBreaking.totalTime = 0.3;
								gameData.currentBlockBreaking.timer = 0.3;

								if (platform::isLMousePressed())
								{
									gameData.currentBlockBreaking.totalTime = 0;
									gameData.currentBlockBreaking.timer = 0;
								}
							}
						}



						if (rayCastPos != gameData.currentBlockBreaking.pos)
						{
							gameData.currentBlockBreaking = {};
							soundTimer = 0;
						}
						else
						{
							gameData.currentBlockBreaking.timer -= deltaTime;
							
							if (gameData.currentBlockBreaking.timer <= 0)
							{
								auto sound = getSoundForBlockBreaking(raycastBlock->getType());
								AudioEngine::playSound(sound, BREAKED_BLOCK_SOUND_VOLUME);
								//AudioEngine::playSound(AudioEngine::crackStone, 0.3);

								//break block
								gameData.chunkSystem.breakBlockByClient(rayCastPos
									, gameData.undoQueue,
									gameData.entityManager.localPlayer.entity.position,
									gameData.lightSystem, gameData.entityManager);
								for(int i=0;i<6;i++){ GameData::BreakParticle p; p.pos = glm::dvec3(rayCastPos)+glm::dvec3(0.5,0.5,0.5); p.vel = glm::vec3((rand()%100-50)/80.f, (rand()%100)/60.f +1.f, (rand()%100-50)/80.f); p.life=0.6f; p.maxLife=0.6f; p.color = glm::vec3(0.6f,0.5f,0.35f); if(raycastBlock && raycastBlock->getType()==BlockTypes::grassBlock) p.color={0.4f,0.6f,0.2f}; else if(raycastBlock && raycastBlock->getType()==BlockTypes::stone) p.color={0.6f,0.6f,0.6f}; gameData.breakParticles.push_back(p); }
								gameData.currentBlockBreaking = {};

								auto b = gameData.chunkSystem.getBlockSafe(rayCastPos.x, rayCastPos.y, rayCastPos.z);
								computeNeighbourChangeUpdate(rayCastPos.x, rayCastPos.y, rayCastPos.z);


							}
							else
							{
								if (soundTimer <= 0)
								{
									soundTimer = 0.2;
									auto sound = getSoundForBlockStepping(raycastBlock->getType());
									AudioEngine::playSound(sound, MINING_BLOCK_SOUND_VOLUME);
								}
								soundTimer -= deltaTime;
							}

						}
					}
					else
					{
						soundTimer = 0;
						gameData.currentBlockBreaking = {};
					}
				}

			

		};

	}
	else
	{
		gameData.currentBlockBreaking = {};
	}
	
#pragma endregion

#pragma region get player positions and stuff

	glm::ivec3 blockPositionPlayer = from3DPointToBlock(gameData.c.position);
	bool underWater = 0;
	auto inBlock = gameData.chunkSystem.getBlockSafe(blockPositionPlayer);
	if (inBlock)
	{
		if (inBlock->getType() == BlockTypes::water)
		{
			underWater = 1;
		}
	}

	// Update player water state and drowning
	player.isInWater = underWater;
	player.isSwimming = underWater;

	// Drowning logic
	if (underWater && player.otherPlayerSettings.gameMode == OtherPlayerSettings::SURVIVAL)
	{
		// Check if head is also underwater
		bool wasInWater = player.isInWater;
		bool headUnderwater = isEntityHeadInWater(player.entity.position, 
			Player::getMaxColliderSize().y, 
			[](glm::ivec2 pos) -> ChunkData* { 
				auto c = gameData.chunkSystem.getChunkSafeFromChunkPos(pos.x, pos.y); 
				return c ? &c->data : nullptr;
			});

		if (headUnderwater)
		{
			// Countdown drowning timer
			player.drowningTimer -= deltaTime;
			
			if (player.drowningTimer <= 0)
			{
				// Start taking drowning damage
				player.drowningDamageTimer -= deltaTime;
				
				if (player.drowningDamageTimer <= 0)
				{
					// Apply drowning damage
					player.life.life -= DROWNING_DAMAGE;
					player.drowningDamageTimer = DROWNING_TICK_INTERVAL;
					player.justRecievedDamageTimer = 0.2f;
					
					// Clamp life to 0
					if (player.life.life < 0) player.life.life = 0;
					
					// Send damage task to server
					Task task;
					task.taskType = Task::clientRecievedDamageLocally;
					task.damage = DROWNING_DAMAGE;					submitTaskClient(task);
				}
			}

			if (!wasInWater && player.otherPlayerSettings.gameMode == OtherPlayerSettings::SURVIVAL)
			{
				AudioEngine::playSound(AudioEngine::waterIn, 0.5f);
			}
		}
		else
		{
			// Head is above water, reset drowning timer
			if (wasInWater && player.otherPlayerSettings.gameMode == OtherPlayerSettings::SURVIVAL)
			{
				AudioEngine::playSound(AudioEngine::waterOut, 0.5f);
			}
			player.drowningTimer = DROWNING_MAX_TIME;
			player.drowningDamageTimer = 0;
		}
	}
else
	{
		// Not in water or in creative mode, reset drowning timer
		player.drowningTimer = DROWNING_MAX_TIME;
		player.drowningDamageTimer = 0;
	}

	glm::vec3 posFloat = {};
	glm::ivec3 posInt = {};
	gameData.c.decomposePosition(posFloat, posInt);

#pragma endregion


	if (!stopMainInput || gameData.isInsideMapView)
	{
		if (platform::isKeyReleased(platform::Button::M))
		{
			if (gameData.isInsideMapView)
			{
				gameData.isInsideMapView = false;
				gameData.mapEngine.close();
			}
			else
			{
				gameData.isInsideMapView = true;
				gameData.mapEngine.open(programData,
					{posInt.x, posInt.z}, gameData.chunkSystem);
			}
		}
	}



#pragma region update lights

	gameData.gameplayFrameProfiler.startSubProfile("lightsSystem");
	gameData.lightSystem.update(gameData.chunkSystem);
	if(!gameData.isInsideMapView){
		static float flick=0; flick+=deltaTime*7.f;
		int pcx = divideChunk(blockPositionPlayer.x);
		int pcz = divideChunk(blockPositionPlayer.z);
		for(int dx=-1;dx<=1;dx++) for(int dz=-1;dz<=1;dz++){
			auto *ch = gameData.chunkSystem.getChunkSafeFromChunkPos(pcx+dx, pcz+dz);
			if(!ch) continue;
			for(int x=0;x<CHUNK_SIZE;x++) for(int z=0;z<CHUNK_SIZE;z++) for(int y=0;y<CHUNK_HEIGHT;y++){
				Block &b = ch->unsafeGet(x,y,z);
				if(b.getType()==BlockTypes::torch || b.getType()==BlockTypes::torchWood || b.getType()==BlockTypes::goblinTorch || b.getType()==BlockTypes::lamp){
					int hash = (x*73856093) ^ (y*19349663) ^ (z*83492791);
					float off = (hash % 100) / 100.f * 6.28f;
					float flicker = 0.85f + 0.15f * sin(flick + off);
					int lvl = (int)(14 * flicker);
					lvl = std::clamp(lvl, 11, 15);
					if(b.getLight()!=lvl){
						b.setLightLevel(lvl);
						gameData.chunkSystem.setChunkAndNeighboursFlagDirtyFromBlockPos(ch->data.x*CHUNK_SIZE+x, ch->data.z*CHUNK_SIZE+z);
					}
				}
			}
		}
	}
	gameData.gameplayFrameProfiler.endSubProfile("lightsSystem");
	if(!gameData.isInsideMapView){
		static float fireTick=0; fireTick+=deltaTime;
		static std::unordered_map<int64_t,float> fireAge;
		static std::unordered_map<int64_t,float> fuelBurn;
		auto posKey = [](int x,int y,int z)->int64_t{ return ((int64_t)(x+30000)<<42) | ((int64_t)(y)<<32) | (uint32_t)(z+30000); };
		const float LIGHTNING_FIRE_CHANCE_STORM = 42.f;
		const float LIGHTNING_FIRE_CHANCE_RAIN  = 18.f;
		if(fireTick>0.55f){
			fireTick=0;
			bool canSpread = true;
			float wetness = programData.weatherState.wetness;
			float rainInt = programData.weatherState.intensity;
			bool isRaining = programData.weatherState.raining && rainInt>0.22f;
			glm::vec2 wind(programData.weatherState.windX, programData.weatherState.windZ);
			float windLen = glm::length(wind);
			glm::vec2 windN = windLen>0.01f ? glm::normalize(wind) : glm::vec2(0.0f);
			int pcx = divideChunk(blockPositionPlayer.x);
			int pcz = divideChunk(blockPositionPlayer.z);
			std::vector<glm::ivec3> toIgnite;
			std::vector<glm::ivec3> toExtinguish;
			std::vector<glm::ivec3> toConsume;
			toIgnite.reserve(64); toExtinguish.reserve(64); toConsume.reserve(64);
			for(int dx=-1;dx<=1;dx++) for(int dz=-1;dz<=1;dz++){
				auto *ch = gameData.chunkSystem.getChunkSafeFromChunkPos(pcx+dx, pcz+dz);
				if(!ch) continue;
				for(int x=0;x<CHUNK_SIZE;x++) for(int z=0;z<CHUNK_SIZE;z++) for(int y=1;y<CHUNK_HEIGHT-1;y++){
					Block &b = ch->unsafeGet(x,y,z);
					if(b.getType()!=BlockTypes::fire) continue;
					int wx = ch->data.x*CHUNK_SIZE+x, wy=y, wz=ch->data.z*CHUNK_SIZE+z;
					int64_t k = posKey(wx,wy,wz);
					float age = fireAge[k] + 0.55f; fireAge[k]=age;
					bool hasWater=false, hasRain=false;
					for(int ddx=-1;ddx<=1 && !hasWater;ddx++) for(int ddz=-1;ddz<=1 && !hasWater;ddz++) for(int ddy=-1;ddy<=1 && !hasWater;ddy++){
						auto nb = gameData.chunkSystem.getBlockSafe(wx+ddx, wy+ddy, wz+ddz);
						if(nb && nb->getType()==BlockTypes::water) hasWater=true;
					}
					if(isRaining){
						auto up = gameData.chunkSystem.getBlockSafe(wx, wy+1, wz);
						bool openSky = !up || up->air();
						if(openSky){
							float rainChance = 35.f + rainInt*25.f + wetness*20.f;
							if((rand()%100) < (int)rainChance) hasRain=true;
						}
						if(wetness>0.6f && (rand()%100)< (int)(wetness*30.f)) hasRain=true;
					}
					if(hasWater || hasRain){
						toExtinguish.push_back({wx,wy,wz});
						fireAge.erase(k);
						continue;
					}
					const int dirs[6][3]={{1,0,0},{-1,0,0},{0,1,0},{0,-1,0},{0,0,1},{0,0,-1}};
					bool hasFuel=false; int fuelCount=0;
					float maxFlam=0.f;
					for(auto &d: dirs){
						auto adj=gameData.chunkSystem.getBlockSafe(wx+d[0], wy+d[1], wz+d[2]);
						if(adj && isFlammable(adj->getType())){ hasFuel=true; fuelCount++; maxFlam = glm::max(maxFlam, getFlammability(adj->getType())); }
						if(adj && adj->getType()==BlockTypes::fire) hasFuel=true;
					}
					if(!hasFuel){
						for(int ox=-1;ox<=1 && !hasFuel;ox++) for(int oz=-1;oz<=1 && !hasFuel;oz++) for(int oy=-1;oy<=1 && !hasFuel;oy++){
							if(ox==0 && oy==0 && oz==0) continue;
							auto nb2 = gameData.chunkSystem.getBlockSafe(wx+ox, wy+oy, wz+oz);
							if(nb2 && isFlammable(nb2->getType())){ hasFuel=true; fuelCount++; }
						}
					}
					float baseLife = hasFuel ? 9.0f : 4.2f;
					if(!hasFuel) baseLife = 3.5f + (rand()%25)/10.f;
					else baseLife += maxFlam*2.0f;
					if(age > baseLife){
						if(!hasFuel || (rand()%100 < 45)){
							toExtinguish.push_back({wx,wy,wz});
							fireAge.erase(k);
							continue;
						}
					}
					if(!canSpread) continue;
					for(auto &d: dirs){
						int fx=wx+d[0], fy=wy+d[1], fz=wz+d[2];
						auto fb = gameData.chunkSystem.getBlockSafe(fx,fy,fz);
						if(!fb || !isFlammable(fb->getType())) continue;
						float flam = getFlammability(fb->getType());
						float windDot = glm::dot(glm::vec2((float)d[0],(float)d[2]), windN);
						float windFactor = 1.0f + windDot*0.38f*windLen;
						windFactor = glm::clamp(windFactor, 0.65f, 1.55f);
						float humidityFactor = 1.0f - wetness*0.68f - rainInt*0.35f;
						humidityFactor = glm::clamp(humidityFactor, 0.18f, 1.0f);
						float upFactor = (d[1]==1) ? 1.18f : (d[1]==-1 ? 0.82f : 1.0f);
						float leafBonus = isAnyLeaves(fb->getType()) ? 1.35f : 1.0f;
						float chance = flam*42.f * windFactor * humidityFactor * upFactor * leafBonus;
						chance = glm::clamp(chance, 1.5f, 78.f);
						int roll = rand()%100;
						if(roll < (int)chance){
							int64_t fk = posKey(fx,fy,fz);
							float bt = getBurnTime(fb->getType());
							float &prog = fuelBurn[fk];
							prog += 0.55f;
							if(prog >= bt*0.55f){
								toConsume.push_back({fx,fy,fz});
								fuelBurn.erase(fk);
							} else if(roll < (int)(chance*0.35f)){
								toIgnite.push_back({fx,fy,fz});
							}
						}
					}
					for(auto &d: dirs){
						int nx=wx+d[0], ny=wy+d[1], nz=wz+d[2];
						auto nb = gameData.chunkSystem.getBlockSafe(nx,ny,nz);
						if(!nb || nb->getType()!=BlockTypes::air) continue;
						bool nearFuel=false; float bestFlam=0.f;
						for(auto &d2: dirs){
							auto adj = gameData.chunkSystem.getBlockSafe(nx+d2[0], ny+d2[1], nz+d2[2]);
							if(adj && isFlammable(adj->getType())){ nearFuel=true; bestFlam = glm::max(bestFlam, getFlammability(adj->getType())); }
						}
						if(!nearFuel) continue;
						float windDot2 = glm::dot(glm::vec2((float)d[0],(float)d[2]), windN);
						float wf = 1.0f + windDot2*0.42f*windLen;
						wf = glm::clamp(wf, 0.55f, 1.62f);
						float humid2 = 1.0f - wetness*0.72f - rainInt*0.40f;
						humid2 = glm::clamp(humid2, 0.15f, 1.0f);
						float up2 = (d[1]==1)?1.22f:(d[1]==-1?0.75f:1.0f);
						float chance2 = bestFlam*36.f * wf * humid2 * up2;
						chance2 = glm::clamp(chance2, 0.8f, 72.f);
						if((rand()%100) < (int)chance2){
							toIgnite.push_back({nx,ny,nz});
						}
					}
				}
			}
			for(auto &p: toConsume){
				Block air; air.setType(BlockTypes::air); air.setLightLevel(0);
				gameData.chunkSystem.placeBlockNoClient(p, air, gameData.lightSystem, nullptr, gameData.interaction, gameData.entityManager);
				fireAge.erase(posKey(p.x,p.y,p.z));
			}
			for(auto &p: toIgnite){
				auto cur = gameData.chunkSystem.getBlockSafe(p.x,p.y,p.z);
				if(!cur || cur->getType()!=BlockTypes::air) continue;
				bool stillFuel=false;
				const int dirs2[6][3]={{1,0,0},{-1,0,0},{0,1,0},{0,-1,0},{0,0,1},{0,0,-1}};
				for(auto &d: dirs2){ auto a2=gameData.chunkSystem.getBlockSafe(p.x+d[0],p.y+d[1],p.z+d[2]); if(a2 && (isFlammable(a2->getType()) || a2->getType()==BlockTypes::fire)) stillFuel=true; }
				if(!stillFuel) continue;
				Block fire; fire.setType(BlockTypes::fire); fire.setLightLevel(13);
				gameData.chunkSystem.placeBlockNoClient(p, fire, gameData.lightSystem, nullptr, gameData.interaction, gameData.entityManager);
				fireAge[posKey(p.x,p.y,p.z)]=0.f;
			}
			for(auto &p: toExtinguish){
				auto cur = gameData.chunkSystem.getBlockSafe(p.x,p.y,p.z);
				if(cur && cur->getType()==BlockTypes::fire){
					Block air; air.setType(BlockTypes::air); air.setLightLevel(0);
					gameData.chunkSystem.placeBlockNoClient(p, air, gameData.lightSystem, nullptr, gameData.interaction, gameData.entityManager);
				}
			}
			if(fireAge.size()>2200){
				auto it = fireAge.begin();
				for(int i=0;i<200 && it!=fireAge.end(); ++i) it = fireAge.erase(it);
			}
		}
		if(programData.weatherState.ambientFlash>0.11f)
		{
			glm::vec3 strikePos;
			if(programData.weatherState.consumeLightningStrike(strikePos))
			{
				float roll = (rand()%100)+ (rand()%100)/100.f;
				float chance = 0.f;
				if(programData.weatherState.type==WeatherType::Weather_Storm) chance = LIGHTNING_FIRE_CHANCE_STORM;
				else if(programData.weatherState.type==WeatherType::Weather_Rain) chance = LIGHTNING_FIRE_CHANCE_RAIN;
				else chance = 4.f;
				chance *= (1.0f - programData.weatherState.wetness*0.55f);
				chance *= (1.0f - programData.weatherState.intensity*0.22f);
				chance = glm::clamp(chance, 0.f, 92.f);
				bool ignite = roll < chance;
				if(ignite){
					int sx = (int)floor(strikePos.x);
					int sz = (int)floor(strikePos.z);
					for(int sy = (int)floor(strikePos.y); sy > 48; sy--)
					{
						auto b = gameData.chunkSystem.getBlockSafe(sx, sy, sz);
						if(!b) continue;
						if(isFlammable(b->getType()))
						{
							Block fire; fire.setType(BlockTypes::fire); fire.setLightLevel(13);
							gameData.chunkSystem.placeBlockNoClient({sx, sy+1, sz}, fire, gameData.lightSystem, nullptr, gameData.interaction, gameData.entityManager);
							fireAge[posKey(sx,sy+1,sz)]=0.f;
							if((rand()%100)<68){
								gameData.chunkSystem.placeBlockNoClient({sx, sy, sz}, fire, gameData.lightSystem, nullptr, gameData.interaction, gameData.entityManager);
								fireAge[posKey(sx,sy,sz)]=0.f;
							}
							for(int dx=-1;dx<=1;dx++) for(int dy=-1;dy<=1;dy++) for(int dz=-1;dz<=1;dz++)
							{
								if(dx==0 && dy==0 && dz==0) continue;
								auto nb = gameData.chunkSystem.getBlockSafe(sx+dx, sy+dy, sz+dz);
								if(nb && isFlammable(nb->getType()))
								{
									float flam = getFlammability(nb->getType());
									float r = (rand()%100)/100.f;
									if(r < flam*0.58f){
										Block f; f.setType(BlockTypes::fire); f.setLightLevel(13);
										gameData.chunkSystem.placeBlockNoClient({sx+dx, sy+dy, sz+dz}, f, gameData.lightSystem, nullptr, gameData.interaction, gameData.entityManager);
										fireAge[posKey(sx+dx,sy+dy,sz+dz)]=0.f;
									}
								}
							}
							break;
						}
						if(!b->air() && b->getType()!=BlockTypes::fire) break;
					}
				}
			}
		}
	}
	// dano fogo player
	{
		auto p = from3DPointToBlock(player.entity.position + glm::dvec3(0,0.8,0));
		auto b = gameData.chunkSystem.getBlockSafe(p.x,p.y,p.z);
		auto b2 = gameData.chunkSystem.getBlockSafe(p.x,p.y-1,p.z);
		if((b && b->getType()==BlockTypes::fire) || (b2 && b2->getType()==BlockTypes::fire)){
			static float fireDmg=0; fireDmg+=deltaTime;
			if(fireDmg>0.6f){ fireDmg=0; dealDamageToLocalPlayer(2); gameData.hitLensDirt=0.6f; }
		}
	}
#pragma endregion

	programData.renderer.entityRenderer.itemEntitiesToRender.push_back({gameData.entityTest});


	//
#pragma region weather and time

	static float dayTime = 0.25;

	//the /time command can override this (setDayTimeGlobally)
	dayTime = globalDayTime;
	programData.renderer.sunPos = calculateSunPosition(dayTime);
	
	//dayTime += deltaTime * 0.05f;
	if (dayTime > 0) { dayTime -= (int)dayTime; }
	globalDayTime = dayTime;

	// Update weather from global state
	{
		int wType = getWeatherGlobally();
		WeatherType wt = (WeatherType)wType;
		std::cerr << "[weather-debug] gameplayFrame read globalWeatherType=" << wType << " current programData.weatherState.type=" << (int)programData.weatherState.type << std::endl;
		if (programData.weatherState.type != wt)
		{
			std::cerr << "[weather-debug] applying weather change to programData.weatherState.type=" << (int)wt << std::endl;
			programData.weatherState.setWeather(wt);
		}
		programData.weatherState.update(deltaTime, glm::vec3(gameData.c.position), gameData.lastFrameInWater);
		setWetnessGlobally(programData.weatherState.wetness);
		globalWeatherStatePtr = &programData.weatherState;
		programData.renderer.wetness = programData.weatherState.wetness;

		// Weather sounds
		{
			static float rainSoundTimer = 0.f;
			static float windSoundTimer = 0.f;

			// Rain ambient: play periodically while raining
			if (programData.weatherState.raining && programData.weatherState.intensity > 0.3f)
			{
				rainSoundTimer += deltaTime;
				if (rainSoundTimer > 2.5f)
				{
					rainSoundTimer = 0.f;
					AudioEngine::playSound(AudioEngine::rainAmbient, programData.weatherState.intensity * 0.5f);
				}
			}
			else
			{
				rainSoundTimer = 2.f; // delay before first play
			}

			// Thunder: play when lightning flashes
			if (programData.weatherState.ambientFlash > 0.8f)
			{
				AudioEngine::playSound(AudioEngine::thunder, 0.8f);
			}

			// Wind: play periodically in storms/snow
			if (programData.weatherState.type == WeatherType::Weather_Storm || programData.weatherState.type == WeatherType::Weather_Snow)
			{
				windSoundTimer += deltaTime;
				if (windSoundTimer > 4.f)
				{
					windSoundTimer = 0.f;
					AudioEngine::playSound(AudioEngine::wind, programData.weatherState.intensity * 0.4f);
				}
			}
			else
			{
				windSoundTimer = 3.f;
			}
		}
	}

#pragma endregion


#pragma region chunks and rendering

	{
		gameData.gameplayFrameProfiler.startSubProfile("chunkSystem");
		gameData.chunkSystem.update(blockPositionPlayer, deltaTime, gameData.undoQueue,
			gameData.lightSystem, gameData.interaction, threadPoolForChunkBaking, programData.renderer, gameData.entityManager);
		gameData.gameplayFrameProfiler.endSubProfile("chunkSystem");
	}


#pragma region underwater water drops
	if (gameData.lastFrameInWater)
	{
		if (!underWater)
		{
			gameData.dropsStrength = 4;
			AudioEngine::playSound(AudioEngine::waterExit, 0.5f);
		}
	}
	else
	{
		if (underWater)
		{
			AudioEngine::playSound(AudioEngine::waterSplash, 0.6f);
		}
	}
	
	if (underWater)
	{
		gameData.dropsStrength = 0;
	}
	gameData.lastFrameInWater = underWater;

	gameData.dropsStrength -= deltaTime;
	if (gameData.dropsStrength < 0) { gameData.dropsStrength = 0; }

	float finalDropStrength = std::min(1.f, gameData.dropsStrength/3.f);
#pragma endregion

#pragma region water swim sounds
	{
		static float swimTimer=0;
		if(underWater && isPlayerMovingSpeed){
			swimTimer -= deltaTime;
			if(swimTimer<=0){
				AudioEngine::playSound(AudioEngine::waterSwim, 0.45f);
				swimTimer = 0.65f;
			}
		}else{
			swimTimer = 0.35f;
		}
	}
#pragma endregion


	if(w != 0 && h != 0 && !gameData.isInsideMapView)
	{

		gameData.gameplayFrameProfiler.startSubProfile("rendering");

		//programData.renderer.render(data, gameData.c, programData.texture);
		bool showHand = gameData.showUI && gameData.cameraMode == 0;
		programData.renderer.renderFromBakedData(gameData.sunShadow,gameData.chunkSystem, 
			gameData.c, programData, programData.blocksLoader, gameData.entityManager,
			programData.modelsManager, 
			gameData.adaptiveExposure, gameData.showLightLevels,
			gameData.point, underWater, w, h, deltaTime, dayTime, gameData.currentSkinBindlessTexture,
			gameData.handHit, isPlayerMovingSpeed, gameData.playerFOVHandTransform,
			gameData.currentItemSelected, finalDropStrength, 
			showHand, gameData.playersConnectionData
			);


		if (gameData.currentBlockBreaking.breaking)
		{
			programData.renderer.renderDecal(rayCastPos, gameData.c, *raycastBlock,
				programData, 1-(gameData.currentBlockBreaking.timer / gameData.currentBlockBreaking.totalTime));
		}


		gameData.c.lastFrameViewProjMatrix =
			gameData.c.getProjectionMatrix() * gameData.c.getViewMatrix();

		gameData.gameplayFrameProfiler.endSubProfile("rendering");
	}
#pragma endregion


#pragma region drop entities that are too far

	gameData.entityManager.dropEntitiesThatAreTooFar({blockPositionPlayer.x,blockPositionPlayer.z},
		gameData.chunkSystem.squareSize);


#pragma endregion


//steppings sound
#pragma region steppings sounds

	{
		static float timer = 0;

		const float shortestTimer = 0.3;
		const float longestTimer = 0.7;

		//player.entity.forces.colidesBottom() && 
		if (isPlayerMovingSpeed)
		{
			auto blockPos = from3DPointToBlock(player.entity.position - glm::dvec3(0, 0.1, 0));
			auto block = gameData.chunkSystem.getBlockSafe(blockPos.x, blockPos.y, blockPos.z);

			if (block)
			{
				auto sound = getSoundForBlockStepping(block->getType());

				if (sound)
				{
					if (timer <= 0)
					{
						timer = glm::mix(longestTimer, shortestTimer, isPlayerMovingSpeed);
						AudioEngine::playSound(sound, STEPPING_SOUND_VOLUME);
					}
					timer -= deltaTime;
				}
				else
				{
					timer = 0;
				}
			}
			else
			{
				timer = 0;
			}
		}
		else
		{
			timer = 0;
		}
	}

#pragma endregion

	
#pragma region debug and gyzmos stuff
	
	if (!gameData.isInsideMapView)
	{

		programData.GPUProfiler.startSubProfile("Debug rendering");


		programData.pointDebugRenderer.renderCubePoint(gameData.c, gameData.point);

		if (gameData.renderBox)
		{
			//programData.gyzmosRenderer.drawCube(from3DPointToBlock(point));
			programData.gyzmosRenderer.drawCube(gameData.point);

			if (gameData.pointSize != glm::ivec3{})
			{
				programData.gyzmosRenderer.drawCube(from3DPointToBlock(gameData.point + glm::ivec3(gameData.pointSize)));
				programData.gyzmosRenderer.drawCube(from3DPointToBlock(gameData.point + glm::ivec3(gameData.pointSize) - glm::ivec3(1, 0, 0)));
				programData.gyzmosRenderer.drawCube(from3DPointToBlock(gameData.point + glm::ivec3(gameData.pointSize) - glm::ivec3(0, 1, 0)));
				programData.gyzmosRenderer.drawCube(from3DPointToBlock(gameData.point + glm::ivec3(gameData.pointSize) - glm::ivec3(0, 0, 1)));
			}
		}


		auto drawPlayerBox = [&](glm::dvec3 pos, glm::vec3 boxSize)
		{
			programData.gyzmosRenderer.drawLine(pos + glm::dvec3(boxSize.x / 2, 0, boxSize.z / 2),
				pos + glm::dvec3(boxSize.x / 2, 0, -boxSize.z / 2));
			programData.gyzmosRenderer.drawLine(pos + glm::dvec3(boxSize.x / 2, 0, -boxSize.z / 2),
				pos + glm::dvec3(-boxSize.x / 2, 0, -boxSize.z / 2));
			programData.gyzmosRenderer.drawLine(pos + glm::dvec3(-boxSize.x / 2, 0, -boxSize.z / 2),
				pos + glm::dvec3(-boxSize.x / 2, 0, boxSize.z / 2));
			programData.gyzmosRenderer.drawLine(pos + glm::dvec3(-boxSize.x / 2, 0, boxSize.z / 2),
				pos + glm::dvec3(boxSize.x / 2, 0, boxSize.z / 2));

			programData.gyzmosRenderer.drawLine(pos + glm::dvec3(boxSize.x / 2, boxSize.y, boxSize.z / 2),
				pos + glm::dvec3(boxSize.x / 2, boxSize.y, -boxSize.z / 2));
			programData.gyzmosRenderer.drawLine(pos + glm::dvec3(boxSize.x / 2, boxSize.y, -boxSize.z / 2),
				pos + glm::dvec3(-boxSize.x / 2, boxSize.y, -boxSize.z / 2));
			programData.gyzmosRenderer.drawLine(pos + glm::dvec3(-boxSize.x / 2, boxSize.y, -boxSize.z / 2),
				pos + glm::dvec3(-boxSize.x / 2, boxSize.y, boxSize.z / 2));
			programData.gyzmosRenderer.drawLine(pos + glm::dvec3(-boxSize.x / 2, boxSize.y, boxSize.z / 2),
				pos + glm::dvec3(boxSize.x / 2, boxSize.y, boxSize.z / 2));

			programData.gyzmosRenderer.drawLine(pos + glm::dvec3(boxSize.x / 2, 0, boxSize.z / 2),
				pos + glm::dvec3(boxSize.x / 2, boxSize.y, boxSize.z / 2));
			programData.gyzmosRenderer.drawLine(pos + glm::dvec3(boxSize.x / 2, 0, -boxSize.z / 2),
				pos + glm::dvec3(boxSize.x / 2, boxSize.y, -boxSize.z / 2));
			programData.gyzmosRenderer.drawLine(pos + glm::dvec3(-boxSize.x / 2, 0, -boxSize.z / 2),
				pos + glm::dvec3(-boxSize.x / 2, boxSize.y, -boxSize.z / 2));
			programData.gyzmosRenderer.drawLine(pos + glm::dvec3(-boxSize.x / 2, 0, boxSize.z / 2),
				pos + glm::dvec3(-boxSize.x / 2, boxSize.y, boxSize.z / 2));
		};

		auto drawBox = [&](glm::dvec3 pos, glm::vec3 boxSize)
		{
			programData.gyzmosRenderer.drawLine(pos,
				pos + glm::dvec3(boxSize.x, 0, 0));

			programData.gyzmosRenderer.drawLine(pos,
				pos + glm::dvec3(0, 0, boxSize.z));

			programData.gyzmosRenderer.drawLine(pos + glm::dvec3(boxSize.x, 0, 0),
				pos + glm::dvec3(boxSize.x, 0, boxSize.z));

			programData.gyzmosRenderer.drawLine(pos + glm::dvec3(0, 0, boxSize.z),
				pos + glm::dvec3(boxSize.x, 0, boxSize.z));



			programData.gyzmosRenderer.drawLine(pos + glm::dvec3(0,boxSize.y,0),
				pos + glm::dvec3(boxSize.x, boxSize.y, 0));

			programData.gyzmosRenderer.drawLine(pos + glm::dvec3(0, boxSize.y, 0),
				pos + glm::dvec3(0, boxSize.y, boxSize.z));

			programData.gyzmosRenderer.drawLine(pos + glm::dvec3(boxSize.x, boxSize.y, 0),
				pos + glm::dvec3(boxSize.x, boxSize.y, boxSize.z));

			programData.gyzmosRenderer.drawLine(pos + glm::dvec3(0, boxSize.y, boxSize.z),
				pos + glm::dvec3(boxSize.x, boxSize.y, boxSize.z));


			programData.gyzmosRenderer.drawLine(pos,
				pos + glm::dvec3(0, boxSize.y, 0));

			programData.gyzmosRenderer.drawLine(pos + glm::dvec3(boxSize.x, 0, 0),
				pos + glm::dvec3(boxSize.x, boxSize.y, 0));

			programData.gyzmosRenderer.drawLine(pos + glm::dvec3(0, 0, boxSize.z),
				pos + glm::dvec3(0, boxSize.y, boxSize.z));

			programData.gyzmosRenderer.drawLine(pos + glm::dvec3(boxSize.x, 0, boxSize.z),
				pos + glm::dvec3(boxSize.x, boxSize.y, boxSize.z));
		};

		if (player.otherPlayerSettings.gameMode == OtherPlayerSettings::CREATIVE)
		{
			//todo sort the chunks in the chunk system once and than keep that vector because we need it
			int maxCount = 100;
			for (auto &c : gameData.chunkSystem.loadedChunks)
			{
				if (c)
				{

					for (auto &b : c->blockData.baseBlocks)
					{

						glm::ivec3 pos = fromHashValueToBlockPosinChunk(b.first);
						glm::vec3 size = {b.second.sizeX,b.second.sizeY,b.second.sizeZ};

						if (size.x != 0 && size.y != 0 && size.z != 0)
						{
							maxCount--;

							glm::dvec3 posD = pos + glm::ivec3(c->data.x * CHUNK_SIZE,0, c->data.z * CHUNK_SIZE);
							posD += glm::dvec3(0.5, -0.5, 0.5);
							posD += glm::ivec3(b.second.offsetX, b.second.offsetY, b.second.offsetZ);

							drawBox(posD, size);
						}

						if (maxCount <= 0)
						{
							break;
						}
					}


				}

				if (maxCount <= 0)
				{
					break;
				}
			}
		};


		if (gameData.renderColliders)
		{

			gameData.entityManager.renderColiders(programData.pointDebugRenderer, programData.gyzmosRenderer, 
				gameData.c);

		}


		//programData.gyzmosRenderer.drawLine(
		//	gameData.point,
		//	glm::vec3(gameData.point) + glm::vec3(gameData.pointSize));

		if (gameData.renderPlayerPos)
		{
			programData.gyzmosRenderer.drawCube(blockPositionPlayer);
		}

		if (gameData.showChunkBorders)
		{
			int pcx = (int)std::floor((double)blockPositionPlayer.x / CHUNK_SIZE);
			int pcz = (int)std::floor((double)blockPositionPlayer.z / CHUNK_SIZE);
			for(int dx=-2;dx<=2;dx++) for(int dz=-2;dz<=2;dz++){
				int cx = pcx + dx;
				int cz = pcz + dz;
				double x0 = cx * CHUNK_SIZE - 0.5;
				double z0 = cz * CHUNK_SIZE - 0.5;
				double x1 = x0 + CHUNK_SIZE;
				double z1 = z0 + CHUNK_SIZE;
				glm::dvec3 p00 = {x0, 0, z0};
				glm::dvec3 p10 = {x1, 0, z0};
				glm::dvec3 p01 = {x0, 0, z1};
				glm::dvec3 p11 = {x1, 0, z1};
				glm::dvec3 p00h = {x0, (double)CHUNK_HEIGHT, z0};
				glm::dvec3 p10h = {x1, (double)CHUNK_HEIGHT, z0};
				glm::dvec3 p01h = {x0, (double)CHUNK_HEIGHT, z1};
				glm::dvec3 p11h = {x1, (double)CHUNK_HEIGHT, z1};
				auto col = (dx==0 && dz==0) ? glm::vec3(1,0,0) : glm::vec3(1,1,0);
				programData.gyzmosRenderer.drawLine(p00, p10);
				programData.gyzmosRenderer.drawLine(p10, p11);
				programData.gyzmosRenderer.drawLine(p11, p01);
				programData.gyzmosRenderer.drawLine(p01, p00);
				programData.gyzmosRenderer.drawLine(p00, p00h);
				programData.gyzmosRenderer.drawLine(p10, p10h);
				programData.gyzmosRenderer.drawLine(p01, p01h);
				programData.gyzmosRenderer.drawLine(p11, p11h);
			}
		}

		for(auto &par : gameData.breakParticles){
			glm::ivec3 ip = glm::floor(par.pos);
			glm::vec3 fp = glm::vec3(par.pos - glm::dvec3(ip));
			programData.gyzmosRenderer.drawCube(ip, fp, glm::vec3(0.14f));
		}
		if(programData.weatherState.intensity>0.01f){
			for(auto &p : programData.weatherState.particles){
				if(p.life<=0) continue;
				glm::dvec3 s = glm::dvec3(p.position);
				glm::dvec3 e = glm::dvec3(p.position) + glm::dvec3(p.velocity)*0.045;
				if(programData.weatherState.type==WeatherType::Weather_Snow) programData.gyzmosRenderer.drawCube(glm::ivec3(s), glm::vec3(0), glm::vec3(0.12f));
				else programData.gyzmosRenderer.drawLine(s, e);
			}
			for(size_t i=1;i<programData.weatherState.currentBolt.points.size();i++){
				programData.gyzmosRenderer.drawLine(programData.weatherState.currentBolt.points[i-1], programData.weatherState.currentBolt.points[i]);
			}
		}

		programData.gyzmosRenderer.render(gameData.c, posInt, posFloat);
		if(programData.weatherState.ambientFlash>0.01f){
			programData.ui.renderer2d.renderRectangle({0,0, programData.ui.renderer2d.windowW, programData.ui.renderer2d.windowH}, glm::vec4(1,1,1, programData.weatherState.ambientFlash*0.5f));
		}

		programData.GPUProfiler.endSubProfile("Debug rendering");
	}

#pragma endregion


	auto centerChunk = gameData.chunkSystem.getChunkSafeFromBlockPos(posInt.x, posInt.z);

	if (gameData.showF3Debug && !gameData.isInsideMapView)
	{
		auto &r2d = programData.ui.renderer2d;
		glm::dvec3 p = gameData.entityManager.localPlayer.entity.position;
		glm::ivec3 bpos = from3DPointToBlock(p);
		glm::ivec2 chunkPos = { (int)std::floor((float)bpos.x / CHUNK_SIZE), (int)std::floor((float)bpos.z / CHUNK_SIZE)};
		int chunkSection = (int)std::floor(bpos.y / 16.f);
		int facing = gameData.c.getViewDirectionRotation();
		const char* facingStr = facing==0?"north (-Z)":facing==1?"west (-X)":facing==2?"south (+Z)":"east (+X)";
		std::string l1 = "fnxCraft F3 | FPS: " + std::to_string(programData.currentFps) + " | " + (gameData.cameraMode==0?"First":gameData.cameraMode==1?"Third Back":"Third Front");
		std::string l2 = "XYZ: " + std::to_string(p.x).substr(0,7) + " / " + std::to_string(p.y).substr(0,7) + " / " + std::to_string(p.z).substr(0,7);
		std::string l3 = "Block: " + std::to_string(bpos.x) + " " + std::to_string(bpos.y) + " " + std::to_string(bpos.z) + " (Section " + std::to_string(chunkSection) + ")";
		std::string l4 = "Chunk: " + std::to_string(chunkPos.x) + " " + std::to_string(chunkPos.y) + " [" + std::to_string(bpos.x - chunkPos.x*CHUNK_SIZE) + " " + std::to_string(bpos.z - chunkPos.y*CHUNK_SIZE) + "]";
		std::string l5 = "Facing: " + std::string(facingStr) + " (" + std::to_string(gameData.c.viewDirection.x).substr(0,5) + ", " + std::to_string(gameData.c.viewDirection.z).substr(0,5) + ")";
		std::string l6 = "";
		if(centerChunk){ l6 = "Biome veg: " + std::to_string(centerChunk->data.vegetation).substr(0,5); } else l6 = "Biome: loading";
		auto blk = gameData.chunkSystem.getBlockSafe(bpos.x,bpos.y,bpos.z);
		std::string l7 = blk ? ("Light sky:" + std::to_string((int)blk->getSkyLight()) + " block:" + std::to_string((int)blk->getLight())) : "Light: -";
		size_t entCount = gameData.entityManager.players.size() + gameData.entityManager.zombies.size() + gameData.entityManager.pigs.size() + gameData.entityManager.goblins.size();
		std::string l8 = "Entities: " + std::to_string(entCount) + " | Mem chunks: " + std::to_string(gameData.chunkSystem.loadedChunks.size());
		std::string l9 = "Sim distance: " + std::to_string(getShadingSettings().viewDistance) + " chunks";
		r2d.renderRectangle({5,5, 360, 178}, {0,0,0,0.55});
		float y = 28;
		auto drawL = [&](std::string s, float yy){ r2d.renderText({10, yy}, s.c_str(), programData.ui.font, Colors_White, 14); };
		drawL(l1, y); y+=16;
		drawL(l2, y); y+=16;
		drawL(l3, y); y+=16;
		drawL(l4, y); y+=16;
		drawL(l5, y); y+=16;
		drawL(l6, y); y+=16;
		drawL(l7, y); y+=16;
		drawL(l8, y); y+=16;
		drawL(l9, y); y+=16;
		if (raycastBlock && raycastBlock->isCrop())
		{
			bool cropWater = false;
			for(int dx=-4;dx<=4 && !cropWater;dx++) for(int dz=-4;dz<=4 && !cropWater;dz++){
				auto *wb = gameData.chunkSystem.getBlockSafe(rayCastPos.x+dx, rayCastPos.y-1, rayCastPos.z+dz);
				if(wb && wb->getType()==BlockTypes::water) cropWater = true;
			}
			std::string cropLine = "Crop: stage " + std::to_string((int)raycastBlock->getCropStage()) + "/7"
				+ " | light sky:" + std::to_string((int)raycastBlock->getSkyLight())
				+ " block:" + std::to_string((int)raycastBlock->getLight())
				+ " | water:" + (cropWater ? "yes" : "no");
			drawL(cropLine, y);
		}
	}

	if (gameData.bowCharging && !gameData.isInsideMapView && !gameData.isInsideChat)
	{
		auto &r2d = programData.ui.renderer2d;
		float pct = std::clamp(gameData.bowCharge / 0.9f, 0.f, 1.f);
		int w = r2d.windowW;
		int h = r2d.windowH;
		int barW = 200;
		int barH = 12;
		int x = w/2 - barW/2;
		int y = h/2 + 40;
		r2d.renderRectangle({x-2, y-2, barW+4, barH+4}, {0,0,0,0.6});
		r2d.renderRectangle({x, y, barW, barH}, {0.3,0.3,0.3,0.8});
		glm::vec4 col = {0.2f + pct*0.8f, 0.8f - pct*0.5f, 0.2f, 1.f};
		r2d.renderRectangle({x, y, (int)(barW * pct), barH}, col);
	}
	if (gameData.fishingManager.bobber.active && !gameData.isInsideMapView && !gameData.isInsideChat)
	{
		auto &r2d = programData.ui.renderer2d;
		int w = r2d.windowW;
		int h = r2d.windowH;
		std::string stateStr;
		glm::vec4 col{0.2f,0.6f,0.9f,1.f};
		float pct=0;
		switch(gameData.fishingManager.bobber.state){
			case FishingState::Waiting: stateStr="Esperando peixe..."; pct=gameData.fishingManager.bobber.waterTimer/3.f; col={0.3f,0.5f,0.7f,1.f}; break;
			case FishingState::Nibbling: stateStr="Peixe se aproximando..."; pct=gameData.fishingManager.bobber.fishApproach; col={0.9f,0.7f,0.2f,1.f}; break;
			case FishingState::Biting: stateStr="RECOLHA AGORA! [Clique]"; pct=gameData.fishingManager.bobber.progress; col={0.9f,0.2f,0.2f,1.f}; break;
			default: stateStr="Pescando..."; break;
		}
		int barW=220; int barH=14; int x=w/2-barW/2; int y=h/2+70;
		r2d.renderRectangle({x-2,y-2,barW+4,barH+18}, {0,0,0,0.65});
		r2d.renderText({w/2, y-10}, stateStr.c_str(), programData.ui.font, Colors_White, 14.f, -1);
		r2d.renderRectangle({x,y,barW,barH}, {0.25f,0.25f,0.28f,0.9f});
		r2d.renderRectangle({x,y,(int)(barW*std::clamp(pct,0.f,1.f)),barH}, col);
		if(gameData.fishingManager.bobber.state==FishingState::Biting){
			float pulse = (sin(gameData.fishingManager.bobber.stateTimer*12.f)*0.5f+0.5f);
			r2d.renderRectangle({x-1,y-1,barW+2,barH+2}, {1,1,0.3f,0.3f+ pulse*0.3f});
		}
	}

#pragma region imgui
	
	bool terminate = false;

#if REMOVE_IMGUI == 0

	if (programData.showImgui)
	{
		gameData.gameplayFrameProfiler.startSubProfile("imgui");

		//if (ImGui::Begin("camera controll", &gameData.escapePressed))
		ImGui::PushStyleColor(ImGuiCol_WindowBg, {26/255.f,26/255.f,26/255.f,0.5f});
		if (ImGui::Begin("client controll"))
		{

			ImGui::SliderFloat("Day time", &dayTime, 0, 1);

			if (centerChunk)
			{
				ImGui::Text("Vegetaion: %f", centerChunk->data.vegetation);
				ImGui::Text("CellValue X: %d", centerChunk->data.regionCenterX);
				ImGui::Text("CellValue Z: %d", centerChunk->data.regionCenterZ);
			}
	
			int l = player.life.life;
			ImGui::SliderInt("Player Life", &l, 0, 20);
			player.life.life = l;


			int timerMsSatiety = player.effects.allEffects[Effects::Saturated].timerMs;

			ImGui::Text("Satiety Ms: %d", timerMsSatiety);
			ImGui::Text("Satiety s : %d", timerMsSatiety/1000);

			if (ImGui::CollapsingHeader("Camera stuff",
				ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_FramePadding))
			{
				ImGui::Checkbox("Colidable", &gameData.colidable);

				ImGui::DragScalarN("Player Body pos", ImGuiDataType_Double,
					&gameData.entityManager.localPlayer.entity.position[0], 3, 0.01);
				gameData.entityManager.localPlayer.entity.lastPosition 
					= gameData.entityManager.localPlayer.entity.position;

				ImGui::Text("Entity pos: %lf, %lf, %lf", player.entity.position.x, player.entity.position.y, player.entity.position.z);
				ImGui::Text("Entity velocity magnitude: %f", glm::length(player.entity.forces.velocity));
				ImGui::Text("camera float: %f, %f, %f", posFloat.x, posFloat.y, posFloat.z);
				ImGui::Text("camera int: %d, %d, %d", posInt.x, posInt.y, posInt.z);
				ImGui::Text("camera view: %f, %f, %f", gameData.c.viewDirection.x, gameData.c.viewDirection.y, gameData.c.viewDirection.z);

				ImGui::Text("Chunk: %d, %d", divideChunk(posInt.x), divideChunk(posInt.z));

				//ImGui::DragScalarN("Point pos", ImGuiDataType_Double, &point[0], 3, 1);
				ImGui::DragInt3("Point pos", &gameData.point[0]);
				ImGui::DragInt3("Point size", &gameData.pointSize[0]);
				ImGui::Checkbox("Render Box", &gameData.renderBox);
				ImGui::Checkbox("Render Player Pos", &gameData.renderPlayerPos);
				ImGui::Checkbox("Render Coliders", &gameData.renderColliders);

				ImGui::DragScalarN("Entity pos test", ImGuiDataType_Double,
					&gameData.entityTest[0], 3, 0.1);

				ImGui::DragFloat("camera speed", &moveSpeed);

			}

			ImGui::NewLine();

			gameData.pointSize = glm::clamp(gameData.pointSize, glm::ivec3(0, 0, 0), glm::ivec3(64, 64, 64));

			if (ImGui::CollapsingHeader("Light Stuff",
				ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_FramePadding))
			{	
				ImGui::Checkbox("showLightLevels", &gameData.showLightLevels);
				ImGui::SliderInt("skyLightIntensity", &gameData.skyLightIntensity, 0, 15);
				ImGui::SliderFloat("metallic", &programData.renderer.metallic, 0, 1);
				ImGui::SliderFloat("roughness", &programData.renderer.roughness, 0, 1);
				ImGui::SliderFloat("exposure", &gameData.adaptiveExposure.currentExposure, 0.001, 10);
				ImGui::Combo("Tonemapper", &programData.renderer.defaultShader.
					shadingSettings.tonemapper, "ACES\0AgX\0ZCAM\0");

				ImGui::SliderFloat3("Sky pos", &programData.renderer.sunPos[0], -1, 1);

				ImGui::ColorPicker3("water color", 
					&programData.renderer.defaultShader.shadingSettings.waterColor[0]);

				ImGui::ColorPicker3("under water color",
					&programData.renderer.defaultShader.shadingSettings.underWaterColor[0]);

				ImGui::SliderFloat("underwaterDarkenStrength",
					&programData.renderer.defaultShader.shadingSettings.underwaterDarkenStrength,
					0, 1);

				ImGui::SliderFloat("underwaterDarkenDistance",
					&programData.renderer.defaultShader.shadingSettings.underwaterDarkenDistance,
					0, 40);

				ImGui::SliderFloat("fogGradientUnderWater",
					&programData.renderer.defaultShader.shadingSettings.fogGradientUnderWater,
					0, 10);

				ImGui::SliderFloat("normal fog",
					&getShadingSettings().fogGradient,
					0, 100);

				if (glm::length(programData.renderer.sunPos[0]) != 0)
				{
					programData.renderer.sunPos = 
						glm::normalize(programData.renderer.sunPos);
				}
				else
				{
					programData.renderer.sunPos = glm::vec3(0, -1, 0);
				}
			}

			auto b = gameData.chunkSystem.getBlockSafe(gameData.point);
			if (b) ImGui::Text("Box Light Value: %d", b->getSkyLight());

			
			ImGui::Text("fps: %d", programData.currentFps);
			if (ImGui::Button("Exit game"))
			{
				terminate = true;
			}

			ImGui::NewLine();

			
			if (ImGui::CollapsingHeader("Sky stuff",
				ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_FramePadding))
			{

				programData.skyBoxLoaderAndDrawer.skyConfig.a1;

				ImGui::ColorEdit3("Sky", &programData.skyBoxLoaderAndDrawer.skyConfig.a1[0]);
				ImGui::ColorEdit3("Ground", &programData.skyBoxLoaderAndDrawer.skyConfig.a2[0]);
				ImGui::SliderFloat("Density", &programData.skyBoxLoaderAndDrawer.skyConfig.g, 0, 1);
					
				if(ImGui::Button("Refresh"))
				{
					programData.skyBoxLoaderAndDrawer.createSkyTextures();
				}

			}

			if (ImGui::CollapsingHeader("Load Save Stuff",
				ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_FramePadding))
			{

				static char fileBuff[256] = RESOURCES_PATH "gameData/structures/test.structure";


				if (ImGui::Button("OPEN FILE DIALOGUE"))
				{
					ShowOpenFileDialog(0, fileBuff, sizeof(fileBuff),
						RESOURCES_PATH "gameData/structures/",
						".structure");
				}


				ImGui::InputText("File:", fileBuff, sizeof(fileBuff));

				if (ImGui::Button("Save structure"))
				{
					std::vector<unsigned char> data;
					data.resize(sizeof(StructureData) + 2 * sizeof(BlockType) * gameData.pointSize.x * gameData.pointSize.y * gameData.pointSize.z);

					StructureData *s = (StructureData *)data.data();

					s->sizeNotRotated = gameData.pointSize;
					s->unused = 0;

					for (int x = 0; x < gameData.pointSize.x; x++)
						for (int z = 0; z < gameData.pointSize.z; z++)
							for (int y = 0; y < gameData.pointSize.y; y++)
							{
								glm::ivec3 pos = gameData.point + glm::ivec3(x, y, z);

								auto rez = gameData.chunkSystem.getBlockSafe(pos.x, pos.y, pos.z);

								if (rez)
								{
									s->unsafeGet(x, y, z) = *rez;
									s->unsafeGet(x, y, z).lightLevel = 0;
								}
								else
								{
									s->unsafeGet(x, y, z) = {};
								}

							}
					sfs::writeEntireFile(s, data.size(), fileBuff);
				}

				if (ImGui::Button("Load structure"))
				{
					std::vector<char> data;
					if (sfs::readEntireFile(data, fileBuff) ==
						sfs::noError)
					{
						int rotation = 0;

						StructureData *s = (StructureData *)data.data();
						auto size = s->getSizeRotated(rotation);

						for (int x = 0; x < size.x; x++)
							for (int z = 0; z < size.z; z++)
								for (int y = 0; y < size.y; y++)
								{
									glm::ivec3 pos = gameData.point + glm::ivec3(x, y, z);

									Block block = s->unsafeGetRotated(x, y, z, rotation);
									block.rotate(rotation);
									

									//todo implement the bulk version...
									gameData.chunkSystem.placeBlockByClientForce(pos,
										block, gameData.undoQueue,
										gameData.lightSystem, gameData.entityManager);

								}

						gameData.pointSize = size;
					}


				}
			}


			ImGui::NewLine();

			if (ImGui::CollapsingHeader("Sun Shadow Map",
				ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_FramePadding))
			{
				ImGui::Image((void *)gameData.sunShadow.shadowTexturePreview.color, {256, 256}, 
					{0, 1}, {1, 0});
				//ImGui::Image((void *)gameData.sunShadow.shadowMap.depth, {256, 256},
				//	{0, 1}, {1, 0});
			}

			if (ImGui::CollapsingHeader("HBAO Map",
				ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_FramePadding))
			{
				ImGui::Image((void *)programData.renderer.fboHBAO.color, {256, 256},
					{0, 1}, {1, 0});
			}

			if (ImGui::CollapsingHeader("Sky Map",
				ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_FramePadding))
			{
				ImGui::Image((void *)programData.renderer.fboSkyBox.color, {256, 256},
					{0, 1}, {1, 0});
			}

			ImGui::NewLine();

			if (ImGui::CollapsingHeader("Screen space pos",
				ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_FramePadding))
			{
				ImGui::Image((void *)programData.renderer.fboLastFramePositions.color, {256, 256},
					{0, 1}, {1, 0});
			}

			if (ImGui::CollapsingHeader("Last frame color",
				ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_FramePadding))
			{
				ImGui::Image((void *)programData.renderer.fboLastFrame.color, {256, 256},
					{0, 1}, {1, 0});
			}

			if (ImGui::CollapsingHeader("Sun for SSGR",
				ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_FramePadding))
			{
				ImGui::Image((void *)programData.renderer.fboSunForGodRays.color, {256, 256},
					{0, 1}, {1, 0});

				ImGui::Image((void *)programData.renderer.fboSunForGodRaysSecond.color, {256, 256},
					{0, 1}, {1, 0});
			}

			if (ImGui::CollapsingHeader("Filtered bloom color",
				ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_FramePadding))
			{
				ImGui::Image((void *)programData.renderer.fboMain.fourthColor, {256, 256},
					{0, 1}, {1, 0});

				ImGui::Image((void *)programData.renderer.bluredColorBuffer[0], {256, 256},
					{0, 1}, {1, 0});

				ImGui::Image((void *)programData.renderer.bluredColorBuffer[1], {256, 256},
					{0, 1}, {1, 0});

				ImGui::SliderFloat("Multiplier", &getShadingSettings().bloomMultiplier, 0, 7);
				ImGui::SliderFloat("Tresshold", &getShadingSettings().bloomTresshold, 0.0001, 10);
			}

			if (ImGui::CollapsingHeader("Chunk system",
				ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_FramePadding))
			{

				static int blockSize = 25;

				ImGui::SliderInt("Size", &blockSize, 10, 100);

				ImVec4 colNotLoaded = {0.2f,0.2f,0.2f,1.f};
				ImVec4 colLoaded = {0.2f,0.9f,0.2f,1.f};
				ImVec4 colCulled = {0.4f,0.1f,0.1f,1.f};
				ImVec4 colLoadedRebakingTransparency = {0.2f,0.4f,1.0f,1.f};
				ImVec4 colLoadedButNotBaked = {0.5f,0.5f,0.2f,1.f};
				ImVec4 colrequested = {0.2f,0.2f,0.9f,1.f};

				ImGui::Text("Gpu buffer entries count: %d",
					(int)gameData.chunkSystem.gpuBuffer.entriesMap.size());

				if (ImGui::Button("Drop all chunks"))
				{
					gameData.chunkSystem.dropAllChunks(&gameData.chunkSystem.gpuBuffer, true);
				}
				
				ImGui::ColorButton("##1", colNotLoaded, ImGuiColorEditFlags_NoInputs, ImVec2(25, 25));
				ImGui::SameLine();
				ImGui::Text("Not loaded."); 

				ImGui::ColorButton("##3", colLoadedButNotBaked, ImGuiColorEditFlags_NoInputs, ImVec2(25, 25));
				ImGui::SameLine();
				ImGui::Text("Loaded not baked");

				ImGui::ColorButton("##4", colLoadedRebakingTransparency, ImGuiColorEditFlags_NoInputs, ImVec2(25, 25));
				ImGui::SameLine();
				ImGui::Text("Rebaking transparency");

				ImGui::ColorButton("##5", colCulled, ImGuiColorEditFlags_NoInputs, ImVec2(25, 25));
				ImGui::SameLine();
				ImGui::Text("Culled (but loaded)");

				ImGui::ColorButton("##6", colLoaded, ImGuiColorEditFlags_NoInputs, ImVec2(25, 25));
				ImGui::SameLine();
				ImGui::Text("Loaded!");

				ImGui::Separator();

				for (int z = 0; z < gameData.chunkSystem.squareSize; z++)
					for (int x = 0; x < gameData.chunkSystem.squareSize; x++)
					{

						auto c = gameData.chunkSystem.getChunksInMatrixSpaceUnsafe(x, z);

						auto currentColor = colNotLoaded;

						if (c != nullptr)
						{
							if (c->isCulled())
							{
								currentColor = colCulled;
							}
							else if (c->isDirty())
							{
								currentColor = colLoadedButNotBaked;
							}
							else if(c->isDirtyTransparency())
							{
								currentColor = colLoadedRebakingTransparency;
							}
							else
							{
								currentColor = colLoaded;
							}

						}

						if (x > 0)
							ImGui::SameLine();

						ImGui::PushID(z * gameData.chunkSystem.squareSize + x);
						if (ImGui::ColorButton("##chunkb", currentColor,
							ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_NoTooltip
							, ImVec2(blockSize, blockSize)))
						{

						}
						ImGui::PopID();


					}

			}

			//ImGui::Checkbox("Unified geometry pool",
			//	&programData.renderer.unifiedGeometry);

			ImGui::Checkbox("Sort chunks",
				&programData.renderer.sortChunks);

			ImGui::Checkbox("Z pre pass",
				&programData.renderer.zprepass);

			ImGui::Checkbox("Render Transparent",
				&programData.renderer.renderTransparent);

			bool shaders = programData.renderer.defaultShader.shadingSettings.shaders;
			ImGui::Checkbox("Shaders",
				&shaders);
			programData.renderer.defaultShader.shadingSettings.shaders = shaders;

			ImGui::Checkbox("Frustum culling",
				&programData.renderer.frustumCulling);

			ImGui::Checkbox("SSAO",
				&programData.renderer.ssao);
			//ImGui::Checkbox("Water Refraction",
			//	&programData.renderer.waterRefraction);
			
			ImGui::Checkbox("FXAA",
				&getShadingSettings().FXAA);

			if (ImGui::CollapsingHeader("Music ",
				ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_FramePadding))
			{
				
				if (ImGui::Button("Play random night music"))
				{
					AudioEngine::playRandomNightMusic();
				}

				if (ImGui::Button("Play sound"))
				{
					AudioEngine::playSound(AudioEngine::toolBreakingStone, 1);
				}

				if (ImGui::Button("Play sound2"))
				{
					AudioEngine::playSound(AudioEngine::grass, 1);
				}
			}


			
			
		}
		ImGui::End();


		if (ImGui::Begin("Profiler"))
		{
			ImGui::Text("profiler");
			ImGui::Text("fps: %d", programData.currentFps);

			gameData.gameplayFrameProfiler.displayPlot("Gameplay Frame");

			programData.GPUProfiler.displayPlot("GPU");

		}
		ImGui::End();
		ImGui::PopStyleColor();



		gameData.gameplayFrameProfiler.endSubProfile("imgui");
	}
#endif
#pragma endregion

	auto hitStatus = gameData.battleUI.update(*player.inventory.getItemFromIndex(gameData.currentItemSelected, nullptr),
		gameData.currentItemSelected, stopMainInput, programData.ui,
		gameData.rng, deltaTime);

	if (hitStatus.hit)
	{
		//std::cout << "Corectness: " << hitStatus.hitCorectness << "\n";
		//std::cout << "Bonus Crit: " << hitStatus.bonusCritChance << "\n";

		if (targetedEntity && !stopMainInput && hitStatus.hitCorectness > 0)
		{

			auto weaponStats = player.inventory.getItemFromIndex(gameData.currentItemSelected, nullptr)->
				getWeaponStats();

			if (entityHitDistance <= weaponStats.range)
			{
				//check if not creative player
				bool isCreativePlayer = 0;

				if (getEntityTypeFromEID(targetedEntity) == EntityType::player)
				{
					auto f = gameData.entityManager.players.find(targetedEntity);
					if (f != gameData.entityManager.players.end())
					{
						//todo is creative
						//if(f->second.entity)
					}

				}

				if (!isCreativePlayer)
				{
					//std::cout << "Attack! ";

					attackEntity(targetedEntity, gameData.currentItemSelected,
						gameData.c.viewDirection, hitStatus);

					AudioEngine::playHitSound();
				}

			};

		}

	}

#pragma region ui

	ChestBlock *currentChestBlock = 0;
	if (isChest(gameData.interaction.block))
	{

		Chunk *c = 0;
		Block *b = gameData.chunkSystem.getBlockSafeAndChunk(gameData.interaction.blockInteractionPosition.x,
			gameData.interaction.blockInteractionPosition.y, gameData.interaction.blockInteractionPosition.z,
			c
		);

		if (!b || !c)
		{
			gameData.interaction = {};
			gameData.insideInventoryMenu = false;
		}
		else
		{
			if (b->getType() != gameData.interaction.block)
			{
				gameData.interaction = {};
				gameData.insideInventoryMenu = false;
			}
			else
			{
				glm::ivec3 pos = glm::ivec3{modBlockToChunk(gameData.interaction.blockInteractionPosition.x),
					gameData.interaction.blockInteractionPosition.y,
				modBlockToChunk(gameData.interaction.blockInteractionPosition.z)};
				currentChestBlock = c->blockData.getOrCreateChestBlock(pos.x, pos.y, pos.z);
			}
		}
	}


	int cursorSelected = -2;
	unsigned short selectedCreativeItem = 0;
	int craftedItemIndex = -1;

	if (gameData.killed)
	{
		gameData.entityManager.localPlayer.lastLife.life = 0;
		gameData.entityManager.localPlayer.life.life = 0;
	}

	if (!gameData.isInsideMapView)
	{

		if (gameData.interaction.blockInteractionType == InteractionTypes::structureBaseBlock)
		{
			if (programData.ui.renderBaseBlockUI(deltaTime, w, h, programData,
				gameData.interaction.baseBlockHolder, gameData.interaction.blockInteractionPosition,
				gameData.chunkSystem, gameData.undoQueue, gameData.lightSystem, gameData.entityManager))
			{

				auto block = gameData.chunkSystem.getBlockSafe(gameData.interaction.blockInteractionPosition.x, gameData.interaction.blockInteractionPosition.y,
					gameData.interaction.blockInteractionPosition.z);

				auto c = gameData.chunkSystem.
					getChunkSafeFromBlockPos(gameData.interaction.blockInteractionPosition.x,
					gameData.interaction.blockInteractionPosition.z);

				if (c && block)
				{
					auto blockData = c->blockData.getOrCreateBaseBlock(
						modBlockToChunk(gameData.interaction.blockInteractionPosition.x),
						gameData.interaction.blockInteractionPosition.y,
						modBlockToChunk(gameData.interaction.blockInteractionPosition.z));


					std::vector<unsigned char> vectorDataOriginal;
					blockData->formatIntoData(vectorDataOriginal);

					gameData.undoQueue.changedBlockDataEvent(gameData.interaction.blockInteractionPosition,
						*block, vectorDataOriginal);

					*blockData = gameData.interaction.baseBlockHolder;
					std::vector<unsigned char> vectorData;
					blockData->formatIntoData(vectorData);

					Packet_ClientChangeBlockData changeBlockData;
					changeBlockData.eventId = gameData.undoQueue.currentEventId;
					changeBlockData.blockDataHeader.blockType = BlockTypes::structureBase;
					changeBlockData.blockDataHeader.pos = gameData.interaction.blockInteractionPosition;
					changeBlockData.blockDataHeader.dataSize = vectorData.size();


					std::vector<unsigned char> finalPacet;
					finalPacet.resize(vectorData.size() + sizeof(changeBlockData));

					memcpy(finalPacet.data(), &changeBlockData, sizeof(changeBlockData));
					memcpy(finalPacet.data() + sizeof(changeBlockData),
						vectorData.data(), vectorData.size());

					sendPacket(getServer(), headerClientChangeBlockData, player.entityId,
						finalPacet.data(), finalPacet.size(), true, channelChunksAndBlocks);

				}

			}
		}
		else
		{
			programData.ui.renderGameUI(deltaTime, w, h, gameData.currentItemSelected,
				player.inventory, programData.blocksLoader, gameData.insideInventoryMenu,
				cursorSelected,  gameData.currentInventoryTab, 
				player.otherPlayerSettings.gameMode == OtherPlayerSettings::CREATIVE,
				selectedCreativeItem, player.life, programData, player, 
				gameData.craftingSlider, craftedItemIndex, gameData.showUI, gameData.interaction.block, currentChestBlock
			);
		}


	}
	else
	{

		gameData.mapEngine.update(programData, deltaTime, {posInt.x, posInt.z},
			gameData.chunkSystem);

	}

#pragma endregion

#pragma region chat

	if (!stopMainInput || gameData.isInsideChat)
	{
		if (platform::isKeyReleased(platform::Button::Enter) && !gameData.isInsideChat)
		{
			gameData.isInsideChat = true;
			gameData.chatBufferPosition = 0;
			gameData.chatCursor = 0;
			memset(gameData.chatBuffer, 0, sizeof(gameData.chatBuffer));
		}

		if (platform::isKeyReleased(platform::Button::SlashQuestionMark) && !gameData.isInsideChat)
		{
			gameData.isInsideChat = true;
			gameData.chatBufferPosition = 1;
			gameData.chatCursor = 1;
			memset(gameData.chatBuffer, 0, sizeof(gameData.chatBuffer));
			gameData.chatBuffer[0] = '/';
		}

		if (gameData.isInsideChat)
		{

			auto typedInput = platform::getTypedInput();
			for (auto c : typedInput)
			{
				if (c == '\b')
				{
					if (gameData.chatCursor > 0 && gameData.chatBufferPosition>0)
					{
						memmove(gameData.chatBuffer + gameData.chatCursor -1, gameData.chatBuffer + gameData.chatCursor, gameData.chatBufferPosition - gameData.chatCursor +1);
						gameData.chatCursor--;
						gameData.chatBufferPosition--;
					}
				}
				else if (gameData.chatBufferPosition < (int)sizeof(gameData.chatBuffer) - 1)
				{
					if (c != '\n' && c!='\r')
					{
						memmove(gameData.chatBuffer + gameData.chatCursor +1, gameData.chatBuffer + gameData.chatCursor, gameData.chatBufferPosition - gameData.chatCursor +1);
						gameData.chatBuffer[gameData.chatCursor] = c;
						gameData.chatCursor++;
						gameData.chatBufferPosition++;
					}
				}
			}
			gameData.chatBuffer[gameData.chatBufferPosition] = 0;
			{
				bool ctrl = platform::isKeyHeld(platform::Button::LeftCtrl);
				if(ctrl && platform::isKeyPressedOn(platform::Button::C) && gameData.chatBufferPosition>0){
					platform::setClipboardText(std::string(gameData.chatBuffer));
				}
				if(ctrl && platform::isKeyPressedOn(platform::Button::V)){
					auto cb = platform::getClipboardText();
					for(char ch: cb){ if(gameData.chatBufferPosition < (int)sizeof(gameData.chatBuffer)-1 && ch!='\n'){ gameData.chatBuffer[gameData.chatBufferPosition++]=ch; } }
					gameData.chatBuffer[gameData.chatBufferPosition]=0;
				}
				if(platform::isKeyPressedOn(platform::Button::Tab) && gameData.chatBufferPosition==0){
					gameData.chatBuffer[0]='/'; gameData.chatBufferPosition=1; gameData.chatBuffer[1]=0;
				}
				if(platform::isKeyReleased(platform::Button::Left) && gameData.chatCursor>0) gameData.chatCursor--;
				if(platform::isKeyReleased(platform::Button::Right) && gameData.chatCursor < gameData.chatBufferPosition) gameData.chatCursor++;
				if(gameData.chatCursor<0) gameData.chatCursor=0;
				if(gameData.chatCursor>gameData.chatBufferPosition) gameData.chatCursor=gameData.chatBufferPosition;
				if(gameData.commandSuggestions.empty()){
					if(platform::isKeyReleased(platform::Button::Up) && !gameData.chatHistory.empty()){
						if(gameData.chatHistoryIndex < (int)gameData.chatHistory.size()-1) gameData.chatHistoryIndex++;
						auto s = gameData.chatHistory[gameData.chatHistory.size()-1 - gameData.chatHistoryIndex];
						strncpy(gameData.chatBuffer, s.c_str(), sizeof(gameData.chatBuffer)-1);
						gameData.chatBufferPosition = s.size();
						gameData.chatCursor = gameData.chatBufferPosition;
					}
					if(platform::isKeyReleased(platform::Button::Down)){
						if(gameData.chatHistoryIndex>0) gameData.chatHistoryIndex--;
						else if(gameData.chatHistoryIndex==0){ gameData.chatHistoryIndex=-1; gameData.chatBuffer[0]=0; gameData.chatBufferPosition=0; gameData.chatCursor=0; }
						if(gameData.chatHistoryIndex>=0){
							auto s = gameData.chatHistory[gameData.chatHistory.size()-1 - gameData.chatHistoryIndex];
							strncpy(gameData.chatBuffer, s.c_str(), sizeof(gameData.chatBuffer)-1);
							gameData.chatBufferPosition = s.size();
							gameData.chatCursor = gameData.chatBufferPosition;
						}
					}
				}
			}


			auto &renderer = programData.ui.renderer2d;
			glui::Frame f({0, 0, renderer.windowW, renderer.windowH});
			
			auto box = glui::Box().xLeft().yBottom(-20).xDimensionPercentage(1.f).yDimensionPixels(65)();

			int startPos = gameData.chatBufferPosition;
			{
				float advance = 10; //adding the padding
				for (int i = gameData.chatBufferPosition - 1; i >= 0; i--)
				{
					advance = renderer.getTextSize(gameData.chatBuffer + i, 
						programData.ui.font, 1).x + 10;

					if (advance <= box.z)
					{
						startPos = i;
					}
					else
					{
						break;
					}
				}
			}

			{

				renderer.renderRectangle(box, {0.1,0.1,0.1,0.6});

				if (startPos < gameData.chatBufferPosition)
				{
					renderer.renderText({10, box.y + box.w - 10}, gameData.chatBuffer + startPos,
						programData.ui.font, Colors_White, 64, 4, 0, false);
				}

			}

			//command tab-completion suggestions
			bool isTypingCommand = gameData.chatBuffer[0] == '/';
			if (isTypingCommand)
			{
				std::string prefix = gameData.chatBuffer + 1;

				if (prefix != gameData.lastRequestedCommandPrefix)
				{
					gameData.lastRequestedCommandPrefix = prefix;
					gameData.chatSuggestionSelected = 0;

					std::string sendPrefix = prefix;
					sendPrefix += '\0';

					sendPacket(getServer(), headerCommandSuggestions, player.entityId,
						(void *)sendPrefix.data(), sendPrefix.size(),
						false, channelHandleConnections);
				}

				//navigate and autocomplete
				if (!gameData.commandSuggestions.empty())
				{

					if (platform::isKeyReleased(platform::Button::Up))
					{
						gameData.chatSuggestionSelected--;
						if (gameData.chatSuggestionSelected < 0)
						{
							gameData.chatSuggestionSelected = (int)gameData.commandSuggestions.size() - 1;
						}
					}

					if (platform::isKeyReleased(platform::Button::Down))
					{
						gameData.chatSuggestionSelected++;
						if (gameData.chatSuggestionSelected >= (int)gameData.commandSuggestions.size())
						{
							gameData.chatSuggestionSelected = 0;
						}
					}

					if (platform::isKeyTyped(platform::Button::Tab))
					{
						int sel = gameData.chatSuggestionSelected;
						if (sel < 0 || sel >= (int)gameData.commandSuggestions.size()) { sel = 0; }

						//replace the last word in the command with the suggestion
						std::string typed = gameData.chatBuffer + 1;
						size_t space = typed.find_last_of(' ');
						std::string before = (space == std::string::npos) ? "" : typed.substr(0, space + 1);
						std::string completed = before + gameData.commandSuggestions[sel];

						//add a trailing space so the next argument can be typed right away
						if (completed.size() + 1 < sizeof(gameData.chatBuffer))
						{
							completed += " ";
						}

						memset(gameData.chatBuffer, 0, sizeof(gameData.chatBuffer));
						memcpy(gameData.chatBuffer, completed.c_str(),
							std::min<size_t>(completed.size(), sizeof(gameData.chatBuffer) - 1));
						gameData.chatBufferPosition = (int)std::min<size_t>(completed.size(), sizeof(gameData.chatBuffer) - 1);
					}
				}

				//draw the suggestion dropdown
				if (!gameData.commandSuggestions.empty())
				{
					const float rowH = 40;
					int rows = std::min<int>((int)gameData.commandSuggestions.size(), 8);

					//show a window around the selected suggestion
					int selected = std::clamp(gameData.chatSuggestionSelected, 0, (int)gameData.commandSuggestions.size() - 1);
					int startRow = std::clamp(selected - rows / 2, 0,
						std::max(0, (int)gameData.commandSuggestions.size() - rows));

					float panelH = rows * rowH + 10;
					glm::ivec4 panel = { 0, box.y - (int)panelH, box.z, (int)panelH };
					float textY = panel.y + 8;

					renderer.renderRectangle(panel, { 0.2,0.2,0.2,0.85 });

					for (int i = 0; i < rows; i++)
					{
						int index = startRow + i;
						if (index >= (int)gameData.commandSuggestions.size()) { break; }

						glm::ivec4 rowRect = { panel.x, (int)textY, panel.z, (int)rowH };
						if (index == selected)
						{
							renderer.renderRectangle(rowRect, { 0.35,0.35,0.35,0.9 });
						}

						renderer.renderText({ 12, textY + rowH - 10 }, gameData.commandSuggestions[index].c_str(),
							programData.ui.font, index == selected ? glm::vec4(0.9,0.9,1,1) : glm::vec4(1,1,1,1),
							40, 4, 0, false);

						textY += rowH;
					}
				}
			}
			else
			{
				gameData.lastRequestedCommandPrefix = "";
				if (!gameData.commandSuggestions.empty())
				{
					gameData.commandSuggestions.clear();
				}
			}

			if (platform::isKeyReleased(platform::Button::Enter) && 
				gameData.chatBufferPosition)
			{
				gameData.chatHistory.push_back(std::string(gameData.chatBuffer));
				if(gameData.chatHistory.size()>30) gameData.chatHistory.erase(gameData.chatHistory.begin());
				gameData.chatHistoryIndex=-1;
				sendPacket(getServer(), headerSendChat, player.entityId,
					gameData.chatBuffer, gameData.chatBufferPosition+1, true, channelHandleConnections);
				gameData.chatBufferPosition = 0;
				gameData.chatCursor=0;
				memset(gameData.chatBuffer, 0, sizeof(gameData.chatBuffer));

				gameData.isInsideChat = false;
			}


		}


		//other chat messages
		if(gameData.chatStayOnTimer || gameData.isInsideChat)
		{

			while (gameData.chat.size() > 10)
			{
				gameData.chat.pop_back();
			}

			auto &renderer = programData.ui.renderer2d;
			glui::Frame f({0, 0, renderer.windowW, renderer.windowH});

			auto box = glui::Box().xLeft().yTop().xDimensionPercentage(0.75f).yDimensionPixels(renderer.windowH - 100)();
			

			//determine how many lines we can fit
			float textSize = 65;
			float textPos = box.y + box.w;
			float textPosCopy = textPos;
			float boxDimension = 0;
			for (auto &c : gameData.chat)
			{
				if (textPos < 0)
				{
					break;
				}
				
				//renderer.renderText({0, textPos}, c.c_str(), programData.ui.font,
				//	Colors_White, 1, 64, 3, false);

				textPos -= textSize;
				boxDimension += textSize;
			}

			if (boxDimension)
			{
				box.y = std::max(textPos, 0.f);
				box.w = std::min((float)box.y, boxDimension) + 10; //we add just a little padding
				renderer.renderRectangle(box, {0.1,0.1,0.1,0.6});

				for (auto &c : gameData.chat)
				{
					if (textPosCopy < 0)
					{
						break;
					}

					renderer.renderText({0, textPosCopy}, c.c_str(), programData.ui.font,
						Colors_White, 64, 3, 0, false);

					textPosCopy -= textSize;
				}

			};


			if (gameData.isInsideChat)
			{
				gameData.chatStayOnTimer = 0;
			}
			else
			{
				gameData.chatStayOnTimer -= deltaTime;
				if (gameData.chatStayOnTimer < 0) { gameData.chatStayOnTimer = 0; }
			}
		}

	}


#pragma endregion


#pragma region crafting

	if (gameData.insideInventoryMenu)
	{
		
		if (craftedItemIndex >= 0)
		{

			if (platform::isLMousePressed())
			{
				
				if (recepieExists(craftedItemIndex))
				{

					auto recepie = getRecepieFromIndexUnsafe(craftedItemIndex);


					if (canItemBeCrafted(recepie, player.inventory))
					{

						if (canItemBeMovedToAndMoveIt(recepie.result,
							*player.inventory.getItemFromIndex(PlayerInventory::CURSOR_INDEX, nullptr)))
						{
							Packet_ClientCraftedItem packet;
							packet.recepieIndex = craftedItemIndex;
							packet.to = PlayerInventory::CURSOR_INDEX;
							packet.revisionNumber = player.inventory.revisionNumber;

							sendPacket(getServer(), headerClientCraftedItem, player.entityId,
								&packet, sizeof(packet), true, channelChunksAndBlocks);


							craftItemUnsafe(recepie, player.inventory);

						}

						

					};

				}

			}

		}


	}


#pragma endregion

	//std::cout << cursorSelected << "\n";


#pragma region move items in inventory
	if (gameData.insideInventoryMenu && !gameData.escapePressed)
	{




		//pickup blocks or items from the creative inventory
		if (selectedCreativeItem)
		{

			if (!player.inventory.heldInMouse.type)
			{
				if (platform::isLMousePressed())
				{
					Item item = itemCreator(selectedCreativeItem);
					
					if (platform::isKeyHeld(platform::Button::LeftCtrl))
					{
						item.counter = item.getStackSize();
					}

					//todo force overwrite item with metadata here!
					forceOverWriteItem(player.inventory, currentChestBlock, PlayerInventory::CURSOR_INDEX,
						item);

					player.inventory.heldInMouse = item;
				}
			}

		}
		else
		{
			static std::bitset<128> rightClickedThisClick = 0;

			if (platform::isLMousePressed())
			{
				//crafting
				//

				//grab items
				if (cursorSelected >= 0)
				{
					Item *selected = player.inventory.getItemFromIndex(cursorSelected, currentChestBlock);
					Item *cursor = &player.inventory.heldInMouse;

					if (selected && selected != cursor)
					{

						if (cursor->type == 0)
						{
							//grab
							grabItem(player.inventory, currentChestBlock, cursorSelected, PlayerInventory::CURSOR_INDEX);
						}
						else
						{
							//place
							if (!placeItem(player.inventory, currentChestBlock, PlayerInventory::CURSOR_INDEX, cursorSelected))
							{
								//swap
								swapItems(player.inventory, currentChestBlock, cursorSelected, PlayerInventory::CURSOR_INDEX);
							}

						}

					}
				}
			}
			else if (platform::isRMousePressed())
			{

				rightClickedThisClick.reset();


				if (cursorSelected >= 0)
				{
					Item *selected = player.inventory.getItemFromIndex(cursorSelected, currentChestBlock);
					Item *cursor = &player.inventory.heldInMouse;

					if (cursor->type != 0)
					{
						//place one
						if (placeItem(player.inventory, currentChestBlock, PlayerInventory::CURSOR_INDEX, cursorSelected, 1))
						{
							rightClickedThisClick[cursorSelected] = true;
						}

					}
					else
					{

						//grab
						grabItem(player.inventory, currentChestBlock, cursorSelected,
							PlayerInventory::CURSOR_INDEX, selected->counter / 2);

						//don't place it again lol
						rightClickedThisClick[cursorSelected] = true;
					}


				}



			}
			else if (platform::isRMouseHeld())
			{

				//right click held place items
				if (cursorSelected >= 0 && !rightClickedThisClick[cursorSelected])
				{
					Item *selected = player.inventory.getItemFromIndex(cursorSelected, currentChestBlock);
					Item *cursor = &player.inventory.heldInMouse;

					if (cursor->type != 0)
					{
						//place one
						if (placeItem(player.inventory, currentChestBlock, PlayerInventory::CURSOR_INDEX, cursorSelected, 1))
						{
							rightClickedThisClick[cursorSelected] = true;
						}

					}
				}


			}

			//outside borders
			if (cursorSelected == -1)
			{

				if (platform::isLMousePressed())
				{
					gameData.entityManager.dropItemByClient(
						gameData.entityManager.localPlayer.entity.position,
						PlayerInventory::CURSOR_INDEX, gameData.undoQueue, gameData.c.viewDirection * 5.f,
						gameData.serverTimer, player.inventory, 0);
				}
				else if (platform::isRMousePressed())
				{
					gameData.entityManager.dropItemByClient(
						gameData.entityManager.localPlayer.entity.position,
						PlayerInventory::CURSOR_INDEX, gameData.undoQueue, gameData.c.viewDirection * 5.f,
						gameData.serverTimer, player.inventory, 1);
				}

			}
		}

		
		

	}

	//close inventory, drop the item held in mouse
	if (!gameData.insideInventoryMenu)
	{
		auto &itemHeld = player.inventory.heldInMouse;

		if (itemHeld.type)
		{


			for (int i = PlayerInventory::INVENTORY_CAPACITY-1; i >=0 ; i--)
			{
				auto &slot = *player.inventory.getItemFromIndex(i, 0);

				if (slot.type == 0)
				{
					swapItems(player.inventory, currentChestBlock, i, PlayerInventory::CURSOR_INDEX);
					break;
				}else
				if (areItemsTheSame(itemHeld, slot))
				{
					placeItem(player.inventory, currentChestBlock, PlayerInventory::CURSOR_INDEX, i);
				}

				if (itemHeld.counter == 0) { break; }

			}

			if (itemHeld.counter != 0)
			{
				gameData.entityManager.dropItemByClient(
					gameData.entityManager.localPlayer.entity.position,
					PlayerInventory::CURSOR_INDEX, gameData.undoQueue, gameData.c.viewDirection * 5.f,
					gameData.serverTimer, player.inventory, 0);
			}

			

		}
	}

	player.inventory.sanitize();

#pragma endregion


#pragma region esc manu

	bool justPressedEsc = 0;
	if (gameData.insideInventoryMenu)
	{
		if (platform::isKeyReleased(platform::Button::Escape))
		{
			if (programData.ui.isItemSearchFocused()) programData.ui.setItemSearchFocused(false);
			else exitInventoryMenu();
		}
	}
	else if (gameData.isInsideMapView)
	{
		if (platform::isKeyReleased(platform::Button::Escape))
		{
			gameData.isInsideMapView = false;
			gameData.mapEngine.close();
		}
	}
	else if (gameData.isInsideChat)
	{
		if (platform::isKeyReleased(platform::Button::Escape))
		{
			gameData.isInsideChat = false;
			gameData.chatBufferPosition = 0;
			memset(gameData.chatBuffer, 0, sizeof(gameData.chatBuffer));
		}
	}
	else if (gameData.interaction.blockInteractionType)
	{
		if (platform::isKeyReleased(platform::Button::Escape))
		{
			exitInventoryMenu();
		}
	}
	else
	{
		if (platform::isKeyReleased(platform::Button::Escape) && !gameData.escapePressed)
		{
			gameData.escapePressed = !gameData.escapePressed;
			justPressedEsc = true;
		}
	}

	if (gameData.killed)
	{
		programData.ui.renderer2d.renderRectangle({0,0, w,h}, {0.1,0,0,0.6});
	}

	if (gameData.escapePressed)
	{


		programData.ui.menuRenderer.Begin(2);
		programData.ui.menuRenderer.SetAlignModeFixedSizeWidgets({0,150});


		//if we are not in the tonemapping section...
		bool dontDarkenScreen = 0;
		for (auto &s : programData.ui.menuRenderer.internal.allMenuStacks
			[programData.ui.menuRenderer.internal.currentId])
		{
			if (s == "Color post processing")
			{
				
				dontDarkenScreen = true;
				break;
			}
		}

		if (!dontDarkenScreen)
		{
			programData.ui.renderer2d.renderRectangle({0,0,programData.ui.renderer2d.windowW,
				programData.ui.renderer2d.windowH}, {0,0,0,0.5});
		};


		programData.ui.renderer2d.renderText({150,50},
			("fps: " + std::to_string(programData.currentFps)).c_str(), programData.ui.font, Colors_Gray, 64.f * 0.75f);



		programData.ui.menuRenderer.Text(loc_GameMenu(), Colors_White);

		if (programData.ui.menuRenderer.Button(loc_BackToGame(), Colors_Gray, programData.ui.buttonTexture))
		{
			gameData.escapePressed = false;
		}

		displaySettingsMenuButton(programData);

		displaySkinSelectorMenuButton(programData);

		if (programData.ui.menuRenderer.Button(loc_Exit(), Colors_Gray, programData.ui.buttonTexture))
		{
			gameData.exitGameModal.open(loc_Exit(), loc_AreYouSureExit());
		}

		if (programData.ui.menuRenderer.Button(loc_BackToMenu(), Colors_Gray, programData.ui.buttonTexture))
		{
			gameData.exitWorldModal.open(loc_BackToMenu(), loc_AreYouSureLeave());
		}

		programData.ui.menuRenderer.End();

		if (programData.ui.menuRenderer.internal.allMenuStacks[2].size() == 0 && 
			platform::isKeyReleased(platform::Button::Escape) && !justPressedEsc)
		{
			gameData.escapePressed = false;
		}

		// Render exit confirmation modal
		if (gameData.exitGameModal.show)
		{
			glm::vec2 screenSize = {programData.ui.renderer2d.windowW, programData.ui.renderer2d.windowH};
			if (gameData.exitGameModal.render(
				programData.ui.renderer2d, programData.ui.font, screenSize))
			{
				if (gameData.exitGameModal.result)
				{
					terminate = true;
				}
			}
		}

		// Render exit world confirmation modal
		if (gameData.exitWorldModal.show)
		{
			glm::vec2 screenSize = {programData.ui.renderer2d.windowW, programData.ui.renderer2d.windowH};
			if (gameData.exitWorldModal.render(
				programData.ui.renderer2d, programData.ui.font, screenSize))
			{
				if (gameData.exitWorldModal.result)
				{
					terminate = true;
				}
			}
		}

	}
	else if(gameData.killed)
	{
		programData.ui.menuRenderer.Begin(3);
		programData.ui.menuRenderer.SetAlignModeFixedSizeWidgets({0,150});

		programData.ui.menuRenderer.Text(loc_DeathMessage(), Colors_White);

		if (programData.ui.menuRenderer.Button(loc_Respawn(), Colors_Gray, programData.ui.buttonTexture))
		{
			sendPacket(getServer(), headerClientWantsToRespawn, player.entityId,
				0, 0, true, channelChunksAndBlocks);
		}

		if (programData.ui.menuRenderer.Button(loc_Exit(), Colors_Gray, programData.ui.buttonTexture))
		{
			terminate = true;
		}

		programData.ui.menuRenderer.End();
	}

	platform::showMouse(gameData.escapePressed || gameData.insideInventoryMenu ||
		gameData.killed || gameData.isInsideMapView || gameData.interaction.blockInteractionType);


#pragma endregion


	//reset stuff
	gameData.justDamaged = false;


	//updateviewdistance changeviewdistance updaterenderdistance
	gameData.chunkSystem.changeRenderDistance(getShadingSettings().viewDistance * 2, true);


	gameData.gameplayFrameProfiler.endFrame();
	programData.GPUProfiler.endFrame();
	gameData.gameplayFrameProfiler.startFrame();
	programData.GPUProfiler.startFrame();
	gameData.gameplayFrameProfiler.startSubProfile("swap chain and others");



	if (terminate)
	{
		return false;
	}

	return true;
}

void closeGameLogic()
{
	static bool cleaning = false;
	if (cleaning) return;
	cleaning = true;
	threadPoolForChunkBaking.cleanup();
	gameData.chunkSystem.cleanup(false);
	gameData.currentSkinTexture.cleanup();
	gameData.entityManager.cleanup();
	gameData.mapEngine.close();
	gameData.clearData();
	AudioEngine::stopAllMusicAndSounds();
	cleaning = false;
}