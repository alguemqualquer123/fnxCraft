#pragma once
#include <string>
#include <vector>
#include <functional>
#include <thread>
#include <atomic>
#include <mutex>
#include <queue>
#include <unordered_map>
#include <iostream>
#include <sstream>

enum class ConsoleMessageType
{
	INFO,
	WARNING,
	ERROR,
	DEBUG,
	SUCCESS
};

using ConsoleCommandHandler = std::function<void(
	const std::vector<std::string> &args,
	int opLevel)>;

struct ConsoleCommand
{
	std::string name;
	std::string description;
	ConsoleCommandHandler handler;
	int minOpLevel = 0; // 0 = everyone, 3 = admin
};

class ServerConsole
{
public:
	ServerConsole() = default;
	~ServerConsole() { stop(); }

	void start();
	void stop();

	// Register a command
	void registerCommand(const std::string &name,
		const std::string &description,
		ConsoleCommandHandler handler,
		int minOpLevel = 0);

	// Execute a command string
	void executeCommand(const std::string &line, int opLevel = 3);

	// Output methods (thread-safe)
	void logInfo(const std::string &msg);
	void logWarning(const std::string &msg);
	void logError(const std::string &msg);
	void logDebug(const std::string &msg);
	void logSuccess(const std::string &msg);

	// Raw message with type
	void printMessage(const std::string &msg, ConsoleMessageType type = ConsoleMessageType::INFO);

	// Get registered commands
	const std::vector<ConsoleCommand> &getCommands() const { return commands; }

private:
	std::thread stdinThread;
	std::atomic<bool> running{false};
	std::vector<ConsoleCommand> commands;
	std::mutex outputMutex;

	// Output queue for deferred printing
	std::queue<std::pair<std::string, ConsoleMessageType>> outputQueue;
	std::mutex queueMutex;

	void printToConsole(const std::string &msg, ConsoleMessageType type);
	void processOutputQueue();
};
