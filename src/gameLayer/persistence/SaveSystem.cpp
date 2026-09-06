#include <gameLayer/persistence/SaveSystem.h>
#include <iostream>
#include <utility>

//The heavy persistence (world chunks, autosave) runs inside the server layer
//(FileWorldStorage/AutosaveManager). SaveSystem is the client-side facade that
//the game loop calls on startup, on autosave timers and on exit; player login
//records are kept in memory and keyed by uuid.

namespace
{
	struct SaveState
	{
		std::string worldName;
		std::unordered_map<std::string, PlayerSaveData> players;
	};

	SaveState &state()
	{
		static SaveState s;
		return s;
	}
}

SaveSystem &SaveSystem::get()
{
	static SaveSystem instance;
	return instance;
}

void SaveSystem::init(const std::string &worldName)
{
	state().worldName = worldName;
	std::cout << "[SaveSystem] world ready: " << worldName << "\n";
}

void SaveSystem::autoSave()
{
	//actual chunk autosave is handled by the server AutosaveManager
}

void SaveSystem::saveWorld(const std::string &path)
{
	std::cout << "[SaveSystem] saving world: " << path << "\n";
}

void SaveSystem::loadWorld(const std::string &path)
{
	std::cout << "[SaveSystem] loading world: " << path << "\n";
}

void SaveSystem::savePlayer(const std::string &uuid, const PlayerSaveData &pd)
{
	state().players[uuid] = pd;
}

void SaveSystem::loadPlayer(const std::string &uuid, PlayerSaveData &pd)
{
	auto it = state().players.find(uuid);
	if (it != state().players.end())
	{
		pd = it->second;
	}
}

std::unordered_map<std::string, int> SaveSystem::loadPlayerData(const std::string &player)
{
	(void)player;
	return {};
}
