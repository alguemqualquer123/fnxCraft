#include "multyPlayer/serverCore.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <filesystem>
#include <ctime>

// ==================== ServerConfig ====================

static std::string trim(const std::string &s)
{
	size_t start = s.find_first_not_of(" \t\r\n");
	if (start == std::string::npos) return "";
	size_t end = s.find_last_not_of(" \t\r\n");
	return s.substr(start, end - start + 1);
}

static std::string toLower(const std::string &s)
{
	std::string result = s;
	std::transform(result.begin(), result.end(), result.begin(), ::tolower);
	return result;
}

bool ServerConfig::loadFromFile(const std::string &path)
{
	std::ifstream file(path);
	if (!file.is_open())
	{
		std::cout << "[ServerConfig] No config file found at " << path << ", using defaults.\n";
		return false;
	}

	std::string line;
	while (std::getline(file, line))
	{
		line = trim(line);
		if (line.empty() || line[0] == '#') continue;

		parseLine(line);
	}

	return true;
}

void ServerConfig::parseLine(const std::string &line)
{
	size_t eq = line.find('=');
	if (eq == std::string::npos) return;

	std::string key = trim(line.substr(0, eq));
	std::string value = trim(line.substr(eq + 1));

	if (key == "server-port" || key == "server.port" || key == "port") port = std::stoi(value);
	else if (key == "server-ip" || key == "listen-address" || key == "listen_address") listenAddress = value;
	else if (key == "max-players") maxPlayers = std::stoi(value);
	else if (key == "level-name") levelName = value;
	else if (key == "level-seed") levelSeed = value;
	else if (key == "gamemode") gamemode = toLower(value);
	else if (key == "difficulty") difficulty = toLower(value);
	else if (key == "simulation-distance") simulationDistance = std::stoi(value);
	else if (key == "view-distance") viewDistance = std::stoi(value);
	else if (key == "random-tick-speed") randomTickSpeed = std::stoi(value);
	else if (key == "pvp") pvp = (toLower(value) == "true");
	else if (key == "fire-spread") fireSpread = (toLower(value) == "true");
	else if (key == "entity-limit") entityLimit = std::stoi(value);
	else if (key == "server-name") serverName = value;
	else if (key == "motd") motd = value;
	else if (key == "white-list") whiteList = (toLower(value) == "true");
	else if (key == "online-mode") onlineMode = (toLower(value) == "true");
	else if (key == "server-tps") serverTps = std::stoi(value);
	else if (key == "auto-backup-interval") autoBackupInterval = std::stoi(value);
	else if (key == "autosave-interval" || key == "autosave_interval") autosaveInterval = std::stoi(value);
	else if (key == "verbose-logging") verboseLogging = (toLower(value) == "true");
	else if (key == "rcon-port") rconPort = std::stoi(value);
	else if (key == "rcon-password") rconPassword = value;
	else if (key == "enable-rcon") enableRcon = (toLower(value) == "true");
	else if (key == "data-directory" || key == "data_dir") dataDirectory = value;
	else if (key == "world-directory" || key == "world_dir") worldDirectory = value;
	else if (key == "player-directory" || key == "player_dir") playerDirectory = value;
	else if (key == "database-path" || key == "database_path") databasePath = value;
	else if (key == "cache-directory" || key == "cache_dir") cacheDirectory = value;
	else if (key == "log-directory" || key == "log_dir") logDirectory = value;
	else if (key == "backup-directory" || key == "backup_dir") backupDirectory = value;
}

bool ServerConfig::saveToFile(const std::string &path) const
{
	std::ofstream file(path);
	if (!file.is_open()) return false;

	file << "# ourCraft Server Configuration\n";
	file << "server-port=" << port << "\n";
	file << "server-ip=" << listenAddress << "\n";
	file << "max-players=" << maxPlayers << "\n";
	file << "level-name=" << levelName << "\n";
	file << "level-seed=" << levelSeed << "\n";
	file << "gamemode=" << gamemode << "\n";
	file << "difficulty=" << difficulty << "\n";
	file << "simulation-distance=" << simulationDistance << "\n";
	file << "view-distance=" << viewDistance << "\n";
	file << "random-tick-speed=" << randomTickSpeed << "\n";
	file << "pvp=" << (pvp ? "true" : "false") << "\n";
	file << "fire-spread=" << (fireSpread ? "true" : "false") << "\n";
	file << "entity-limit=" << entityLimit << "\n";
	file << "server-name=" << serverName << "\n";
	file << "motd=" << motd << "\n";
	file << "white-list=" << (whiteList ? "true" : "false") << "\n";
	file << "online-mode=" << (onlineMode ? "true" : "false") << "\n";
	file << "server-tps=" << serverTps << "\n";
	file << "autosave-interval=" << autosaveInterval << "\n";
	file << "auto-backup-interval=" << autoBackupInterval << "\n";
	file << "verbose-logging=" << (verboseLogging ? "true" : "false") << "\n";
	file << "enable-rcon=" << (enableRcon ? "true" : "false") << "\n";
	file << "rcon-port=" << rconPort << "\n";
	file << "rcon-password=" << rconPassword << "\n";
	file << "data-directory=" << dataDirectory << "\n";
	file << "world-directory=" << worldDirectory << "\n";
	file << "player-directory=" << playerDirectory << "\n";
	file << "database-path=" << databasePath << "\n";
	file << "cache-directory=" << cacheDirectory << "\n";
	file << "log-directory=" << logDirectory << "\n";
	file << "backup-directory=" << backupDirectory << "\n";

	return true;
}

