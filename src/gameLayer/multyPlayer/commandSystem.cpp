#include <multyPlayer/commandSystem.h>
#include <multyPlayer/server.h>
#include <multyPlayer/enetServerFunction.h>
#include <multyPlayer/client.h>
#include <multyPlayer/packet.h>
#include <gameplay/player.h>
#include <gameplay/items.h>
#include <gameplay/effects.h>
#include <gamePlayLogic.h>
#include <blocks.h>
#include <glm/glm.hpp>
#include <magic_enum.hpp>
#include <cstring>
#include <cctype>
#include <cstdlib>
#include <cmath>
#include <sstream>
#include <algorithm>
#include <vector>
#include <unordered_map>
#include <set>

//item and block name tables come from items.cpp
extern const char *itemsNames[];
extern char *blockNames[];

static bool systemInitialized = false;

static std::unordered_map<std::string, CommandDefinition> &getRegistry()
{
	static std::unordered_map<std::string, CommandDefinition> registry;
	return registry;
}

static std::vector<CommandDefinition> &getHashedRegistry()
{
	static std::vector<CommandDefinition> commands;
	return commands;
}

const std::vector<CommandDefinition> &getAllCommandDefinitions()
{
	return getHashedRegistry();
}

static void registerCommand(CommandDefinition def)
{
	if (getRegistry().count(def.name)) { return; }
	getRegistry()[def.name] = def;
	getHashedRegistry().push_back(def);
}

bool commandIsRegistred(const std::string &name)
{
	return getRegistry().count(name) != 0;
}

static const std::vector<std::string> &getAllItemsAndBlocksNames()
{
	static std::vector<std::string> all = []()
	{
		std::vector<std::string> out;
		for (int i = 0; i < BlocksCount; i++)
		{
			out.push_back(blockNames[i]);
		}
		for (int i = 0; i < lastItem - ItemsStartPoint; i++)
		{
			out.push_back(itemsNames[i]);
		}
		return out;
	}();
	return all;
}

static const std::vector<std::string> &getAllEffectsNames()
{
	static std::vector<std::string> all = []()
	{
		std::vector<std::string> out;
		for (int i = 0; i < Effects::Effects_Count; i++)
		{
			out.push_back(std::string(magic_enum::enum_name((Effects::EffectsNames)i).substr()));
		}
		return out;
	}();
	return all;
}

//resolves an item or block name to a type that Item constructor understands
static bool resolveItemType(const std::string &name, unsigned short &outType)
{
	auto lower = name;
	std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c){ return std::tolower(c); });

	for (int i = 0; i < BlocksCount; i++)
	{
		if (lower == blockNames[i]) { outType = (unsigned short)i; return true; }
	}
	for (int i = 0; i < lastItem - ItemsStartPoint; i++)
	{
		if (lower == itemsNames[i]) { outType = (unsigned short)(i + ItemsStartPoint); return true; }
	}
	return false;
}

//parses a player arg, returns 0 if not found
static Client *resolvePlayerArg(const CommandContext &ctx, size_t index)
{
	const CommandValue *v = ctx.arg(index);
	if (!v) { return nullptr; }

	std::uint64_t cid = 0;
	if (v->isNumber)
	{
		cid = (std::uint64_t)v->number;
	}
	else
	{
		char *end = nullptr;
		cid = std::strtoull(v->str.c_str(), &end, 10);
		if (end == v->str.c_str() || *end != '\0') { return nullptr; }
	}

	return getClientSafe(cid);
}

static std::vector<std::string> getOnlinePlayerSuggestions()
{
	std::vector<std::string> out;
	auto &clients = getAllClientsReff();
	for (auto &c : clients)
	{
		out.push_back(std::to_string(c.first));
	}
	return out;
}

//tells a client to jump to a position (same mechanism the respawn uses)
static bool sendTeleport(Client *target, std::uint64_t targetCid, const glm::dvec3 &pos)
{
	if (!target)
	{
		return false;
	}

	target->playerData.entity.position = pos;
	target->playerData.entity.lastPosition = pos;

	Packet_RespawnPlayer resp;
	resp.pos = pos;

	Packet p;
	p.cid = targetCid;
	p.header = headerRespawnPlayer;

	sendPacket(target->peer, p, (const char *)&resp, sizeof(resp), true, channelHandleConnections);
	return true;
}

//finds the cid of a connected client (0 if the client wasn't found)
static std::uint64_t findClientCid(Client *target)
{
	if (!target)
	{
		return 0;
	}
	for (auto &c : getAllClientsReff())
	{
		if (&c.second == target)
		{
			return c.first;
		}
	}
	return 0;
}

