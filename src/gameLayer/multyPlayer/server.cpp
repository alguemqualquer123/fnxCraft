#include "multyPlayer/server.h"
#include <glm/vec3.hpp>
#include "chunkSystem.h"
#include "threadStuff.h"
#include <thread>
#include <mutex>
#include <queue>
#include "worldGenerator.h"
#include <unordered_map>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <atomic>
#include <enet/enet.h>
#include "multyPlayer/packet.h"
#include "multyPlayer/enetServerFunction.h"
#include <platformTools.h>
#include <fstream>
#include <sstream>
#include <structure.h>
#include <biome.h>
#include <unordered_set>
#include <profilerLib.h>
#include "multyPlayer/chunkSaver.h"
#include "multyPlayer/serverChunkStorer.h"
#include <multyPlayer/tick.h>
#include <multyPlayer/splitUpdatesLogic.h>
#include <gameplay/crafting.h>
#include <gameplay/cat.h>
#include <gameplay/gameplayRules.h>
#include <gameplay/food.h>
#include <profiler.h>
#include <magic_enum.hpp>

static std::atomic<bool> serverRunning = false;

bool serverStartupStuff(const std::string &path);

bool isServerRunning()
{
	return serverRunning;
}

bool startServer(const std::string &path)
{

	bool expected = 0;
	if (serverRunning.compare_exchange_strong(expected, 1))
	{
		if (!serverStartupStuff(path))
		{
			serverRunning = false;
			return 0;
		}

		return 1;
	}
	else
	{
		return 0;
	}
}


void updateLoadedChunks(
	WorldGenerator &wg,
	StructuresManager &structureManager,
	BiomesManager &biomesManager,
	std::vector<SendBlocksBack> &sendNewBlocksToPlayers,
	WorldSaver &worldSaver, bool generateNewChunks, std::minstd_rand &rng);


struct ServerData
{

	//todo probably move this just locally
	ServerChunkStorer chunkCache = {};
	ENetHost *server = nullptr;
	ServerSettings settings = {};

	float tickTimer = 0;
	float tickDeltaTime = 0;
	int tickDeltaTimeMs = 0;
	int ticksPerSeccond = 0;
	int runsPerSeccond = 0;
	float seccondsTimer = 0;

	float saveEntitiesTimer = 5;
	std::uint64_t lastTimer = 0;

	//this is used as an unique id for chunk packets
	unsigned int chunkPacketId = 0;

}sd;

bool g_flatMobsRequested = false;
glm::ivec3 g_flatMobsCenter = {};
bool g_mobsFrozen = false;

int outTicksPerSeccond = 0;

int getServerTicksPerSeccond()
{
	return outTicksPerSeccond;
}

ServerChunkStorer &getServerChunkStorer()
{
	return sd.chunkCache;
}

void clearSD(WorldSaver &worldSaver)
{
	//todo saveEntityId stuff
	//worldSaver.saveEntityId(getCurrentEntityId());
	sd.chunkCache.saveAllChunks(worldSaver);
	sd.chunkCache.cleanup();
	closeThreadPool();
}

int getChunkCapacity()
{
	return sd.chunkCache.savedChunks.size();
}

void closeServer()
{
	//todo cleanup stuff
	if (serverRunning)
	{

		closeEnetListener();


		//close loop
		serverRunning = false;

		//then signal the barier from the task waiting to unlock the mutex

		//then wait for the server to close
		//serverThread.join();

		enet_host_destroy(sd.server);

		//todo clear othher stuff
		sd = {};
	}

	//serverSettingsMutex.unlock();
}


//Note: it is a problem that the block validation and the item validation are on sepparate threads.
bool computeRevisionStuff(Client &client, bool allowed, 
	const EventId &eventId, std::uint64_t *oldid, std::uint64_t *newid)
{

	permaAssertComment((oldid == 0 && newid == 0) || (oldid != 0 && newid != 0),
		"both ids should be supplied or none");


	bool noNeedToNotifyUndo = false;

	if (client.revisionNumber > eventId.revision)
	{
		//if the revision number is increased it means that we already undoed all those moves
		allowed = false;
		noNeedToNotifyUndo = true;
		//std::cout << "Server revision number ignore: " << client->revisionNumber << " "
		//	<< i.t.eventId.revision << "\n";
	}


	//validate event
	if(allowed)
	{
		if (oldid && newid)
		{
			Packet packet;
			packet.header = headerValidateEventAndChangeID;

			Packet_ValidateEventAndChangeId packetData;
			packetData.eventId = eventId;
			packetData.oldId = *oldid;
			packetData.newId = *newid;

			sendPacket(client.peer, packet,
				(char *)&packetData, sizeof(Packet_ValidateEventAndChangeId),
				true, channelChunksAndBlocks);
		}
		else
		{
			Packet packet;
			packet.header = headerValidateEvent;

			Packet_ValidateEvent packetData;
			packetData.eventId = eventId;

			sendPacket(client.peer, packet,
				(char *)&packetData, sizeof(Packet_ValidateEvent),
				true, channelChunksAndBlocks);
		}
		
	}
	else if (!noNeedToNotifyUndo)
	{
		Packet packet;
		//packet.cid = i.cid;
		packet.header = headerInValidateEvent;

		Packet_InValidateEvent packetData;
		packetData.eventId = eventId;

		client.revisionNumber++;

		sendPacket(client.peer, packet, (char *)&packetData,
			sizeof(Packet_ValidateEvent), true, channelChunksAndBlocks);
	}

	return allowed;
}

