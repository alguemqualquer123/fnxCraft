#pragma once
#include <string>
#include <filesystem>

class GamePaths
{
public:
	static GamePaths &get();

	void init(int argc, char *argv[]);
	void init(const std::string &rootOverride);

	std::filesystem::path root() const;
	std::filesystem::path server() const;
	std::filesystem::path client() const;
	std::filesystem::path data() const;
	std::filesystem::path worlds() const;
	std::filesystem::path world(const std::string &name) const;
	std::filesystem::path players() const;
	std::filesystem::path databases() const;
	std::filesystem::path cache() const;
	std::filesystem::path logs() const;
	std::filesystem::path backups() const;
	std::filesystem::path config() const;
	std::filesystem::path plugins() const;
	std::filesystem::path resources() const;
	std::filesystem::path playerSettings() const;
	std::filesystem::path userAccounts() const;

	void setRoot(const std::filesystem::path &p);
	void setDataDir(const std::filesystem::path &p);
	void setWorldsDir(const std::filesystem::path &p);
	void setPlayersDir(const std::filesystem::path &p);
	void setCacheDir(const std::filesystem::path &p);
	void setLogsDir(const std::filesystem::path &p);
	void setBackupsDir(const std::filesystem::path &p);
	void setConfigFile(const std::filesystem::path &p);

	void ensureDirectories() const;

private:
	GamePaths() = default;
	void parseCliArgs(int argc, char *argv[]);

	std::filesystem::path m_root;
	std::filesystem::path m_dataDir;
	std::filesystem::path m_worldsDir;
	std::filesystem::path m_playersDir;
	std::filesystem::path m_databasesDir;
	std::filesystem::path m_cacheDir;
	std::filesystem::path m_logsDir;
	std::filesystem::path m_backupsDir;
	std::filesystem::path m_configFile;
	std::filesystem::path m_serverDir;
	std::filesystem::path m_clientDir;
};