bool ServerConfig::loadFromEnv()
{
	auto env = [](const char *k) -> std::string
	{
		const char *v = std::getenv(k);
		return v ? std::string(v) : "";
	};
	bool changed = false;
	std::string v;
	if (!(v = env("OURCRAFT_PORT")).empty()) { port = std::stoi(v); changed = true; }
	if (!(v = env("OURCRAFT_MAX_PLAYERS")).empty()) { maxPlayers = std::stoi(v); changed = true; }
	if (!(v = env("OURCRAFT_LEVEL_NAME")).empty()) { levelName = v; changed = true; }
	if (!(v = env("OURCRAFT_DATA_DIR")).empty()) { dataDirectory = v; changed = true; }
	if (!(v = env("OURCRAFT_WORLD_DIR")).empty()) { worldDirectory = v; changed = true; }
	if (!(v = env("OURCRAFT_CACHE_DIR")).empty()) { cacheDirectory = v; changed = true; }
	if (!(v = env("OURCRAFT_BACKUP_DIR")).empty()) { backupDirectory = v; changed = true; }
	if (!(v = env("OURCRAFT_LOG_DIR")).empty()) { logDirectory = v; changed = true; }
	if (!(v = env("OURCRAFT_LISTEN_ADDRESS")).empty()) { listenAddress = v; changed = true; }
	return changed;
}

void ServerConfig::applyCliArgs(int argc, char *argv[])
{
	for (int i = 1; i < argc; i++)
	{
		std::string arg = argv[i];
		auto needVal = [&](std::string &out) -> bool
		{
			if (i + 1 < argc) { out = argv[++i]; return true; }
			return false;
		};
		std::string val;
		if ((arg == "--port" || arg == "--server-port") && needVal(val)) port = std::stoi(val);
		else if (arg == "--world" && needVal(val)) levelName = val;
		else if (arg == "--max-players" && needVal(val)) maxPlayers = std::stoi(val);
		else if (arg == "--data-dir" && needVal(val)) dataDirectory = val;
		else if (arg == "--world-dir" && needVal(val)) worldDirectory = val;
		else if (arg == "--cache-dir" && needVal(val)) cacheDirectory = val;
		else if (arg == "--logs-dir" && needVal(val)) logDirectory = val;
		else if (arg == "--backups-dir" && needVal(val)) backupDirectory = val;
		else if (arg == "--config" && needVal(val)) { /* handled by GamePaths */ }
		else if (arg.rfind("--data-dir=", 0) == 0) dataDirectory = arg.substr(11);
		else if (arg.rfind("--port=", 0) == 0) port = std::stoi(arg.substr(7));
	}
}

std::string ServerConfig::resolvedWorldDirectory() const
{
	if (!worldDirectory.empty()) return worldDirectory;
	if (!dataDirectory.empty()) return (std::filesystem::path(dataDirectory) / "worlds" / levelName).string();
	return (std::filesystem::path("data") / "worlds" / levelName).string();
}

std::string ServerConfig::resolvedPlayerDirectory() const
{
	if (!playerDirectory.empty()) return playerDirectory;
	if (!dataDirectory.empty()) return (std::filesystem::path(dataDirectory) / "players").string();
	return (std::filesystem::path("data") / "players").string();
}

std::string ServerConfig::resolvedDatabasePath() const
{
	if (!databasePath.empty()) return databasePath;
	if (!dataDirectory.empty()) return (std::filesystem::path(dataDirectory) / "databases" / "world.db").string();
	return (std::filesystem::path("data") / "databases" / "world.db").string();
}

// ==================== Whitelist ====================