bool serverStartupStuff(const std::string &path)
{
	sd = ServerData{};
	sd.settings = loadWorldConfig(path);
	sd.settings.worldName = path;


	//start enet server
	ENetAddress adress;
	adress.host = ENET_HOST_ANY;
	adress.port = 7771;
	ENetEvent event;

	//first param adress, players limit, channels, bandwith limit
	sd.server = enet_host_create(&adress, 32, SERVER_CHANNELS, 0, 0);


	if (!sd.server)
	{
		//todo some king of error reporting to the player
		return 0;
	}

	if (!startEnetListener(sd.server, path))
	{
		enet_host_destroy(sd.server);
		sd.server = 0;
		return 0;
	}

	sd.lastTimer = getTimer();

	return true;
}


void updateOtherPlayerSettings(Client &client)
{
	Packet_UpdateOwnOtherPlayerSettings packet;
	packet.otherPlayerSettings = client.playerData.otherPlayerSettings;

	sendPacket(client.peer, headerUpdateOwnOtherPlayerSettings,
		&packet, sizeof(packet), true, channelChunksAndBlocks);
}

void changePlayerGameMode(std::uint64_t cid, unsigned char gameMode)
{

	auto client = getClientNotLocked(cid);

	if (client)
	{
		if (client->playerData.otherPlayerSettings.gameMode != gameMode)
		{
			client->playerData.otherPlayerSettings.gameMode = gameMode;

			updateOtherPlayerSettings(*client);
		}

	}
}


ServerSettings getServerSettingsCopy()
{
	return sd.settings;
}

ServerSettings &getServerSettingsReff()
{
	return sd.settings;
}

unsigned int getRandomTickSpeed()
{
	return sd.settings.randomTickSpeed;
}

void setServerSettings(ServerSettings settings)
{
	auto perClient = sd.settings.perClientSettings;
	for (auto &s : settings.perClientSettings)
	{
		auto it = perClient.find(s.first);
		if (it != perClient.end())
		{
			s.second = it->second;
		}
	}
	sd.settings = settings;
	sd.settings.perClientSettings = perClient;
	for (auto &s : sd.settings.perClientSettings)
	{
		auto it = settings.perClientSettings.find(s.first);
		if (it != settings.perClientSettings.end())
		{
			s.second = it->second;
		}
	}
}

std::string worldConfigPath(const std::string &worldName)
{
	return std::string(RESOURCES_PATH) + "worlds/" + worldName + "/worldConfig.json";
}

