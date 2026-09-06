#pragma once
#include <string>
#include <functional>

struct ScriptingIntegration;

struct Scripting
{
	static bool init();
	static void update(float deltaTime);
	static void shutdown();
	static ScriptingIntegration& integration();
};

struct ScriptingIntegration
{
	static ScriptingIntegration& instance();
	void loadScript(const std::string& path);
	void execute(const std::string& code);
	void trigger(const std::string& event);
	std::unordered_map<std::string, std::function<void()>> callbacks;
};

inline ScriptingIntegration& ScriptingIntegration::instance()
{
	static ScriptingIntegration s_instance;
	return s_instance;
}

inline bool Scripting::init()
{
	// TODO: integrate Lua/lua integration later
	return true;
}

inline void Scripting::update(float deltaTime)
{
	// TODO: call Lua update later
}

inline void Scripting::shutdown()
{
	// TODO: shutdown Lua later
}

inline ScriptingIntegration& Scripting::integration()
{
	return ScriptingIntegration::instance();
}
