#pragma once
#include "IPlayerStorage.h"
#include <filesystem>
#include <mutex>

class PlayerStorage : public IPlayerStorage
{
public:
	explicit PlayerStorage(std::filesystem::path root);
	std::optional<PlayerData> load(const PlayerId &id) override;
	bool save(const PlayerData &data) override;
	bool remove(const PlayerId &id) override;
	bool exists(const PlayerId &id) const override;
	std::vector<PlayerId> listAll() const override;

private:
	std::filesystem::path fileFor(const PlayerId &id) const;
	std::filesystem::path m_root;
	mutable std::mutex m_mutex;
};