ServerSettings loadWorldConfig(const std::string &worldName)
{
	ServerSettings s;
	s.worldName = worldName;
	std::string path = worldConfigPath(worldName);
	std::ifstream f(path);
	if (!f.is_open()) return s;
	std::string content((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
	auto getBool = [&](const std::string &key, bool def){
		auto pos = content.find("\"" + key + "\"");
		if (pos == std::string::npos) return def;
		auto colon = content.find(":", pos);
		if (colon == std::string::npos) return def;
		auto v = content.substr(colon+1, 10);
		if (v.find("true") != std::string::npos) return true;
		if (v.find("false") != std::string::npos) return false;
		return def;
	};
	auto getStr = [&](const std::string &key, std::string def){
		auto pos = content.find("\"" + key + "\"");
		if (pos == std::string::npos) return def;
		auto colon = content.find(":", pos);
		if (colon == std::string::npos) return def;
		auto q1 = content.find("\"", colon+1);
		if (q1 == std::string::npos) return def;
		auto q2 = content.find("\"", q1+1);
		if (q2 == std::string::npos) return def;
		return content.substr(q1+1, q2-q1-1);
	};
	s.allowCheats = getBool("allowCheats", false);
	s.pvpEnabled = getBool("pvpEnabled", false);
	s.keepInventory = getBool("keepInventory", true);
	s.hungerEnabled = getBool("hungerEnabled", true);
	s.thirstEnabled = getBool("thirstEnabled", true);
	s.difficulty = getStr("difficulty", "normal");
	s.defaultGamemode = getStr("defaultGamemode", "survival");
	s.worldOwner = getStr("worldOwner", "");
	auto getInt = [&](const std::string &key, int def){
		auto pos = content.find("\"" + key + "\"");
		if (pos == std::string::npos) return def;
		auto colon = content.find(":", pos);
		if (colon == std::string::npos) return def;
		try{ return std::stoi(content.substr(colon+1)); }catch(...){return def;}
	};
	s.randomTickSpeed = getInt("randomTickSpeed", 3);
	s.simulationDistanceRadius = getInt("simulationDistanceRadius", 8);
	return s;
}

void saveWorldConfig(const std::string &worldName, const ServerSettings &s)
{
	std::string path = worldConfigPath(worldName);
	std::filesystem::create_directories(std::filesystem::path(path).parent_path());
	std::ofstream f(path);
	if (!f.is_open()) return;
	f << "{\n";
	f << "  \"worldName\": \"" << s.worldName << "\",\n";
	f << "  \"allowCheats\": " << (s.allowCheats?"true":"false") << ",\n";
	f << "  \"pvpEnabled\": " << (s.pvpEnabled?"true":"false") << ",\n";
	f << "  \"keepInventory\": " << (s.keepInventory?"true":"false") << ",\n";
	f << "  \"hungerEnabled\": " << (s.hungerEnabled?"true":"false") << ",\n";
	f << "  \"thirstEnabled\": " << (s.thirstEnabled?"true":"false") << ",\n";
	f << "  \"difficulty\": \"" << s.difficulty << "\",\n";
	f << "  \"defaultGamemode\": \"" << s.defaultGamemode << "\",\n";
	f << "  \"worldOwner\": \"" << s.worldOwner << "\",\n";
	f << "  \"randomTickSpeed\": " << s.randomTickSpeed << ",\n";
	f << "  \"simulationDistanceRadius\": " << s.simulationDistanceRadius << "\n";
	f << "}\n";
}

void genericBroadcastEntityDeleteFromServerToPlayer(std::uint64_t eid, bool reliable, 
	std::unordered_map<std::uint64_t, Client *> &allClients, 
	glm::ivec2 lastChunkClientsGotUpdates)
{
	Packet packet;
	packet.header = headerRemoveEntity;

	Packet_RemoveEntity data;
	data.EID = eid;

	broadCast(packet, &data, sizeof(data),
		nullptr, reliable, channelEntityPositions);
}


void genericBroadcastEntityKillFromServerToPlayer(std::uint64_t eid, bool reliable, ENetPeer *peerToIgnore)
{
	Packet packet;
	packet.header = headerKillEntity;

	Packet_KillEntity data;
	data.EID = eid;

	broadCast(packet, &data, sizeof(data),
		peerToIgnore, reliable, channelEntityPositions);
}

void serverWorkerUpdate(
	WorldGenerator &wg,
	StructuresManager &structuresManager,
	BiomesManager &biomesManager,
	WorldSaver &worldSaver,
	std::vector<ServerTask> &serverTask,
	float deltaTime, Profiler &serverProfiler
	)
{

#pragma region timers stuff
	if (deltaTime > 0.05f) deltaTime = 0.05f;
	if (deltaTime < 0) deltaTime = 0;
	auto currentTimer = getTimer();
	sd.tickTimer += deltaTime;
	sd.seccondsTimer += deltaTime;
	sd.tickDeltaTime += deltaTime;
	sd.saveEntitiesTimer -= deltaTime;
	auto deltaTimeMS = currentTimer - sd.lastTimer;
	if (deltaTimeMS > 50) deltaTimeMS = 50;
	if (deltaTimeMS < 0) deltaTimeMS = 0;
	sd.tickDeltaTimeMs += deltaTimeMS;
#pragma endregion

	auto &settings = sd.settings;

	static std::minstd_rand rng(std::random_device{}());

	if(g_flatMobsRequested){
		glm::ivec3 c = g_flatMobsCenter;
		int platY = 60;
		int half = 32;
		for(int dx=-half; dx<=half; dx++) for(int dz=-half; dz<=half; dz++){
			int wx = c.x + dx;
			int wz = c.z + dz;
			for(int y=0; y<=platY+1; y++){
				SavedChunk *ch = nullptr;
				auto b = sd.chunkCache.getBlockSafeAndChunk(glm::ivec3(wx,y,wz), ch);
				if(!b || !ch) continue;
				Block nb;
				if(y < platY-4) nb.setType(BlockTypes::stone);
				else if(y < platY) nb.setType(BlockTypes::dirt);
				else if(y == platY) nb.setType(BlockTypes::grassBlock);
				else nb.setType(BlockTypes::air);
				nb.setLightLevel(0);
				*b = nb;
				ch->otherData.dirty = true;
				ch->otherData.dirtyBlockData = true;
			}
		}
		std::cout << "[FlatMobs] Plataforma gerada em " << c.x << "," << platY << "," << c.z << " half=" << half << std::endl;
		{
			int idx=0;
			auto getPos = [&](int i)->glm::dvec3{
				int cols = 8;
				int r = i / cols;
				int col = i % cols;
				double x = c.x - 28 + col*7 + 0.5;
				double z = c.z - 28 + r*7 + 0.5;
				return glm::dvec3(x, platY+1, z);
			};
			auto doSpawn = [&](glm::dvec3 pos, int type, auto creator){
				auto chunkPos = glm::ivec2(divideChunk((int)pos.x), divideChunk((int)pos.z));
				auto ch = sd.chunkCache.getChunkOrGetNull(chunkPos.x, chunkPos.y);
				if(!ch) { std::cout << "[FlatMobs] chunk null at " << chunkPos.x << "," << chunkPos.y << std::endl; return; }
				creator(ch, pos, chunkPos);
				std::cout << "[FlatMobs] spawn type " << type << " at " << pos.x << "," << pos.y << "," << pos.z << std::endl;
			};
			{ glm::dvec3 pos=getPos(idx++); Pig pig{}; pig.position=pos; pig.lastPosition=pos; auto cp=glm::ivec2(divideChunk((int)pos.x), divideChunk((int)pos.z)); auto ch=sd.chunkCache.getChunkOrGetNull(cp.x, cp.y); if(ch){ uint64_t nid=getEntityIdAndIncrement(worldSaver, EntityType::pigs); PigServer s; s.entity=pig; s.configureSpawnSettings(rng); ch->entityData.pigs.insert({nid,s}); sd.chunkCache.entityChunkPositions[nid]=cp; std::cout << "[FlatMobs] pig " << nid << std::endl; } }
			{ glm::dvec3 pos=getPos(idx++); Cow cow{}; cow.position=pos; cow.lastPosition=pos; auto cp=glm::ivec2(divideChunk((int)pos.x), divideChunk((int)pos.z)); auto ch=sd.chunkCache.getChunkOrGetNull(cp.x, cp.y); if(ch){ uint64_t nid=getEntityIdAndIncrement(worldSaver, EntityType::cows); CowServer s; s.entity=cow; s.configureSpawnSettings(rng); ch->entityData.cows.insert({nid,s}); sd.chunkCache.entityChunkPositions[nid]=cp; } }
			{ glm::dvec3 pos=getPos(idx++); Sheep sheep{}; sheep.position=pos; sheep.lastPosition=pos; auto cp=glm::ivec2(divideChunk((int)pos.x), divideChunk((int)pos.z)); auto ch=sd.chunkCache.getChunkOrGetNull(cp.x, cp.y); if(ch){ uint64_t nid=getEntityIdAndIncrement(worldSaver, EntityType::sheeps); SheepServer s; s.entity=sheep; s.configureSpawnSettings(rng); ch->entityData.sheeps.insert({nid,s}); sd.chunkCache.entityChunkPositions[nid]=cp; } }
			{ glm::dvec3 pos=getPos(idx++); Zombie zombie{}; zombie.position=pos; zombie.lastPosition=pos; auto cp=glm::ivec2(divideChunk((int)pos.x), divideChunk((int)pos.z)); auto ch=sd.chunkCache.getChunkOrGetNull(cp.x, cp.y); if(ch){ uint64_t nid=getEntityIdAndIncrement(worldSaver, EntityType::zombies); ZombieServer s; s.entity=zombie; ch->entityData.zombies.insert({nid,s}); sd.chunkCache.entityChunkPositions[nid]=cp; } }
			{ glm::dvec3 pos=getPos(idx++); Cat cat{}; cat.position=pos; cat.lastPosition=pos; auto cp=glm::ivec2(divideChunk((int)pos.x), divideChunk((int)pos.z)); auto ch=sd.chunkCache.getChunkOrGetNull(cp.x, cp.y); if(ch){ uint64_t nid=getEntityIdAndIncrement(worldSaver, EntityType::cats); CatServer s; s.entity=cat; s.configureSpawnSettings(rng); ch->entityData.cats.insert({nid,s}); sd.chunkCache.entityChunkPositions[nid]=cp; } }
			{ glm::dvec3 pos=getPos(idx++); Goblin goblin{}; goblin.position=pos; goblin.lastPosition=pos; auto cp=glm::ivec2(divideChunk((int)pos.x), divideChunk((int)pos.z)); auto ch=sd.chunkCache.getChunkOrGetNull(cp.x, cp.y); if(ch){ uint64_t nid=getEntityIdAndIncrement(worldSaver, EntityType::goblins); GoblinServer s; s.entity=goblin; ch->entityData.goblins.insert({nid,s}); sd.chunkCache.entityChunkPositions[nid]=cp; } }
			{ glm::dvec3 pos=getPos(idx++); Fish fish{}; fish.position=pos; fish.lastPosition=pos; auto cp=glm::ivec2(divideChunk((int)pos.x), divideChunk((int)pos.z)); auto ch=sd.chunkCache.getChunkOrGetNull(cp.x, cp.y); if(ch){ uint64_t nid=getEntityIdAndIncrement(worldSaver, EntityType::fish); FishServer s; s.entity=fish; ch->entityData.fish.insert({nid,s}); sd.chunkCache.entityChunkPositions[nid]=cp; } }
			{ glm::dvec3 pos=getPos(idx++); ScareCrow sc{}; sc.position=pos; sc.lastPosition=pos; auto cp=glm::ivec2(divideChunk((int)pos.x), divideChunk((int)pos.z)); auto ch=sd.chunkCache.getChunkOrGetNull(cp.x, cp.y); if(ch){ uint64_t nid=getEntityIdAndIncrement(worldSaver, EntityType::scareCrow); ScareCrowServer s; s.entity=sc; ch->entityData.scareCrows.insert({nid,s}); sd.chunkCache.entityChunkPositions[nid]=cp; } }
			std::cout << "[FlatMobs] Spawnados " << idx << " mobs congelados." << std::endl;
		}
		g_flatMobsRequested = false;
	}
#pragma region send chunks to players

	serverProfiler.startSubProfile("Send Chunks To players");

	std::vector<SendBlocksBack> sendNewBlocksToPlayers;
	bool generateNewChunks = true;
	if (sd.seccondsTimer * targetTicksPerSeccond >= sd.runsPerSeccond)
	{
		generateNewChunks = false; // the server can potentially lag a little, so we stop sending chunks
	}

	if (sd.ticksPerSeccond < 5)
	{
		//make sure we still generate at least a few chunks even though the server is lagging
		generateNewChunks = true;
	}

	updateLoadedChunks(wg, structuresManager, biomesManager, sendNewBlocksToPlayers,
		worldSaver, generateNewChunks, rng);

	serverProfiler.endSubProfile("Send Chunks To players");


#pragma endregion


#pragma region unload chunks
	serverProfiler.startSubProfile("Unload chunks");
	sd.chunkCache.unloadChunksThatNeedUnloading(worldSaver, 2);
	serverProfiler.startSubProfile("Unload chunks");
#pragma endregion



	//here used to be the tasks


	//todo check if there are too many loaded chunks and unload them before processing
	//generate chunk

#pragma region gameplay tick


	while (sd.tickTimer >= 1.f / targetTicksPerSeccond)
	{

	#pragma region set players in their chunks
		for (auto &c : sd.chunkCache.savedChunks)
		{
			c.second->entityData.players.clear();
		}

		//todo move in tick probably
		//set players in their chunks, set players in chunks

		for (auto &client : getAllClientsReff())
		{

			auto cPos = determineChunkThatIsEntityIn(client.second.playerData.entity.position);

			auto chunk = sd.chunkCache.getChunkOrGetNull(cPos.x, cPos.y);

			permaAssertComment(chunk, "Error, A chunk that a player is in unloaded...");

			chunk->entityData.players[client.first] = &client.second.playerData;
			sd.chunkCache.entityChunkPositions[client.first] = cPos;

		}

	#pragma endregion


		//ALL CHUNKS THAT PLAYERS ARE IN SHOULD BE LOADED!!!!


		//for (auto &c : sd.chunkCache.savedChunks)
		//{
		//	c.second->entityData.players.clear();
		//}
		//
		//for (auto &client : getAllClients())
		//{
		//
		//	auto cPos = determineChunkThatIsEntityIn(client.second.playerData.entity.position);
		//	
		//	auto chunk = sd.chunkCache.getChunkOrGetNull(cPos.x, cPos.y);
		//
		//	permaAssertComment(chunk, "Error, A chunk that a player is in unloaded...");
		//
		//	chunk->entityData.players.insert({client.first, &client.second.playerData});
		//
		//}

		//todo if first time ever or not do it if the chunk isn't loaded!
	#pragma region replace spawn position
		//worldSaver.spawnPosition.y = 170;
		//if(0)
		//TODO this should run once at server startup, and also create this chunk,
		// also this should run when someone wants to respawn.
		//just at start
		{

			//wg, structuresManager, biomesManager,
			//sendNewBlocksToPlayers, true, nullptr, worldSaver

			glm::ivec3 spawnPos = worldSaver.spawnPosition;
			auto spawnChunk = sd.chunkCache.getChunkOrGetNull(divideChunk(spawnPos.x),
				divideChunk(spawnPos.z));

			if (!spawnChunk)
			{
				worldSaver.spawnPosition.y = 70;
			}
			else
			{
				glm::ivec3 blockPos = spawnPos;
				blockPos.x = modBlockToChunk(blockPos.x);
				blockPos.z = modBlockToChunk(blockPos.z);

				if (blockPos.y >= CHUNK_HEIGHT)
				{
					worldSaver.spawnPosition.y = CHUNK_HEIGHT;
				}
				else
				{
					if (blockPos.y < 1)
					{
						blockPos.y = 1;
					}

					//try down first
					{
						while (true)
						{
							auto b = spawnChunk->chunk.safeGet(blockPos.x, blockPos.y, blockPos.z);

							if (!b)
							{
								break;
							}

							if (!b->isColidable())
							{
								auto bunder = spawnChunk->chunk.safeGet(blockPos.x, blockPos.y - 1, blockPos.z);
								if (bunder && !bunder->isColidable())
								{
									blockPos.y--;
								}
								else
								{
									break;
								}
							}
							else
							{
								break;
							}
						}
					}
					
					while (true)
					{
						auto b = spawnChunk->chunk.safeGet(blockPos.x, blockPos.y, blockPos.z);

						if (!b)
						{
							worldSaver.spawnPosition.y = blockPos.y;
							break;
						}

						if (!b->isColidable())
						{
							auto bunder = spawnChunk->chunk.safeGet(blockPos.x, blockPos.y - 1, blockPos.z);
							if (bunder && bunder->isColidable())
							{
								auto bUp = spawnChunk->chunk.safeGet(blockPos.x, blockPos.y + 1, blockPos.z);
								if (!bUp || !bUp->isColidable())
								{
									//good
									worldSaver.spawnPosition.y = blockPos.y;
									break;
								}
							}
						}
						blockPos.y++;
					}

				}
			}

		}
	#pragma endregion

		//keep the /spawn command (and the initial spawn) in sync with the world
		setWorldSpawnPosition(worldSaver.spawnPosition);


		sd.tickTimer -= (1.f / targetTicksPerSeccond);
		sd.tickTimer = std::min(sd.tickTimer, 2.f / targetTicksPerSeccond);

		sd.ticksPerSeccond++;

		if(settings.perClientSettings.size())
		{


			if (settings.perClientSettings.begin()->second.resendInventory)
			{
				settings.perClientSettings.begin()->second.resendInventory = false;
				auto &c = getAllClientsReff();

				sendPlayerInventoryAndIncrementRevision(c.begin()->second);
			}

			if (settings.perClientSettings.begin()->second.damage)
			{
				settings.perClientSettings.begin()->second.damage = false;
				auto &c = getAllClientsReff();

				c.begin()->second.playerData.applyDamageOrLife(-10);
			}

			if (settings.perClientSettings.begin()->second.heal)
			{
				settings.perClientSettings.begin()->second.heal = false;
				auto &c = getAllClientsReff();

				c.begin()->second.playerData.applyDamageOrLife(10);
			}

			if (settings.perClientSettings.begin()->second.generateStructure)
			{
				settings.perClientSettings.begin()->second.generateStructure = false;
				auto &c = getAllClientsReff();

				glm::ivec3 pos = c.begin()->second.playerData.getPosition();
				pos.y -= 21;
					
				StructureToGenerate s;
				s.type = Structure_MinesDungeon;
				s.randomNumber1 = getRandomNumberFloat(rng, 0, 1);
				s.randomNumber2 = getRandomNumberFloat(rng, 0, 1);
				s.randomNumber3 = getRandomNumberFloat(rng, 0, 1);
				s.randomNumber4 = getRandomNumberFloat(rng, 0, 1);
				s.pos = pos;
				s.replaceBlocks = true;

				std::unordered_map<glm::ivec2, SavedChunk *, Ivec2Hash> newCreatedOrLoadedChunks;
				std::vector<glm::ivec3> controlBlocks;
				sd.chunkCache.generateStructure(s, structuresManager, newCreatedOrLoadedChunks,
					sendNewBlocksToPlayers, &controlBlocks);

			}


			//TODO chunks shouldn't be nullptrs so why check them?
			//	// so maybe just perma assert comment at the beginning

			//if (settings.perClientSettings.begin()->second.killApig)
			//{
			//	settings.perClientSettings.begin()->second.killApig = false;
			//
			//	
			//
			//	for (auto &c : sd.chunkCache.savedChunks)
			//	{
			//		if (c.second && c.second->entityData.pigs.size())
			//		{
			//			killEntity(worldSaver, c.second->entityData.pigs.begin()->first);
			//			break;
			//		}
			//	}
			//
			//}
		}



		//todo error and warning logs for server.


		//todo get all clients should probably dissapear.
		auto &clients = getAllClientsReff();

		for (auto &c : clients)
		{
			c.second.playerData.inventory.sanitize();
		}

		splitUpdatesLogic(sd.tickDeltaTime, sd.tickDeltaTimeMs,
			currentTimer, sd.chunkCache, rng(), clients, worldSaver, serverTask,
			serverProfiler);

		sd.tickDeltaTime = 0;
		sd.tickDeltaTimeMs = 0;
	}

	//std::cout << deltaTime << " <- dt / 1/dt-> " << (1.f / (deltaTime)) << "\n";

	//std::cout << seccondsTimer << '\n';

	sd.runsPerSeccond++;

	if (sd.seccondsTimer >= 1)
	{
		sd.seccondsTimer -= 1;
		sd.seccondsTimer = std::min(sd.seccondsTimer, 1.f);
		//std::cout << "Server ticks per seccond: " << sd.ticksPerSeccond << "\n";
		//std::cout << "Server runs per seccond: " << sd.runsPerSeccond << "\n";
		outTicksPerSeccond = sd.ticksPerSeccond;
		sd.ticksPerSeccond = 0;
		sd.runsPerSeccond = 0;
	}

#pragma endregion

	//this are blocks created by new chunks so everyone needs them
	if (!sendNewBlocksToPlayers.empty())
	{
		Packet_PlaceBlocks *newBlocks = new Packet_PlaceBlocks[sendNewBlocksToPlayers.size()];

		Packet packet;
		packet.cid = 0;
		packet.header = headerPlaceBlocks;

		int i = 0;
		for (auto &b : sendNewBlocksToPlayers)
		{
			//todo an option to send multiple blocks per place block
			//std::cout << "Sending block...";

			//Packet packet;
			//packet.cid = 0;
			//packet.header = headerPlaceBlock;
			//
			//Packet_PlaceBlock packetData;
			//packetData.blockPos = b.pos;
			//packetData.blockType = b.block;
			//
			//broadCast(packet, &packetData, sizeof(Packet_PlaceBlock), nullptr, true, channelChunksAndBlocks);

			newBlocks[i].blockPos = b.pos;
			newBlocks[i].blockInfo = b.blockInfo;

			i++;
		}

		broadCast(packet, newBlocks,
			sizeof(Packet_PlaceBlocks) * sendNewBlocksToPlayers.size(),
			nullptr, true, channelChunksAndBlocks);


		delete[] newBlocks;
	}

#pragma region save stuff
	//save one chunk on disk
	serverProfiler.startSubProfile("Save chunk on disk");
	sd.chunkCache.saveNextChunk(worldSaver);
	serverProfiler.endSubProfile("Save chunk on disk");

	//mark all entities as dirty every 5 secconds, so we save them
	//TODO some chunks aren't in the simulation distance so ther's no need to mark them as dirty.
	//so find a way to check if a chunk was inactive since the last update.
	if (sd.saveEntitiesTimer <= 0)
	{
		sd.saveEntitiesTimer = 5;

		for (auto &c : sd.chunkCache.savedChunks)
		{
			c.second->otherData.dirtyEntity = true;
		}

	}
#pragma endregion

	sd.lastTimer = currentTimer;

}



std::uint64_t getTimer()
{
	static const auto start_time = std::chrono::steady_clock::now();
	auto now = std::chrono::steady_clock::now();
	auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time).count();
	return millis;
}


void addCidToServerSettings(std::uint64_t cid)
{
	sd.settings.perClientSettings.insert({cid, {}});
}

void removeCidFromServerSettings(std::uint64_t cid)
{
	sd.settings.perClientSettings.erase(cid);
}


void onPacketDestroyForChunkSending(ENetPacket *packet)
{
	unsigned int userData = (unsigned int)(uintptr_t)packet->userData;
	
	Packet p = {};
	size_t dataSize = 0;
	parsePacket(*packet, p, dataSize);

	auto cid = p.cid;


	auto &clients = getAllClientsReff();

	auto found = clients.find(cid);
	if (found != clients.end())
	{
		auto rez = found->second.chunksPacketPendingConfirmation.erase(userData);
		int a = 0;
	}

	// Custom logic for when the packet is destroyed
	//std::cout << "Packet of size " << packet->dataLength << " was destroyed (acknowledged or dropped)." << std::endl;
}


//adds loaded chunks.
void updateLoadedChunks(
	WorldGenerator &wg,
	StructuresManager &structureManager,
	BiomesManager &biomesManager,
	std::vector<SendBlocksBack> &sendNewBlocksToPlayers,
	WorldSaver &worldSaver, bool generateNewChunks, std::minstd_rand &rng)
{


	constexpr const int MAX_GENERATE = 1;
	constexpr const int MAX_LOAD = 5;
	constexpr const int MAX_CHUNKS_PENDING = 5; //how many packets can be waiting to be sent at one time

	for (auto &c : sd.chunkCache.savedChunks)
	{
		c.second->otherData.shouldUnload = true;
		c.second->otherData.withinSimulationDistance = false;
	}

	auto &clients = getAllClientsReff();


	//todo a better way to prioritize ordering and stuff
	std::vector<glm::ivec2> positions;
	positions.reserve(200);

	std::vector<uint64_t> cids;
	cids.reserve(clients.size());

	for (auto &cl : clients)
	{
		cids.push_back(cl.first);
	}

	std::shuffle(cids.begin(), cids.end(), rng);


	int geenratedThisFrame = 0;
	int loadedThisFrame = 0;
	for (auto cid : cids)
	{

		auto &client = clients[cid];
		//if (c.second.playerData.killed) { continue; }


		glm::ivec2 pos(divideChunk(client.playerData.entity.position.x),
			divideChunk(client.playerData.entity.position.z));
		
		auto playerBlockPos = from3DPointToBlock(client.playerData.entity.position);

		int distance = (client.playerData.entity.chunkDistance/2) + 1;

		auto clientCid = cid;

		//drop chunks that are too far
		{
			for (auto it = client.loadedChunks.begin(); it != client.loadedChunks.end();)
			{

				if (!isChunkInRadius({playerBlockPos.x, playerBlockPos.z}, *it, client.playerData.entity.chunkDistance))
				{
					it = client.loadedChunks.erase(it);
				}
				else
				{
					++it;
				}
			}
		}
		positions.clear();

		for (int i = -distance; i <= distance; i++)
			for (int j = -distance; j <= distance; j++)
			{
				glm::vec2 vect(i, j);
				auto chunkPos = pos + glm::ivec2(i, j);

				if (isChunkInRadius({playerBlockPos.x, playerBlockPos.z}, 
					chunkPos, client.playerData.entity.chunkDistance))
				{
					//if (client.loadedChunks.find(chunkPos) ==
					//	client.loadedChunks.end())
					{
						positions.push_back({chunkPos});
					}
				}
			}
		
		//make sure we send the right chunks if the player is right at the border corner between 4 chunks
		auto posAugmentedForSort = glm::ivec2(divideChunk(playerBlockPos.x-1), divideChunk(playerBlockPos.z-1));
		std::sort(positions.begin(), positions.end(),
			[&](auto &a, auto &b)
		{
			glm::vec2 diff1 = a - posAugmentedForSort;
			float distance1 = glm::dot(diff1, diff1);

			glm::vec2 diff2 = b - posAugmentedForSort;
			float distance2 = glm::dot(diff2, diff2);

			return distance1 < distance2;
		});

		bool generatedChunkPlayerIsIn = 0;
		for (auto chunkPos : positions)
		{
			SavedChunk *c = 0;

			bool generateMoreChunks = true;
			if (geenratedThisFrame >= MAX_GENERATE)generateMoreChunks = false;
			if (loadedThisFrame >= MAX_LOAD)generateMoreChunks = false;

			bool canSendMoreChunks = true;


			if ((generateNewChunks && (generateMoreChunks))
				
				//always generate the chunk that the player is in
				|| (chunkPos == pos))
			{

				if (chunkPos == pos) { generatedChunkPlayerIsIn = true; }

				bool generated = 0;
				bool loaded = 0;

				//generate new chunks! (or load them)
				c = sd.chunkCache.getOrCreateChunk(chunkPos.x, chunkPos.y,
					wg, structureManager, biomesManager, sendNewBlocksToPlayers, 
					worldSaver, &generated, &loaded
				);

				if (generated)
				{
					geenratedThisFrame++;
				}
				
				if(loaded)
				{
					loadedThisFrame++;
				}
			}
		

			//number of chunks that are being sent rn
			//stop sending more chunks than the pending number
			int currentPendingChunks = client.chunksPacketPendingConfirmation.size();
			if (currentPendingChunks > MAX_CHUNKS_PENDING) { canSendMoreChunks = false; }


			//always generate the chunk player is in,
			{

				if (!c)
				{
					c = sd.chunkCache.getChunkOrGetNull(chunkPos.x, chunkPos.y);
				}

				if (c)
				{
					c->otherData.shouldUnload = false;

					if (isChunkInRadius({playerBlockPos.x, playerBlockPos.z},
						chunkPos, getServerSettingsReff().simulationDistanceRadius*2))
					{
						c->otherData.withinSimulationDistance = true;
					}


					//send chunk to player
				#pragma region send chunk to player

					if (client.loadedChunks.find(chunkPos) ==
						client.loadedChunks.end() && (canSendMoreChunks || chunkPos == pos))
					{

						client.loadedChunks.insert(chunkPos);

						Packet packet;
						packet.header = headerRecieveChunk;
						packet.cid = clientCid;

						//if you have modified Packet_RecieveChunk make sure you didn't break this!
						static_assert(sizeof(Packet_RecieveChunk) == sizeof(ChunkData));

						{
							//TODO merge this 2 packets into one!

							client.chunksPacketPendingConfirmation.insert(sd.chunkPacketId);

							sendPacketAndCompress(client.peer, packet, (char *)(&c->chunk),
								sizeof(Packet_RecieveChunk), true, channelChunksAndBlocks,
								onPacketDestroyForChunkSending, sd.chunkPacketId++);


							std::vector<unsigned char> blockData;
							c->blockData.formatBlockData(blockData, c->chunk.x, c->chunk.z);

							if (blockData.size())
							{
								Packet packet;
								packet.header = headerRecieveEntireBlockDataForChunk;

								if (blockData.size() > 100)
								{
									sendPacketAndCompress(client.peer, packet, (char *)blockData.data(),
										blockData.size(), true, channelChunksAndBlocks);
								}
								else
								{
									sendPacket(client.peer, packet, (char *)blockData.data(),
										blockData.size(), true, channelChunksAndBlocks);
								};

							}

						}
					}
				#pragma endregion




				}

			};

			

		};


	}


};
	


//the server commands are handled in commandSystem.cpp

 