//attempts to parse one item arg that may span multiple tokens (names like "iron sword")
//returns how many tokens were consumed, or 0 on failure
static int matchItemToken(const std::vector<std::string> &tokens, size_t startIndex,
	unsigned short &outType)
{
	std::string combined;
	for (size_t j = startIndex; j < tokens.size() && j - startIndex < 3; j++)
	{
		if (!combined.empty()) { combined += " "; }
		combined += tokens[j];
		if (resolveItemType(combined, outType))
		{
			return (int)(j - startIndex) + 1;
		}
	}
	return 0;
}

//tries to parse every token against one overload, returns true if it fully matches
static bool tryParseOverload(const std::vector<std::string> &tokens,
	const CommandOverload &overload, std::vector<CommandValue> &outValues)
{
	outValues.clear();

	size_t argIndex = 0;
	size_t tokenIndex = 0;
	size_t initialTokens = tokens.size();

	while (tokenIndex < initialTokens)
	{
		if (argIndex >= overload.args.size())
		{
			//too many arguments
			return false;
		}

		const CommandArg &arg = overload.args[argIndex];

		//the last String argument eats up everything that's left
		bool lastArgString = (arg.type == CommandArgType::String) &&
			(argIndex == overload.args.size() - 1);
		if (lastArgString)
		{
			std::string combined;
			for (size_t i = tokenIndex; i < initialTokens; i++)
			{
				if (!combined.empty()) { combined += " "; }
				combined += tokens[i];
			}
			outValues.push_back(CommandValue(combined));
			tokenIndex = initialTokens;
			argIndex++;
			break;
		}

		const std::string &token = tokens[tokenIndex];
		size_t consumed = 1;

		switch (arg.type)
		{
		case CommandArgType::Number:
		{
			double num = 0;
			char *end = nullptr;
			num = std::strtod(token.c_str(), &end);
			if (end == token.c_str() || *end != '\0') { return false; }
			outValues.push_back(CommandValue(num));
			break;
		}
		case CommandArgType::Enum:
		{
			auto it = std::find(arg.valueNames.begin(), arg.valueNames.end(), token);
			if (it == arg.valueNames.end()) { return false; }
			outValues.push_back(CommandValue(token));
			break;
		}
		case CommandArgType::Item:
		{
			unsigned short type = 0;
			consumed = matchItemToken(tokens, tokenIndex, type);
			if (!consumed) { return false; }

			std::string itemName;
			for (size_t k = tokenIndex; k < tokenIndex + consumed; k++)
			{
				if (!itemName.empty()) { itemName += " "; }
				itemName += tokens[k];
			}
			outValues.push_back(CommandValue(itemName));
			break;
		}
		case CommandArgType::Effect:
		{
			auto it = std::find(arg.valueNames.begin(), arg.valueNames.end(), token);
			if (it == arg.valueNames.end()) { return false; }
			outValues.push_back(CommandValue(token));
			break;
		}
		case CommandArgType::Player:
		case CommandArgType::String:
		default:
		{
			outValues.push_back(CommandValue(token));
			break;
		}
		}

		tokenIndex += consumed;
		argIndex++;
	}

	//check for missing required args
	while (argIndex < overload.args.size())
	{
		if (!overload.args[argIndex].optional) { return false; }
		argIndex++;
	}

	return true;
}

static std::vector<std::string> split(const std::string &s)
{
	std::vector<std::string> out;
	std::istringstream stream(s);
	std::string token;
	while (stream >> token)
	{
		out.push_back(token);
	}
	return out;
}

static std::string jointUsage(const CommandDefinition &def)
{
	std::string usage;
	if (def.overloads.empty()) { return ""; }
	//just show the first overload's args
	for (auto &arg : def.overloads[0].args)
	{
		if (!usage.empty()) { usage += " "; }
		usage += arg.optional ? ("[" + arg.name + "]") : ("<" + arg.name + ">");
	}
	return usage;
}

//checks if the tokens can be parsed as a prefix of one overload's arguments
//(unlike tryParseOverload this allows the args to be incomplete, that's what
// tab completion needs)
static bool tryMatchPrefix(const std::vector<std::string> &tokens, const CommandOverload &overload)
{
	size_t argIndex = 0;
	size_t tokenIndex = 0;

	while (tokenIndex < tokens.size())
	{
		if (argIndex >= overload.args.size())
		{
			return false;
		}

		const CommandArg &arg = overload.args[argIndex];

		bool lastArgString = (arg.type == CommandArgType::String) &&
			(argIndex == overload.args.size() - 1);

		if (lastArgString)
		{
			return true;
		}

		size_t consumed = 1;

		switch (arg.type)
		{
		case CommandArgType::Number:
		{
			double num = 0;
			char *end = nullptr;
			num = std::strtod(tokens[tokenIndex].c_str(), &end);
			if (end == tokens[tokenIndex].c_str() || *end != '\0') { return false; }
			break;
		}
		case CommandArgType::Enum:
		case CommandArgType::Effect:
		{
			bool matched = false;
			for (auto &name : arg.valueNames)
			{
				//allow a partial match here, tab completion then filters on the current word
				if (name == tokens[tokenIndex] || name.rfind(tokens[tokenIndex], 0) == 0)
				{
					matched = true;
					break;
				}
			}
			if (!matched) { return false; }
			break;
		}
		case CommandArgType::Item:
		{
			unsigned short type = 0;
			consumed = matchItemToken(tokens, tokenIndex, type);
			if (!consumed) { return false; }
			break;
		}
		case CommandArgType::Player:
		case CommandArgType::String:
		default:
			break;
		}

		tokenIndex += consumed;
		argIndex++;
	}

	return true;
}

