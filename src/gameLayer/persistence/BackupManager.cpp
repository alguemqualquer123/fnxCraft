#include <gameLayer/persistence/BackupManager.h>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <vector>
#include <algorithm>
#include <vector>
#include <algorithm>

BackupManager::BackupManager(std::filesystem::path backupRoot) : m_backupRoot(std::move(backupRoot))
{
	std::error_code ec;
	std::filesystem::create_directories(m_backupRoot, ec);
}

bool BackupManager::createWorldBackup(const std::filesystem::path &worldPath, const std::string &worldName)
{
	if (!std::filesystem::exists(worldPath)) return false;
	auto now = std::chrono::system_clock::now();
	std::time_t t = std::chrono::system_clock::to_time_t(now);
	std::tm tm{};
	localtime_r(&t, &tm);
	char buf[64];
	strftime(buf, sizeof(buf), "%Y%m%d_%H%M%S", &tm);
	std::filesystem::path dest = m_backupRoot / (worldName + "_" + buf);
	std::error_code ec;
	std::filesystem::copy(worldPath, dest, std::filesystem::copy_options::recursive, ec);
	return !ec;
}

bool BackupManager::createFullBackup(const std::filesystem::path &dataRoot)
{
	if (!std::filesystem::exists(dataRoot)) return false;
	auto now = std::chrono::system_clock::now();
	std::time_t t = std::chrono::system_clock::to_time_t(now);
	std::tm tm{};
	localtime_r(&t, &tm);
	char buf[64];
	strftime(buf, sizeof(buf), "%Y%m%d_%H%M%S", &tm);
	std::filesystem::path dest = m_backupRoot / ("full_" + std::string(buf));
	std::error_code ec;
	std::filesystem::copy(dataRoot, dest, std::filesystem::copy_options::recursive, ec);
	return !ec;
}

void BackupManager::setRetention(int keepCount) { m_retention = keepCount; }

void BackupManager::pruneOldBackups()
{
	std::vector<std::filesystem::directory_entry> entries;
	std::error_code ec;
	for (auto &e : std::filesystem::directory_iterator(m_backupRoot, ec)) entries.push_back(e);
	std::sort(entries.begin(), entries.end(), [](auto &a, auto &b){ return a.path().string() < b.path().string(); });
	while ((int)entries.size() > m_retention)
	{
		std::filesystem::remove_all(entries.front().path(), ec);
		entries.erase(entries.begin());
	}
}
