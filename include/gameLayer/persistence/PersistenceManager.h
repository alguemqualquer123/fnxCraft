#pragma once
#include "IWorldStorage.h"
#include "FileWorldStorage.h"
#include "IPlayerStorage.h"
#include "PlayerStorage.h"
#include "IDatabase.h"
#include "CacheManager.h"
#include "AutosaveManager.h"
#include "BackupManager.h"
#include <memory>

class PersistenceManager
{
public:
	static PersistenceManager &get();
	void init(const std::string &dataRoot, const std::string &cacheRoot, const std::string &backupRoot, const std::string &logRoot);
	void shutdown();

	IWorldStorage *worldStorage();
	IPlayerStorage *playerStorage();
	IDatabase *database();
	CacheManager *cache();
	BackupManager *backups();
	AutosaveManager *autosave();

	bool saveAll();
	bool flush();

private:
	PersistenceManager() = default;
	std::unique_ptr<FileWorldStorage> m_worldStorage;
	std::unique_ptr<PlayerStorage> m_playerStorage;
	std::unique_ptr<IDatabase> m_database;
	std::unique_ptr<CacheManager> m_cache;
	std::unique_ptr<BackupManager> m_backup;
	std::unique_ptr<AutosaveManager> m_autosave;
};
