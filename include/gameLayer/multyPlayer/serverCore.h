#pragma once
#include <cstdint>
#include <string>
#include <functional>
#include <vector>

// Forward declarations
class ServerConsole;

// Server configuration loaded from server.properties
struct ServerConfig
{
	int port = 7771;
	int maxPlayers = 32;
	std::string levelName = "world";
	std::string levelSeed = "";
	std::string gamemode = "survival";
	std::string difficulty = "normal";
	int simulationDistance = 8;
	int viewDistance = 12;
	int randomTickSpeed = 3;
	bool pvp = true;
	bool fireSpread = true;
	int entityLimit = 50;
	std::string serverName = "ourCraft Server";
	std::string motd = "Bem-vindo ao ourCraft!";
	bool whiteList = false;
	bool onlineMode = true;
	int serverTps = 20;
	int autoBackupInterval = 30;
	bool verboseLogging = false;
	int rconPort = 25575;
	std::string rconPassword = "";
	bool enableRcon = false;

	std::string dataDirectory = "data";
	std::string worldDirectory = "";
	std::string playerDirectory = "";
	std::string databasePath = "";
	std::string cacheDirectory = "cache";
	std::string logDirectory = "logs";
	std::string backupDirectory = "backups";
	int autosaveInterval = 300;

	std::string listenAddress = "0.0.0.0";

	bool loadFromFile(const std::string &path);
	bool saveToFile(const std::string &path) const;
	void parseLine(const std::string &line);

	bool loadFromEnv();
	void applyCliArgs(int argc, char *argv[]);

	std::string resolvedWorldDirectory() const;
	std::string resolvedPlayerDirectory() const;
	std::string resolvedDatabasePath() const;
};

// Whitelist entry
struct WhitelistEntry
{
	std::string name;
	std::string uuid;
	std::string addedBy;
	std::string addedAt;
};

// Whitelist manager
class Whitelist
{
public:
	bool loadFromFile(const std::string &path);
	bool saveToFile(const std::string &path) const;

	bool isEnabled() const { return enabled; }
	void setEnabled(bool e) { enabled = e; }

	bool isPlayerAllowed(const std::string &uuid) const;
	bool isPlayerAllowed(const std::string &name, const std::string &uuid) const;

	void addPlayer(const std::string &name, const std::string &uuid,
		const std::string &addedBy);
	void removePlayer(const std::string &uuid);

	const std::vector<WhitelistEntry> &getEntries() const { return entries; }

private:
	bool enabled = false;
	std::vector<WhitelistEntry> entries;
};

// User account
struct UserAccount
{
	std::string uuid;
	std::string username;
	int64_t firstJoin = 0;
	int64_t lastJoin = 0;
	int64_t totalPlaytime = 0;
	std::vector<std::string> ipAddresses;
	bool banned = false;
	std::string banReason;
	int64_t banExpires = 0;
	bool isOp = false;
	int opLevel = 0;
};

// Account manager
class AccountManager
{
public:
	bool loadAllAccounts(const std::string &directory);
	bool saveAccount(const UserAccount &account) const;

	UserAccount *getAccount(const std::string &uuid);
	UserAccount *getOrCreateAccount(const std::string &uuid,
		const std::string &username);

	bool isPlayerBanned(const std::string &uuid) const;
	bool isPlayerOp(const std::string &uuid) const;

	void banPlayer(const std::string &uuid, const std::string &reason,
		int64_t expiresAt = 0);
	void unbanPlayer(const std::string &uuid);
	void setOp(const std::string &uuid, bool isOp, int level = 3);

	void recordConnection(const std::string &uuid, const std::string &ip);
	int getConnectionCount(const std::string &ip, int64_t withinSeconds) const;

private:
	std::unordered_map<std::string, UserAccount> accounts;
	std::string accountsDirectory;
};
