#pragma once
#include <atomic>
#include <thread>
#include <functional>
#include <chrono>

class AutosaveManager
{
public:
	AutosaveManager();
	~AutosaveManager();
	void start(int intervalSeconds, std::function<void()> saveFn);
	void stop();
	void triggerNow();
	bool isRunning() const { return m_running; }
private:
	void loop();
	std::atomic<bool> m_running{false};
	std::thread m_thread;
	int m_intervalSec = 300;
	std::function<void()> m_saveFn;
};
