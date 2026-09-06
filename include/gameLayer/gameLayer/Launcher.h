#pragma once

#include <string>

enum class LauncherStateEnum : int { MENU = 0, PLAYING = 1, PAUSED = 2, EXIT = 3 };

struct LauncherState
{
	LauncherStateEnum state = LauncherStateEnum::MENU;
	bool showLauncher = true;
	bool loggedIn = false;
	std::string currentUsername;
	std::string currentUUID;

	void logout()
	{
		loggedIn = false;
		currentUsername.clear();
		currentUUID.clear();
	}
};

inline LauncherState& getLauncherState()
{
	static LauncherState s;
	return s;
}

inline LauncherState& getLauncher()
{
	return getLauncherState();
}

inline void renderLauncherUI(ProgramData&) {}