std::string executeServerCommand(std::uint64_t cid, const char *command)
{
	if (!command) { return ""; }
	if (!*command) { return ""; }

	initCommandSystem();

	Client *client = nullptr;
	int commandPermisionLevel = 0;

	if (cid)
	{
		client = getClientSafe(cid);
		if (!client) { return "Error, client not existing, " + std::to_string(cid); }
		commandPermisionLevel = client->playerData.otherPlayerSettings.commandPermisionLevel;
	}
	else
	{
		//command from the server console
		commandPermisionLevel = 3;
	}

	std::string cmd(command);

	std::vector<std::string> tokens = split(cmd);
	if (tokens.empty()) { return ""; }

	std::string name = tokens[0];
	std::transform(name.begin(), name.end(), name.begin(),
		[](unsigned char c){ return std::tolower(c); });

	auto it = getRegistry().find(name);
	if (it == getRegistry().end())
	{
		return "Unknown command \"" + name + "\". Type /help for a list of commands.";
	}

	CommandDefinition &def = it->second;

	if (commandPermisionLevel < def.permissionLevel)
	{
		return "You do not have permission to use that command.";
	}

	std::vector<std::string> rest(tokens.begin() + 1, tokens.end());

	std::vector<CommandValue> parsedValues;
	bool overloadFound = false;
	for (auto &overload : def.overloads)
	{
		if (tryParseOverload(rest, overload, parsedValues))
		{
			overloadFound = true;
			break;
		}
	}

	if (!overloadFound)
	{
		if (rest.empty())
		{
			return "Incomplete command. Usage: /" + name + " " + jointUsage(def);
		}
		return "Invalid arguments for /" + name + ". Usage: /" + name + " " + jointUsage(def);
	}

	CommandContext ctx;
	ctx.cid = cid;
	ctx.client = client;
	ctx.permissionLevel = commandPermisionLevel;
	ctx.values = parsedValues;

	if (!def.handler)
	{
		return "Command not implemented yet.";
	}

	return def.handler(ctx);
}

std::vector<std::string> getCommandSuggestions(int permissionLevel, const std::string &command)
{
	initCommandSystem();

	std::vector<std::string> result;

	if (command.empty())
	{
		for (auto &name : getHashedRegistry())
		{
			if (name.permissionLevel <= permissionLevel)
			{
				result.push_back(name.name);
			}
		}
		return result;
	}

	//are we in the middle of typing the command name?
	bool typedJustCommandName = command.find(' ') == std::string::npos && command.back() != ' ';
	if (typedJustCommandName)
	{
		for (auto &def : getHashedRegistry())
		{
			if (def.permissionLevel > permissionLevel) { continue; }
			if (def.name.rfind(command, 0) == 0)
			{
				result.push_back(def.name);
			}
		}
		//if the command name is fully typed we also want its first args, handled below
	}

	std::vector<std::string> tokens = split(command);
	if (tokens.empty()) { return result; }

	//if we typed the command name but with trailing space, keep going as usual
	bool endsWithSpace = command.back() == ' ';
	std::string currentWord = endsWithSpace ? "" : tokens.back();
	size_t baseCount = endsWithSpace ? tokens.size() : tokens.size() - 1;
	if (baseCount == 0) { return result; }

	std::string name = tokens[0];
	auto it = getRegistry().find(name);
	if (it == getRegistry().end())
	{
		//partial command name (like "gam") -> the name suggestions already cover it
		return result;
	}

	CommandDefinition &def = it->second;
	if (def.permissionLevel > permissionLevel) { return result; }

	//find which overloads match the already typed args (as a prefix)
	std::vector<const CommandOverload *> valid;
	for (auto &overload : def.overloads)
	{
		std::vector<std::string> priorTokens(tokens.begin() + 1, tokens.begin() + baseCount);
		if (tryMatchPrefix(priorTokens, overload))
		{
			valid.push_back(&overload);
		}
	}

	if (valid.empty()) { return result; }

	//the arg we should complete is at position baseCount-1 inside each overload
	int targetIndex = (int)baseCount - 1;

	std::set<std::string> suggestions;

	for (auto *overload : valid)
	{
		const auto &args = overload->args;
		if (targetIndex < 0 || targetIndex >= (int)args.size())
		{
			//all the args are already filled, offer nothing
			continue;
		}

		const CommandArg &arg = args[targetIndex];

		std::vector<std::string> candidates;
		switch (arg.type)
		{
		case CommandArgType::Enum:
		case CommandArgType::Effect:
			candidates = arg.valueNames;
			break;
		case CommandArgType::Item:
			candidates = getAllItemsAndBlocksNames();
			break;
		case CommandArgType::Player:
			candidates = getOnlinePlayerSuggestions();
			break;
		default:
			break;
		}

		for (auto &candidate : candidates)
		{
			if (!currentWord.empty() && candidate.rfind(currentWord, 0) != 0) { continue; }
			suggestions.insert(candidate);
		}
	}

	for (auto &s : suggestions)
	{
		result.push_back(s);
	}

	//only keep up to 32 suggestions
	if (result.size() > 32)
	{
		result.resize(32);
	}

	return result;
}

