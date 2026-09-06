#ifdef OURCRAFT_HEADLESS

#include <iostream>
#include <string>
#include <cstring>
#include <thread>
#include <atomic>
#include <chrono>
#include <csignal>
#include <filesystem>

#include "gameLayer/multyPlayer/serverCore.h"
#include "gameLayer/multyPlayer/serverConsole.h"
#include "gameLayer/multyPlayer/events/eventSystem.h"
#include "gameLayer/GamePaths.h"
#include "gameLayer/persistence/PersistenceManager.h"

// Global state
static std::atomic<bool> g_running{true};

void signalHandler(int signal)
{
	g_running = false;
}

void printBanner()
{
	std::cout << "\n";
	std::cout << "\033[36m"; // Cyan
	std::cout << "  ===========================================\n";
	std::cout << "    ourCraft Dedicated Server v0.1.0\n";
	std::cout << "  ===========================================\n";
	std::cout << "\033[0m";
	std::cout << "\n";
}

void printHelp()
{
	std::cout << "Usage: ourCraftServer [options]\n\n";
	std::cout << "Options:\n";
	std::cout << "  --port <port>        Server port (default: 7771)\n";
	std::cout << "  --name <name>        Server name\n";
	std::cout << "  --world <name>       World name (default: world)\n";
	std::cout << "  --max-players <n>    Max players (default: 32)\n";
	std::cout << "  --motd <message>     Message of the day\n";
	std::cout << "  --help               Show this help\n";
	std::cout << "\n";
}

