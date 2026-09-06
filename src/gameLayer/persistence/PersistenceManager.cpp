#include <gameLayer/persistence/PersistenceManager.h>
#include <gameLayer/GamePaths.h>

PersistenceManager &PersistenceManager::get()
{
	static PersistenceManager inst;
	return inst;
}

void PersistenceManager::init(const std::string &dataRoot, const std::string &cacheRoot, const std::string &backupRoot, const std::string &logRoot)
{
	std::filesystem::path dataPath = dataRoot.empty() ? GamePaths::get().data() : std::filesystem::path(dataRoot);
	std::filesystem::path worldPath = dataPath / "worlds" / "default";
	m_worldStorage = std::make_unique<FileWorldStorage>(worldPath);
	m_playerStorage = std::make_unique<PlayerStorage>(dataPath / "players");
	m_database = std::make_unique<NullDatabase>();
	m_cache = std::make_unique<CacheManager>(cacheRoot.empty() ? GamePaths::get().cache() : std::filesystem::path(cacheRoot));
	m_cache->init();
	m_backup = std::make_unique<BackupManager>(backupRoot.empty() ? GamePaths::get().backups() : std::filesystem::path(backupRoot));
	m_autosave = std::make_unique<AutosaveManager>();
}

void PersistenceManager::shutdown()
{
	if (m_autosave) m_autosave->stop();
	saveAll();
	flush();
}

IWorldStorage *PersistenceManager::worldStorage() { return m_worldStorage.get(); }
IPlayerStorage *PersistenceManager::playerStorage() { return m_playerStorage.get(); }
IDatabase *PersistenceManager::database() { return m_database.get(); }
CacheManager *PersistenceManager::cache() { return m_cache.get(); }
BackupManager *PersistenceManager::backups() { return m_backup.get(); }
AutosaveManager *PersistenceManager::autosave() { return m_autosave.get(); }

bool PersistenceManager::saveAll()
{
	if (m_worldStorage) m_worldStorage->flush();
	return true;
}

bool PersistenceManager::flush()
{
	if (m_worldStorage) m_worldStorage->flush();
	return true;
}