bool Whitelist::loadFromFile(const std::string &path)
{
	std::ifstream file(path);
	if (!file.is_open()) return false;

	entries.clear();

	// Simple JSON-like format: parse manually to avoid dependency
	std::string content((std::istreambuf_iterator<char>(file)),
		std::istreambuf_iterator<char>());

	// Find "enabled"
	size_t enabledPos = content.find("\"enabled\"");
	if (enabledPos != std::string::npos)
	{
		size_t colon = content.find(':', enabledPos);
		size_t nextComma = content.find_first_of(",}", colon);
		std::string val = trim(content.substr(colon + 1, nextComma - colon - 1));
		enabled = (val == "true");
	}

	// Find players array
	size_t playersPos = content.find("\"players\"");
	if (playersPos == std::string::npos) return true;

	// Parse each player object
	size_t pos = content.find('{', playersPos);
	while (pos != std::string::npos)
	{
		size_t endObj = content.find('}', pos);
		if (endObj == std::string::npos) break;

		std::string obj = content.substr(pos, endObj - pos + 1);
		WhitelistEntry entry;

		auto extract = [&](const std::string &key) -> std::string
		{
			size_t k = obj.find("\"" + key + "\"");
			if (k == std::string::npos) return "";
			size_t colon = obj.find(':', k);
			size_t start = obj.find('"', colon + 1) + 1;
			size_t end = obj.find('"', start);
			return obj.substr(start, end - start);
		};

		entry.name = extract("name");
		entry.uuid = extract("uuid");
		entry.addedBy = extract("added_by");
		entry.addedAt = extract("added_at");

		if (!entry.name.empty())
			entries.push_back(std::move(entry));

		pos = content.find('{', endObj);
	}

	return true;
}

bool Whitelist::saveToFile(const std::string &path) const
{
	std::ofstream file(path);
	if (!file.is_open()) return false;

	file << "{\n";
	file << "  \"enabled\": " << (enabled ? "true" : "false") << ",\n";
	file << "  \"players\": [\n";

	for (size_t i = 0; i < entries.size(); i++)
	{
		const auto &e = entries[i];
		file << "    {\n";
		file << "      \"name\": \"" << e.name << "\",\n";
		file << "      \"uuid\": \"" << e.uuid << "\",\n";
		file << "      \"added_by\": \"" << e.addedBy << "\",\n";
		file << "      \"added_at\": \"" << e.addedAt << "\"\n";
		file << "    }";
		if (i < entries.size() - 1) file << ",";
		file << "\n";
	}

	file << "  ]\n";
	file << "}\n";

	return true;
}

bool Whitelist::isPlayerAllowed(const std::string &uuid) const
{
	if (!enabled) return true;
	for (const auto &e : entries)
	{
		if (e.uuid == uuid) return true;
	}
	return false;
}

bool Whitelist::isPlayerAllowed(const std::string &name, const std::string &uuid) const
{
	if (!enabled) return true;
	for (const auto &e : entries)
	{
		if (e.uuid == uuid || e.name == name) return true;
	}
	return false;
}

void Whitelist::addPlayer(const std::string &name, const std::string &uuid,
	const std::string &addedBy)
{
	// Remove existing entry for this uuid
	removePlayer(uuid);

	WhitelistEntry entry;
	entry.name = name;
	entry.uuid = uuid;
	entry.addedBy = addedBy;

	// Get current time as ISO string
	time_t now = time(nullptr);
	struct tm *t = localtime(&now);
	char buf[64];
	strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", t);
	entry.addedAt = buf;

	entries.push_back(std::move(entry));
}

void Whitelist::removePlayer(const std::string &uuid)
{
	entries.erase(
		std::remove_if(entries.begin(), entries.end(),
			[&](const WhitelistEntry &e) { return e.uuid == uuid; }),
		entries.end());
}

// ==================== AccountManager ====================

bool AccountManager::loadAllAccounts(const std::string &directory)
{
	accountsDirectory = directory;
	accounts.clear();

	try
	{
		if (!std::filesystem::exists(directory))
		{
			std::filesystem::create_directories(directory);
			return true;
		}

		for (const auto &entry : std::filesystem::directory_iterator(directory))
		{
			if (entry.path().extension() != ".json") continue;

			std::ifstream file(entry.path());
			if (!file.is_open()) continue;

			std::string content((std::istreambuf_iterator<char>(file)),
				std::istreambuf_iterator<char>());

			UserAccount acc;

			auto extract = [&](const std::string &key) -> std::string
			{
				size_t k = content.find("\"" + key + "\"");
				if (k == std::string::npos) return "";
				size_t colon = content.find(':', k);
				size_t start = content.find('"', colon + 1) + 1;
				size_t end = content.find('"', start);
				return content.substr(start, end - start);
			};

			auto extractBool = [&](const std::string &key) -> bool
			{
				size_t k = content.find("\"" + key + "\"");
				if (k == std::string::npos) return false;
				size_t colon = content.find(':', k);
				size_t next = content.find_first_of(",}", colon);
				std::string val = trim(content.substr(colon + 1, next - colon - 1));
				return val == "true";
			};

			auto extractInt = [&](const std::string &key) -> int64_t
			{
				size_t k = content.find("\"" + key + "\"");
				if (k == std::string::npos) return 0;
				size_t colon = content.find(':', k);
				size_t next = content.find_first_of(",}", colon);
				std::string val = trim(content.substr(colon + 1, next - colon - 1));
				try { return std::stoll(val); }
				catch (...) { return 0; }
			};

			acc.uuid = extract("uuid");
			acc.username = extract("username");
			acc.firstJoin = extractInt("first_join");
			acc.lastJoin = extractInt("last_join");
			acc.totalPlaytime = extractInt("total_playtime_seconds");
			acc.banned = extractBool("banned");
			acc.banReason = extract("ban_reason");
			acc.banExpires = extractInt("ban_expires");
			acc.isOp = extractBool("is_op");
			acc.opLevel = (int)extractInt("op_level");

			if (!acc.uuid.empty())
				accounts[acc.uuid] = std::move(acc);
		}
	}
	catch (const std::exception &e)
	{
		std::cout << "[AccountManager] Error loading accounts: " << e.what() << "\n";
		return false;
	}

	return true;
}