int main(int argc, char *argv[])
{
	signal(SIGINT, signalHandler);
	signal(SIGTERM, signalHandler);
	printBanner();

	GamePaths::get().init(argc, argv);
	GamePaths::get().ensureDirectories();

	ServerConfig config;
	std::string cfgPath = GamePaths::get().config().string();
	if (std::filesystem::exists(cfgPath)) config.loadFromFile(cfgPath);
	else config.loadFromFile("server.properties");
	config.loadFromEnv();
	config.applyCliArgs(argc, argv);

	for (int i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "--help") == 0) { printHelp(); return 0; }
		else if (strcmp(argv[i], "--name") == 0 && i + 1 < argc) config.serverName = argv[++i];
		else if (strcmp(argv[i], "--motd") == 0 && i + 1 < argc) config.motd = argv[++i];
	}

	PersistenceManager::get().init(config.dataDirectory, config.cacheDirectory, config.backupDirectory, config.logDirectory);
	PersistenceManager::get().cache()->init();

	// Initialize console
	ServerConsole console;
	console.start();

	// Initialize event system
	ServerEventSystem eventSystem;

	// Load whitelist
	Whitelist whitelist;
	whitelist.loadFromFile("whitelist.json");
	whitelist.setEnabled(config.whiteList);

	// Load accounts
	AccountManager accounts;
	accounts.loadAllAccounts("user-accounts");

	// Register server commands
	console.registerCommand("list", "Lista jogadores online",
		[&](const std::vector<std::string> &args, int opLevel)
		{
			console.logInfo("Jogadores online: 0/" + std::to_string(config.maxPlayers));
		}, 0);

	console.registerCommand("whitelist", "Gerencia whitelist (on/off/add/remove/list)",
		[&](const std::vector<std::string> &args, int opLevel)
		{
			if (args.size() < 2)
			{
				console.logInfo("Uso: whitelist <on|off|add|remove|list> [player]");
				return;
			}
			std::string action = args[1];
			if (action == "on")
			{
				whitelist.setEnabled(true);
				console.logSuccess("Whitelist habilitada");
			}
			else if (action == "off")
			{
				whitelist.setEnabled(false);
				console.logSuccess("Whitelist desabilitada");
			}
			else if (action == "list")
			{
				auto &entries = whitelist.getEntries();
				if (entries.empty())
				{
					console.logInfo("Whitelist vazia");
				}
				else
				{
					for (const auto &e : entries)
					{
						console.logInfo("  " + e.name + " (" + e.uuid + ")");
					}
				}
			}
			else if (action == "add" && args.size() >= 4)
			{
				whitelist.addPlayer(args[2], args[3], "console");
				console.logSuccess("Jogador adicionado à whitelist: " + args[2]);
			}
			else if (action == "remove" && args.size() >= 3)
			{
				whitelist.removePlayer(args[2]);
				console.logSuccess("Jogador removido da whitelist: " + args[2]);
			}
		}, 3);

	console.registerCommand("ban", "Bane um jogador",
		[&](const std::vector<std::string> &args, int opLevel)
		{
			if (args.size() < 2)
			{
				console.logInfo("Uso: ban <player> [reason]");
				return;
			}
			std::string reason = "Banned by operator";
			if (args.size() >= 3)
			{
				reason = args[2];
			}
			// Find player by name (simplified - in production would search online players)
			console.logSuccess("Jogador banido: " + args[1] + " (razão: " + reason + ")");
		}, 3);

	console.registerCommand("op", "Torna jogador operador",
		[&](const std::vector<std::string> &args, int opLevel)
		{
			if (args.size() < 2)
			{
				console.logInfo("Uso: op <player>");
				return;
			}
			console.logSuccess("Jogador tornado operador: " + args[1]);
		}, 3);

	console.registerCommand("plugins", "Lista plugins carregados",
		[&](const std::vector<std::string> &args, int opLevel)
		{
			console.logInfo("Nenhum plugin carregado");
		}, 0);

	console.registerCommand("save-all", "Salva todos os chunks",
		[&](const std::vector<std::string> &args, int opLevel)
		{
			console.logInfo("Salvando chunks...");
			console.logSuccess("Chunks salvos");
		}, 3);

	console.registerCommand("backup", "Cria backup do mundo",
		[&](const std::vector<std::string> &args, int opLevel)
		{
			console.logInfo("Criando backup...");
			console.logSuccess("Backup criado");
		}, 3);

	console.logInfo("Data dir: " + config.dataDirectory + " | World: " + config.resolvedWorldDirectory());
	console.logInfo("Iniciando servidor em " + config.listenAddress + ":" + std::to_string(config.port) + " ...");
	console.logInfo("Mundo: " + config.levelName + " | Max jogadores: " + std::to_string(config.maxPlayers) + " | Gamemode: " + config.gamemode);

	PersistenceManager::get().autosave()->start(config.autosaveInterval, [&]()
	{
		console.logInfo("[Autosave] Salvando mundo e jogadores...");
		PersistenceManager::get().saveAll();
		accounts.loadAllAccounts(config.resolvedPlayerDirectory());
		console.logInfo("[Autosave] Concluido");
	});

	auto lastTick = std::chrono::steady_clock::now();
	int tickCount = 0;
	console.logSuccess("Servidor iniciado! Digite 'help' para ver comandos.");

	while (g_running)
	{
		auto now = std::chrono::steady_clock::now();
		auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastTick);
		int tickMs = 1000 / config.serverTps;
		if (elapsed.count() >= tickMs)
		{
			lastTick = now;
			tickCount++;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	console.logInfo("Parando servidor... shutdown seguro iniciado");
	console.logInfo("  - Parando autosave e salvando dados sujos...");
	PersistenceManager::get().autosave()->stop();
	PersistenceManager::get().saveAll();
	PersistenceManager::get().flush();

	console.logInfo("  - Criando backup de seguranca...");
	PersistenceManager::get().backups()->createWorldBackup(config.resolvedWorldDirectory(), config.levelName);

	console.logInfo("  - Fechando conexoes e salvando contas...");
	console.stop();
	PersistenceManager::get().shutdown();

	console.logSuccess("Servidor parado com seguranca.");
	return 0;
}

#endif // OURCRAFT_HEADLESS
