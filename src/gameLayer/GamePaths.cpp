#include <gameLayer/GamePaths.h>
#include <cstdlib>
#include <iostream>

GamePaths &GamePaths::get()
{
	static GamePaths instance;
	return instance;
}

void GamePaths::init(int argc, char *argv[])
{
	if (m_root.empty())
		m_root = std::filesystem::current_path();

	const char *envRoot = std::getenv("FNXCRAFT_ROOT");
	if (envRoot && envRoot[0]) m_root = envRoot;

	const char *envData = std::getenv("FNXCRAFT_DATA_DIR");
	if (envData && envData[0]) m_dataDir = envData;

	const char *envCache = std::getenv("FNXCRAFT_CACHE_DIR");
	if (envCache && envCache[0]) m_cacheDir = envCache;

	parseCliArgs(argc, argv);
}

void GamePaths::init(const std::string &rootOverride)
{
	if (!rootOverride.empty()) m_root = rootOverride;
	else if (m_root.empty()) m_root = std::filesystem::current_path();
}

void GamePaths::parseCliArgs(int argc, char *argv[])
{
	for (int i = 1; i < argc; i++)
	{
		std::string arg = argv[i];
		auto next = [&](std::string &out) -> bool
		{
			if (i + 1 < argc) { out = argv[++i]; return true; }
			return false;
		};
		std::string val;
		if (arg == "--data-dir" && next(val)) m_dataDir = val;
		else if (arg == "--worlds-dir" && next(val)) m_worldsDir = val;
		else if (arg == "--players-dir" && next(val)) m_playersDir = val;
		else if (arg == "--cache-dir" && next(val)) m_cacheDir = val;
		else if (arg == "--logs-dir" && next(val)) m_logsDir = val;
		else if (arg == "--backups-dir" && next(val)) m_backupsDir = val;
		else if (arg == "--config" && next(val)) m_configFile = val;
		else if (arg.rfind("--data-dir=", 0) == 0) m_dataDir = arg.substr(11);
		else if (arg.rfind("--cache-dir=", 0) == 0) m_cacheDir = arg.substr(12);
	}
}

std::filesystem::path GamePaths::root() const
{
	if (!m_root.empty()) return m_root;
	return std::filesystem::current_path();
}

std::filesystem::path GamePaths::server() const
{
	if (!m_serverDir.empty()) return m_serverDir;
	return root() / "server";
}

std::filesystem::path GamePaths::client() const
{
	if (!m_clientDir.empty()) return m_clientDir;
	return root() / "client";
}

std::filesystem::path GamePaths::data() const
{
	if (!m_dataDir.empty()) return m_dataDir;
	return root() / "data";
}

std::filesystem::path GamePaths::worlds() const
{
	if (!m_worldsDir.empty()) return m_worldsDir;
	return data() / "worlds";
}

std::filesystem::path GamePaths::world(const std::string &name) const
{
	return worlds() / name;
}

std::filesystem::path GamePaths::players() const
{
	if (!m_playersDir.empty()) return m_playersDir;
	return data() / "players";
}

std::filesystem::path GamePaths::databases() const
{
	if (!m_databasesDir.empty()) return m_databasesDir;
	return data() / "databases";
}

std::filesystem::path GamePaths::cache() const
{
	if (!m_cacheDir.empty()) return m_cacheDir;
	return root() / "cache";
}

std::filesystem::path GamePaths::logs() const
{
	if (!m_logsDir.empty()) return m_logsDir;
	return root() / "logs";
}

std::filesystem::path GamePaths::backups() const
{
	if (!m_backupsDir.empty()) return m_backupsDir;
	return root() / "backups";
}

std::filesystem::path GamePaths::config() const
{
	if (!m_configFile.empty()) return m_configFile;
	return root() / "server.properties";
}

std::filesystem::path GamePaths::plugins() const
{
	return server() / "plugins";
}

std::filesystem::path GamePaths::resources() const
{
	return root() / "resources";
}

std::filesystem::path GamePaths::playerSettings() const
{
	return root() / "playerSettings";
}

std::filesystem::path GamePaths::userAccounts() const
{
	return root() / "user-accounts";
}

void GamePaths::setRoot(const std::filesystem::path &p) { m_root = p; }
void GamePaths::setDataDir(const std::filesystem::path &p) { m_dataDir = p; }
void GamePaths::setWorldsDir(const std::filesystem::path &p) { m_worldsDir = p; }
void GamePaths::setPlayersDir(const std::filesystem::path &p) { m_playersDir = p; }
void GamePaths::setCacheDir(const std::filesystem::path &p) { m_cacheDir = p; }
void GamePaths::setLogsDir(const std::filesystem::path &p) { m_logsDir = p; }
void GamePaths::setBackupsDir(const std::filesystem::path &p) { m_backupsDir = p; }
void GamePaths::setConfigFile(const std::filesystem::path &p) { m_configFile = p; }

void GamePaths::ensureDirectories() const
{
	std::error_code ec;
	std::filesystem::create_directories(data(), ec);
	std::filesystem::create_directories(worlds(), ec);
	std::filesystem::create_directories(players(), ec);
	std::filesystem::create_directories(databases(), ec);
	std::filesystem::create_directories(cache(), ec);
	std::filesystem::create_directories(logs(), ec);
	std::filesystem::create_directories(backups(), ec);
	std::filesystem::create_directories(players(), ec);
}
