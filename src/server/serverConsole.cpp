#include "multyPlayer/serverConsole.h"
#include <iostream>
#include <algorithm>
#include <cstring>

void ServerConsole::start()
{
	running = true;

	// Register built-in commands
	registerCommand("help", "Mostra comandos disponíveis",
		[this](const std::vector<std::string> &args, int opLevel)
		{
			logInfo("=== Comandos disponíveis ===");
			for (const auto &cmd : commands)
			{
				if (opLevel >= cmd.minOpLevel)
				{
					logInfo("  " + cmd.name + " - " + cmd.description);
				}
			}
		}, 0);

	registerCommand("stop", "Para o servidor",
		[this](const std::vector<std::string> &args, int opLevel)
		{
			logInfo("Parando servidor...");
			running = false;
		}, 3);

	registerCommand("say", "Envia mensagem global",
		[this](const std::vector<std::string> &args, int opLevel)
		{
			std::string msg;
			for (size_t i = 1; i < args.size(); i++)
			{
				if (i > 1) msg += " ";
				msg += args[i];
			}
			if (!msg.empty())
			{
				logInfo("[Server] " + msg);
			}
		}, 1);

	// Start stdin reader thread
	stdinThread = std::thread([this]()
	{
		std::string line;
		while (running)
		{
			if (std::getline(std::cin, line))
			{
				if (!line.empty())
				{
					executeCommand(line);
				}
			}
		}
	});
}

void ServerConsole::stop()
{
	running = false;
	if (stdinThread.joinable())
		stdinThread.join();
}

void ServerConsole::registerCommand(const std::string &name,
	const std::string &description,
	ConsoleCommandHandler handler,
	int minOpLevel)
{
	ConsoleCommand cmd;
	cmd.name = name;
	cmd.description = description;
	cmd.handler = std::move(handler);
	cmd.minOpLevel = minOpLevel;
	commands.push_back(std::move(cmd));
}

void ServerConsole::executeCommand(const std::string &line, int opLevel)
{
	// Parse command
	std::istringstream iss(line);
	std::vector<std::string> args;
	std::string arg;
	while (iss >> arg)
	{
		args.push_back(arg);
	}

	if (args.empty()) return;

	// Find command
	std::string cmdName = args[0];
	std::transform(cmdName.begin(), cmdName.end(), cmdName.begin(), ::tolower);

	for (const auto &cmd : commands)
	{
		if (cmd.name == cmdName)
		{
			if (opLevel < cmd.minOpLevel)
			{
				logError("Sem permissão para executar este comando.");
				return;
			}
			try
			{
				cmd.handler(args, opLevel);
			}
			catch (const std::exception &e)
			{
				logError("Erro ao executar comando: " + std::string(e.what()));
			}
			return;
		}
	}

	logError("Comando não encontrado: " + cmdName + ". Digite 'help' para ver os comandos.");
}

void ServerConsole::logInfo(const std::string &msg)
{
	printMessage("[INFO] " + msg, ConsoleMessageType::INFO);
}

void ServerConsole::logWarning(const std::string &msg)
{
	printMessage("[WARN] " + msg, ConsoleMessageType::WARNING);
}

void ServerConsole::logError(const std::string &msg)
{
	printMessage("[ERROR] " + msg, ConsoleMessageType::ERROR);
}

void ServerConsole::logDebug(const std::string &msg)
{
	printMessage("[DEBUG] " + msg, ConsoleMessageType::DEBUG);
}

void ServerConsole::logSuccess(const std::string &msg)
{
	printMessage("[OK] " + msg, ConsoleMessageType::SUCCESS);
}

void ServerConsole::printMessage(const std::string &msg, ConsoleMessageType type)
{
	std::lock_guard<std::mutex> lock(outputMutex);
	printToConsole(msg, type);
}

void ServerConsole::printToConsole(const std::string &msg, ConsoleMessageType type)
{
	// Use ANSI colors
	switch (type)
	{
	case ConsoleMessageType::INFO:
		std::cout << "\033[37m" << msg << "\033[0m\n";
		break;
	case ConsoleMessageType::WARNING:
		std::cout << "\033[33m" << msg << "\033[0m\n";
		break;
	case ConsoleMessageType::ERROR:
		std::cout << "\033[31m" << msg << "\033[0m\n";
		break;
	case ConsoleMessageType::DEBUG:
		std::cout << "\033[36m" << msg << "\033[0m\n";
		break;
	case ConsoleMessageType::SUCCESS:
		std::cout << "\033[32m" << msg << "\033[0m\n";
		break;
	}
	std::cout.flush();
}
