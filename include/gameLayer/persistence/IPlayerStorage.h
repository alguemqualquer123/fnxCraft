#pragma once
#include <string>
#include <vector>
#include <optional>

using PlayerId = std::string;

struct PlayerData
{
	PlayerId id;
	std::string username;
	std::vector<char> blob;
	int64_t lastSeen = 0;
};

class IPlayerStorage
{
public:
	virtual ~IPlayerStorage() = default;
	virtual std::optional<PlayerData> load(const PlayerId &id) = 0;
	virtual bool save(const PlayerData &data) = 0;
	virtual bool remove(const PlayerId &id) = 0;
	virtual bool exists(const PlayerId &id) const = 0;
	virtual std::vector<PlayerId> listAll() const = 0;
};
