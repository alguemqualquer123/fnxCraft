#pragma once
struct Launcher
{
	static Launcher& instance();
	void update(float dt);
};