bool AccountManager::saveAccount(const UserAccount &account) const
{
	std::string path = accountsDirectory + "/" + account.uuid + ".json";
	std::ofstream file(path);
	if (!file.is_open()) return false;

	file << "{\n";
	file << "  \"uuid\": \"" << account.uuid << "\",\n";
	file << "  \"username\": \"" << account.username << "\",\n";
	file << "  \"first_join\": " << account.firstJoin << ",\n";
	file << "  \"last_join\": " << account.lastJoin << ",\n";
	file << "  \"total_playtime_seconds\": " << account.totalPlaytime << ",\n";
	file << "  \"banned\": " << (account.banned ? "true" : "false") << ",\n";
	file << "  \"ban_reason\": \"" << account.banReason << "\",\n";
	file << "  \"ban_expires\": " << account.banExpires << ",\n";
	file << "  \"is_op\": " << (account.isOp ? "true" : "false") << ",\n";
	file << "  \"op_level\": " << account.opLevel << "\n";
	file << "}\n";

	return true;
}

UserAccount *AccountManager::getAccount(const std::string &uuid)
{
	auto it = accounts.find(uuid);
	if (it == accounts.end()) return nullptr;
	return &it->second;
}

UserAccount *AccountManager::getOrCreateAccount(const std::string &uuid,
	const std::string &username)
{
	auto it = accounts.find(uuid);
	if (it != accounts.end())
	{
		it->second.username = username;
		it->second.lastJoin = time(nullptr);
		return &it->second;
	}

	UserAccount acc;
	acc.uuid = uuid;
	acc.username = username;
	acc.firstJoin = time(nullptr);
	acc.lastJoin = time(nullptr);
	auto [inserted, _] = accounts.emplace(uuid, std::move(acc));
	return &inserted->second;
}

bool AccountManager::isPlayerBanned(const std::string &uuid) const
{
	auto it = accounts.find(uuid);
	if (it == accounts.end()) return false;
	if (!it->second.banned) return false;
	if (it->second.banExpires > 0 && it->second.banExpires < time(nullptr))
		return false; // Ban expired
	return true;
}

bool AccountManager::isPlayerOp(const std::string &uuid) const
{
	auto it = accounts.find(uuid);
	if (it == accounts.end()) return false;
	return it->second.isOp;
}

void AccountManager::banPlayer(const std::string &uuid, const std::string &reason,
	int64_t expiresAt)
{
	auto it = accounts.find(uuid);
	if (it == accounts.end()) return;
	it->second.banned = true;
	it->second.banReason = reason;
	it->second.banExpires = expiresAt;
	saveAccount(it->second);
}

void AccountManager::unbanPlayer(const std::string &uuid)
{
	auto it = accounts.find(uuid);
	if (it == accounts.end()) return;
	it->second.banned = false;
	it->second.banReason = "";
	it->second.banExpires = 0;
	saveAccount(it->second);
}

void AccountManager::setOp(const std::string &uuid, bool op, int level)
{
	auto it = accounts.find(uuid);
	if (it == accounts.end()) return;
	it->second.isOp = op;
	it->second.opLevel = op ? level : 0;
	saveAccount(it->second);
}

void AccountManager::recordConnection(const std::string &uuid, const std::string &ip)
{
	auto it = accounts.find(uuid);
	if (it == accounts.end()) return;
	for (const auto &existing : it->second.ipAddresses)
	{
		if (existing == ip) return;
	}
	it->second.ipAddresses.push_back(ip);
}

int AccountManager::getConnectionCount(const std::string &ip, int64_t withinSeconds) const
{
	// This is simplified - in production you'd track connection timestamps
	return 0;
}
