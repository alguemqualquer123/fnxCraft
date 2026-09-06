#pragma once
#include <string>
#include <vector>
#include <functional>
#include <cstdint>

//command arguments
enum class CommandArgType
{
	String,		//any identifier, if it's the last arg it can contain spaces
	Number,		//a numeric value
	Enum,		//one of CommandArg.valueNames
	Item,		//an item or block name
	Effect,		//an effect name
	Player,		//a player id, either numeric or an online player's number
};

struct CommandArg
{
	std::string name = "";
	CommandArgType type = CommandArgType::String;
	std::vector<std::string> valueNames = {}; //for Enum/Effect/Item (static suggestions)
	bool optional = false;
};

struct CommandOverload
{
	std::vector<CommandArg> args = {};
};

//a parsed value
struct CommandValue
{
	std::string str = "";
	double number = 0;
	bool isNumber = true;

	CommandValue() {}
	CommandValue(const std::string &s){ str = s; isNumber = false; }
	CommandValue(double n){ number = n; str = std::to_string((long long)n); isNumber = true; }
};

struct CommandContext
{
	std::uint64_t cid = 0;
	struct Client *client = nullptr; //this can be null if the command comes from the console
	int permissionLevel = 0;
	std::vector<CommandValue> values = {}; //parsed args in order

	const CommandValue *arg(size_t index) const
	{
		if (index < values.size()) { return &values[index]; }
		return nullptr;
	}
};

struct CommandDefinition
{
	std::string name = "";
	std::string description = "";
	int permissionLevel = 1; //0 nothing, 1 basic, 2 admin, 3 main admin
	std::vector<CommandOverload> overloads = {};
	std::function<std::string(CommandContext &)> handler = {};
};

//registers all the built-in commands, safe to call multiple times
void initCommandSystem();

//executes a / command, returns a chat message to show to the player (empty means nothing to say)
//command is everything AFTER the leading slash
std::string executeServerCommand(std::uint64_t cid, const char *command);

//tab-completion: command is everything after the '/' and it's the complete current input
std::vector<std::string> getCommandSuggestions(int permissionLevel, const std::string &command);

//for the help command
const std::vector<CommandDefinition> &getAllCommandDefinitions();

//used by the /time command, implemented on the client side
void setWorldDayTime(float dayTime);

//used by weather commands
void setWeatherType(int type);
float getWetness();
void setWetness(float w);