void initCommandSystem()
{
	if (systemInitialized) { return; }
	systemInitialized = true;

	//the basic effect applying logic shared between give effect and effect
	auto applyEffect = [](CommandContext &ctx, size_t effectIndex, size_t timeIndex, Client *target) -> std::string
	{
		if (!target)
		{
			return std::string("No client was given for the command");
		}

		const CommandValue *effectV = ctx.arg(effectIndex);
		const CommandValue *timeV = ctx.arg(timeIndex);
		if (!effectV || !timeV) { return std::string("Invalid arguments"); }

		int effectId = -1;
		for (int i = 0; i < Effects::Effects_Count; i++)
		{
			if (effectV->str == magic_enum::enum_name((Effects::EffectsNames)i).substr())
			{
				effectId = i;
				break;
			}
		}
		if (effectId < 0) { return "Unknown effect: " + effectV->str; }

		target->playerData.effects.allEffects[effectId].timerMs = timeV->number * 1000;
		updatePlayerEffects(*target);

		return "Applied effect: " + effectV->str + " for " +
			CommandValue(timeV->number).str + " seconds!";
	};

	auto getTargetOrDefault = [](CommandContext &ctx, size_t playerIndex) -> Client *
	{
		if (ctx.arg(playerIndex))
		{
			return resolvePlayerArg(ctx, playerIndex);
		}
		return ctx.client;
	};

	//help
	{
		CommandDefinition def;
		def.name = "help";
		def.description = "Lists all the commands you can use.";
		def.permissionLevel = 0;

		CommandOverload ov;
		ov.args = {};
		def.overloads.push_back(ov);

		def.handler = [](CommandContext &ctx) -> std::string
		{
			std::string list;
			for (auto &def : getHashedRegistry())
			{
				if (def.permissionLevel > ctx.permissionLevel) { continue; }
				if (!list.empty()) { list += ", "; }
				list += "/" + def.name;
			}
			return "Available commands: " + list;
		};

		registerCommand(std::move(def));
	}

	//list
	{
		CommandDefinition def;
		def.name = "list";
		def.description = "Lists all the players that are online.";
		def.permissionLevel = 1;

		CommandOverload ov;
		ov.args = {};
		def.overloads.push_back(ov);

		def.handler = [](CommandContext &ctx) -> std::string
		{
			std::string list;
			int count = 0;
			for (auto &c : getAllClientsReff())
			{
				if (!list.empty()) { list += ", "; }
				list += std::to_string(c.first);
				count++;
			}
			if (count == 0) { return "No players online."; }
			return "Online players (" + std::to_string(count) + "): " + list;
		};

		registerCommand(std::move(def));
	}

	//say
	{
		CommandDefinition def;
		def.name = "say";
		def.description = "Sends a message to all players.";
		def.permissionLevel = 1;

		CommandOverload ov;
		CommandArg msg;
		msg.name = "message";
		msg.type = CommandArgType::String;
		ov.args = { msg };
		def.overloads.push_back(ov);

		def.handler = [](CommandContext &ctx) -> std::string
		{
			const CommandValue *msg = ctx.arg(0);
			if (!msg || msg->str.empty()) { return "Say what?"; }

			std::string text = "<" + (ctx.cid ? ("player " + std::to_string(ctx.cid)) : std::string("server")) + "> " + msg->str;

			Packet newPacket;
			newPacket.cid = 0;
			newPacket.header = headerSendChat;

			broadCast(newPacket, (void *)text.c_str(), text.size() + 1, nullptr, true,
				channelHandleConnections);

			return "Message sent.";
		};

		registerCommand(std::move(def));
	}

	//gamemode
	{
		CommandDefinition def;
		def.name = "gamemode";
		def.description = "Switches your (or another player's) gamemode between survival and creative.";
		def.permissionLevel = 2;

		{
			CommandOverload ov;
			CommandArg mode;
			mode.name = "mode";
			mode.type = CommandArgType::Enum;
			mode.valueNames = { "survival", "creative" };
			ov.args = { mode };
			def.overloads.push_back(ov);
		}

		{
			CommandOverload ov;
			CommandArg mode;
			mode.name = "mode";
			mode.type = CommandArgType::Enum;
			mode.valueNames = { "survival", "creative" };
			CommandArg player;
			player.name = "player";
			player.type = CommandArgType::Player;
			player.optional = true;
			ov.args = { mode, player };
			def.overloads.push_back(ov);
		}

		def.handler = [getTargetOrDefault](CommandContext &ctx) -> std::string
		{
			const CommandValue *mode = ctx.arg(0);
			if (!mode) { return "Invalid arguments"; }

			int gameMode = (mode->str == "creative") ? OtherPlayerSettings::CREATIVE : OtherPlayerSettings::SURVIVAL;

			Client *target = getTargetOrDefault(ctx, 1);
			if (!target) { return "Player not found."; }

			std::uint64_t targetCid = 0;
			for (auto &c : getAllClientsReff())
			{
				if (&c.second == target) { targetCid = c.first; break; }
			}

			changePlayerGameMode(targetCid, gameMode);

			return "Gamemode set to " + mode->str + ".";
		};

		registerCommand(std::move(def));
	}

	//heal
	{
		CommandDefinition def;
		def.name = "heal";
		def.description = "Heals you up to max life.";
		def.permissionLevel = 2;

		CommandOverload ov;
		ov.args = {};
		def.overloads.push_back(ov);

		def.handler = [](CommandContext &ctx) -> std::string
		{
			if (!ctx.client)
			{
				return "No client was given for the command";
			}

			ctx.client->playerData.newLife.life = ctx.client->playerData.newLife.maxLife;
			return "You were healed.";
		};

		registerCommand(std::move(def));
	}

	//effect
	{
		CommandDefinition def;
		def.name = "effect";
		def.description = "Applies an effect to you (or another player) for a number of seconds.";
		def.permissionLevel = 2;

		{
			CommandOverload ov;
			CommandArg effect;
			effect.name = "effect";
			effect.type = CommandArgType::Effect;
			effect.valueNames = getAllEffectsNames();
			CommandArg seconds;
			seconds.name = "seconds";
			seconds.type = CommandArgType::Number;
			ov.args = { effect, seconds };
			def.overloads.push_back(ov);
		}

		{
			CommandOverload ov;
			CommandArg effect;
			effect.name = "effect";
			effect.type = CommandArgType::Effect;
			effect.valueNames = getAllEffectsNames();
			CommandArg seconds;
			seconds.name = "seconds";
			seconds.type = CommandArgType::Number;
			CommandArg player;
			player.name = "player";
			player.type = CommandArgType::Player;
			player.optional = true;
			ov.args = { effect, seconds, player };
			def.overloads.push_back(ov);
		}

		def.handler = [applyEffect, getTargetOrDefault](CommandContext &ctx) -> std::string
		{
			return applyEffect(ctx, 0, 1, getTargetOrDefault(ctx, 2));
		};

		registerCommand(std::move(def));
	}

	//give
	{
		CommandDefinition def;
		def.name = "give";
		def.description = "Gives an item or block: /give <item> [count], /give <player> <item> <count> or /give effect <effect> <seconds>.";
		def.permissionLevel = 2;

		{
			CommandOverload ov;
			CommandArg effect;
			effect.name = "effect";
			effect.type = CommandArgType::Enum;
			effect.valueNames = { "effect" };
			CommandArg effectName;
			effectName.name = "effect";
			effectName.type = CommandArgType::Effect;
			effectName.valueNames = getAllEffectsNames();
			CommandArg seconds;
			seconds.name = "seconds";
			seconds.type = CommandArgType::Number;
			ov.args = { effect, effectName, seconds };
			def.overloads.push_back(ov);
		}

		{
			CommandOverload ov;
			CommandArg item;
			item.name = "item";
			item.type = CommandArgType::Item;
			CommandArg count;
			count.name = "count";
			count.type = CommandArgType::Number;
			count.optional = true;
			ov.args = { item, count };
			def.overloads.push_back(ov);
		}

		{
			CommandOverload ov;
			CommandArg player;
			player.name = "player";
			player.type = CommandArgType::Player;
			CommandArg item;
			item.name = "item";
			item.type = CommandArgType::Item;
			CommandArg count;
			count.name = "count";
			count.type = CommandArgType::Number;
			count.optional = true;
			ov.args = { player, item, count };
			def.overloads.push_back(ov);
		}

		def.handler = [applyEffect](CommandContext &ctx) -> std::string
		{
			const CommandValue *first = ctx.arg(0);
			if (!first) { return "Invalid arguments"; }

			//give effect <effect> <seconds>
			if (first->str == "effect")
			{
				return applyEffect(ctx, 1, 2, ctx.client);
			}

			Client *target = ctx.client;
			unsigned short count = 1;
			unsigned short type = 0;
			std::string itemName;

			//give <item> [count]
			if (ctx.arg(1) && ctx.arg(1)->isNumber)
			{
				itemName = first->str;
				count = (unsigned short)ctx.arg(1)->number;
			}
			//give <item> only
			else if (!ctx.arg(1))
			{
				itemName = first->str;
			}
			//give <player> <item> [count]
			else
			{
				target = resolvePlayerArg(ctx, 0);
				itemName = ctx.arg(1)->str;
				if (ctx.arg(2) && ctx.arg(2)->isNumber)
				{
					count = (unsigned short)ctx.arg(2)->number;
				}
			}

			if (!target)
			{
				return "Player not found.";
			}

			if (count < 1) { count = 1; }

			if (!resolveItemType(itemName, type))
			{
				return "Unknown item: " + itemName;
			}

			Item it = itemCreator(type, count);
			if (!it.counter) { return "Bad item count."; }

			int picked = target->playerData.inventory.tryPickupItem(it);
			sendPlayerInventoryAndIncrementRevision(*target);

			if (picked <= 0)
			{
				return "Inventory is full.";
			}

			return "Gave " + it.getItemName() + " x" + std::to_string(picked) + ".";
		};

		registerCommand(std::move(def));
	}

	//clear
	{
		CommandDefinition def;
		def.name = "clear";
		def.description = "Clears your (or another player's) inventory.";
		def.permissionLevel = 2;

		{
			CommandOverload ov;
			ov.args = {};
			def.overloads.push_back(ov);
		}

		{
			CommandOverload ov;
			CommandArg player;
			player.name = "player";
			player.type = CommandArgType::Player;
			ov.args = { player };
			def.overloads.push_back(ov);
		}

		def.handler = [getTargetOrDefault](CommandContext &ctx) -> std::string
		{
			Client *target = getTargetOrDefault(ctx, 0);
			if (!target) { return "Player not found."; }

			std::memset(&target->playerData.inventory, 0, sizeof(PlayerInventory));
			target->playerData.inventory.sanitize();
			sendPlayerInventoryAndIncrementRevision(*target);

			return "Inventory cleared.";
		};

		registerCommand(std::move(def));
	}

	//tp
	{
		CommandDefinition def;
		def.name = "tp";
		def.description = "Teleports you or another player somewhere: /tp <x> <y> <z>, /tp <player> <x> <y> <z> or /tp <player> <player>.";
		def.permissionLevel = 2;

		{
			CommandOverload ov;
			CommandArg x; x.name = "x"; x.type = CommandArgType::Number;
			CommandArg y; y.name = "y"; y.type = CommandArgType::Number;
			CommandArg z; z.name = "z"; z.type = CommandArgType::Number;
			ov.args = { x, y, z };
			def.overloads.push_back(ov);
		}

		{
			CommandOverload ov;
			CommandArg player; player.name = "player"; player.type = CommandArgType::Player;
			CommandArg x; x.name = "x"; x.type = CommandArgType::Number;
			CommandArg y; y.name = "y"; y.type = CommandArgType::Number;
			CommandArg z; z.name = "z"; z.type = CommandArgType::Number;
			ov.args = { player, x, y, z };
			def.overloads.push_back(ov);
		}

		{
			CommandOverload ov;
			CommandArg player; player.name = "player"; player.type = CommandArgType::Player;
			CommandArg target; target.name = "target"; target.type = CommandArgType::Player;
			ov.args = { player, target };
			def.overloads.push_back(ov);
		}

		def.handler = [](CommandContext &ctx) -> std::string
		{
			//figure out who gets teleported and to where
			Client *who = nullptr;
			std::uint64_t whoCid = 0;
			glm::dvec3 dest;

			if (ctx.arg(0) && ctx.arg(0)->isNumber)
			{
				//tp <x> <y> <z>
				who = ctx.client;
				whoCid = ctx.cid;
				dest = glm::dvec3(ctx.arg(0)->number, ctx.arg(1)->number, ctx.arg(2)->number);
			}
			else if (ctx.arg(0) && ctx.arg(2) && ctx.arg(2)->isNumber)
			{
				//tp <player> <x> <y> <z>
				Client *target = resolvePlayerArg(ctx, 0);
				if (!target) { return "Player not found."; }
				who = target;
				whoCid = findClientCid(target);
				dest = glm::dvec3(ctx.arg(1)->number, ctx.arg(2)->number, ctx.arg(3)->number);
			}
			else if (ctx.arg(0) && ctx.arg(1))
			{
				//tp <player> <player>
				Client *target = resolvePlayerArg(ctx, 0);
				Client *dst = resolvePlayerArg(ctx, 1);
				if (!target || !dst) { return "Player not found."; }
				if (target == dst) { return "That's already the same player!"; }
				who = target;
				whoCid = findClientCid(target);
				dest = dst->playerData.entity.position;
			}
			else
			{
				return "Invalid arguments";
			}

			if (!who)
			{
				return "No client was given for the command";
			}

			if (!sendTeleport(who, whoCid, dest))
			{
				return "Couldn't teleport the player.";
			}

			return "Teleported to " + std::to_string((long long)dest.x) + ", " +
				std::to_string((long long)dest.y) + ", " + std::to_string((long long)dest.z) + ".";
		};

		registerCommand(std::move(def));
	}

	//kill
	{
		CommandDefinition def;
		def.name = "kill";
		def.description = "Kills another player.";
		def.permissionLevel = 3;

		CommandOverload ov;
		CommandArg player;
		player.name = "player";
		player.type = CommandArgType::Player;
		ov.args = { player };
		def.overloads.push_back(ov);

		def.handler = [](CommandContext &ctx) -> std::string
		{
			Client *target = resolvePlayerArg(ctx, 0);
			if (!target) { return "Player not found."; }

			if (ctx.client == target) { return "You can't kill yourself that way!"; }

			target->playerData.applyDamageOrLife(-999999);
			return "Player killed.";
		};

		registerCommand(std::move(def));
	}

	//kick
	{
		CommandDefinition def;
		def.name = "kick";
		def.description = "Kicks another player from the server.";
		def.permissionLevel = 3;

		CommandOverload ov;
		CommandArg player;
		player.name = "player";
		player.type = CommandArgType::Player;
		ov.args = { player };
		def.overloads.push_back(ov);

		def.handler = [](CommandContext &ctx) -> std::string
		{
			Client *target = resolvePlayerArg(ctx, 0);
			if (!target) { return "Player not found."; }

			if (ctx.client == target) { return "You can't kick yourself!"; }

			enet_peer_disconnect(target->peer, 0);
			return "Player kicked.";
		};

		registerCommand(std::move(def));
	}

	//pos / coords
	{
		CommandDefinition def;
		def.name = "pos";
		def.description = "Shows your position. Example: /pos";
		def.permissionLevel = 0;

		CommandOverload ov;
		ov.args = {};
		def.overloads.push_back(ov);

		def.handler = [](CommandContext &ctx) -> std::string
		{
			if (!ctx.client)
			{
				return "No client was given for the command";
			}

			glm::dvec3 p = ctx.client->playerData.entity.position;
			return "X: " + std::to_string((long long)p.x) +
				" Y: " + std::to_string((long long)p.y) +
				" Z: " + std::to_string((long long)p.z) + ".";
		};

		registerCommand(std::move(def));
	}

	//seed
	{
		CommandDefinition def;
		def.name = "seed";
		def.description = "Shows the world seed. Example: /seed";
		def.permissionLevel = 2;

		CommandOverload ov;
		ov.args = {};
		def.overloads.push_back(ov);

		def.handler = [](CommandContext &ctx) -> std::string
		{
			return "World seed: " + std::to_string(getWorldSeed()) + ".";
		};

		registerCommand(std::move(def));
	}

	//spawn
	{
		CommandDefinition def;
		def.name = "spawn";
		def.description = "Teleports you back to the world spawn. Example: /spawn";
		def.permissionLevel = 2;

		CommandOverload ov;
		ov.args = {};
		def.overloads.push_back(ov);

		def.handler = [](CommandContext &ctx) -> std::string
		{
			if (!ctx.client)
			{
				return "No client was given for the command";
			}

			glm::ivec3 spawn = getWorldSpawnPosition();
			glm::dvec3 targetPos = glm::dvec3((double)spawn.x, (double)spawn.y, (double)spawn.z);

			if (!sendTeleport(ctx.client, ctx.cid, targetPos))
			{
				return "Couldn't teleport the player.";
			}

			return "Teleported to spawn.";
		};

		registerCommand(std::move(def));
	}

	//fly
	{
		CommandDefinition def;
		def.name = "fly";
		def.description = "Enables or disables flying (creative only in vanilla). Example: /fly or /fly <player>";
		def.permissionLevel = 2;

		{
			CommandOverload ov;
			ov.args = {};
			def.overloads.push_back(ov);
		}

		{
			CommandOverload ov;
			CommandArg player;
			player.name = "player";
			player.type = CommandArgType::Player;
			ov.args = { player };
			def.overloads.push_back(ov);
		}

		def.handler = [getTargetOrDefault](CommandContext &ctx) -> std::string
		{
			Client *target = getTargetOrDefault(ctx, 0);
			if (!target)
			{
				return "Player not found.";
			}

			int currentlyFlying = target->playerData.entity.fly;
			int nextFlying = currentlyFlying ? 0 : 1;

			target->playerData.entity.fly = nextFlying;

			Packet_Fly flyData;
			flyData.fly = (char)nextFlying;

			Packet p;
			p.cid = findClientCid(target);
			p.header = headerFly;

			sendPacket(target->peer, p, (const char *)&flyData, sizeof(flyData), true, channelPlayerPositions);

			if (nextFlying)
			{
				return "Flying enabled.";
			}
			else
			{
				return "Flying disabled.";
			}
		};

		registerCommand(std::move(def));
	}

	//op / deop
	{
		auto setOpLevel = [](const CommandContext &ctx, int level) -> std::string
		{
			Client *target = resolvePlayerArg(ctx, 0);
			if (!target)
			{
				return "Player not found.";
			}

			target->playerData.otherPlayerSettings.commandPermisionLevel = (char)level;

			std::uint64_t targetCid = 0;
			for (auto &c : getAllClientsReff())
			{
				if (&c.second == target) { targetCid = c.first; break; }
			}

			if (level >= 3)
			{
				return "Opped " + std::to_string(targetCid) + ".";
			}

			return "Deopped " + std::to_string(targetCid) + ".";
		};

		//op
		{
			CommandDefinition def;
			def.name = "op";
			def.description = "Gives a player operator powers. Example: /op <player>";
			def.permissionLevel = 3;

			CommandOverload ov;
			CommandArg player;
			player.name = "player";
			player.type = CommandArgType::Player;
			ov.args = { player };
			def.overloads.push_back(ov);

			def.handler = [setOpLevel](CommandContext &ctx) -> std::string
			{
				if (!ctx.arg(0))
				{
					return "You need to specify a player.";
				}

				return setOpLevel(ctx, 3);
			};

			registerCommand(std::move(def));
		}

		//deop
		{
			CommandDefinition def;
			def.name = "deop";
			def.description = "Removes operator powers. Example: /deop <player>";
			def.permissionLevel = 3;

			CommandOverload ov;
			CommandArg player;
			player.name = "player";
			player.type = CommandArgType::Player;
			ov.args = { player };
			def.overloads.push_back(ov);

			def.handler = [setOpLevel](CommandContext &ctx) -> std::string
			{
				if (!ctx.arg(0))
				{
					return "You need to specify a player.";
				}

				return setOpLevel(ctx, 0);
			};

			registerCommand(std::move(def));
		}
	}

	//time
	{
		CommandDefinition def;
		def.name = "time";
		def.description = "Sets the time of day: /time day, /time night, /time noon, /time midnight or /time set <hour 0-24>.";
		def.permissionLevel = 2;

		{
			CommandOverload ov;
			CommandArg time;
			time.name = "time";
			time.type = CommandArgType::Enum;
			time.valueNames = { "day", "night", "noon", "midnight" };
			ov.args = { time };
			def.overloads.push_back(ov);
		}

		{
			CommandOverload ov;
			CommandArg set;
			set.name = "set";
			set.type = CommandArgType::Enum;
			set.valueNames = { "set" };
			CommandArg hour;
			hour.name = "hour";
			hour.type = CommandArgType::Number;
			ov.args = { set, hour };
			def.overloads.push_back(ov);
		}

		def.handler = [](CommandContext &ctx) -> std::string
		{
			const CommandValue *first = ctx.arg(0);
			if (!first) { return "Invalid arguments"; }

			float dayTime = 0.22f;
			std::string message = "Time set to day.";

			if (first->isNumber)
			{
				dayTime = std::fmod((float)first->number, 24.f) / 24.f;
				message = "Time set to " + CommandValue(dayTime * 24.f).str + " o'clock.";
				setWorldDayTime(dayTime);
				return message;
			}

			if (first->str == "day")
			{
				dayTime = 0.22f;
				message = "Time set to day.";
			}
			else if (first->str == "night")
			{
				dayTime = 0.75f;
				message = "Time set to night.";
			}
			else if (first->str == "noon")
			{
				dayTime = 0.5f;
				message = "Time set to noon.";
			}
			else if (first->str == "midnight")
			{
				dayTime = 0.0f;
				message = "Time set to midnight.";
			}
			else if (first->str == "set")
			{
				const CommandValue *hour = ctx.arg(1);
				if (!hour)
				{
					return "Usage: /time set <hour> (0-24)";
				}

				dayTime = std::fmod((float)hour->number, 24.f) / 24.f;
				message = "Time set to " + CommandValue(dayTime * 24.f).str + " o'clock.";
				setWorldDayTime(dayTime);
				return message;
			}
			else
			{
				return "Invalid arguments";
			}

			setWorldDayTime(dayTime);
			return message;
		};

		registerCommand(std::move(def));
	}
}

void setWorldDayTime(float dayTime)
{
	setDayTimeGlobally(dayTime);
}