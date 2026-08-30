#pragma once
#include <filesystem>
#include <string>
#include <chrono>

class BackupManager
{
public:
	explicit BackupManager(std::filesystem::path backupRoot);
	bool createWorldBackup(const std::filesystem::path &worldPath, const std::string &worldName);
	bool createFullBackup(const std::filesystem::path &dataRoot);
	void setRetention(int keepCount);
	void pruneOldBackups();
private:
	std::filesystem::path m_backupRoot;
	int m_retention = 10;
};
