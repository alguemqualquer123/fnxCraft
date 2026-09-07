#pragma once

#include <string>

struct ProgramData;

enum class LauncherStateEnum : int { MENU = 0, PLAYING = 1, PAUSED = 2, EXIT = 3 };

struct LauncherState
{
	LauncherStateEnum state = LauncherStateEnum::MENU;
	bool showLauncher = false;
	bool loggedIn = true;
	std::string currentUsername = "Player";
	std::string currentUUID = "offline-player";

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

void renderLauncherUI(ProgramData&);

