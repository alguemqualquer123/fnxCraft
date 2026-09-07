#pragma once
#include <string>
#include <unordered_map>
#include <glm/vec3.hpp>
struct PlayerSaveData
{
	std::string username;
	std::string uuid;
	glm::dvec3 position = {};
	bool hasPosition = false;
};

struct SaveSystem
{
	static SaveSystem& get();
	void init(const std::string& worldName);
	void autoSave();
	void saveWorld(const std::string& path);
	void loadWorld(const std::string& path);
	void savePlayer(const std::string& uuid, const PlayerSaveData& pd);
	void loadPlayer(const std::string& uuid, PlayerSaveData& pd);
	std::unordered_map<std::string, int> loadPlayerData(const std::string& player);
};
