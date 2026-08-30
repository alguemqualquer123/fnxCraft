#include <gameLayer/persistence/AutosaveManager.h>

AutosaveManager::AutosaveManager() {}
AutosaveManager::~AutosaveManager() { stop(); }

void AutosaveManager::start(int intervalSeconds, std::function<void()> saveFn)
{
	if (m_running) stop();
	m_intervalSec = intervalSeconds;
	m_saveFn = std::move(saveFn);
	m_running = true;
	m_thread = std::thread(&AutosaveManager::loop, this);
}

void AutosaveManager::stop()
{
	m_running = false;
	if (m_thread.joinable()) m_thread.join();
}

void AutosaveManager::triggerNow()
{
	if (m_saveFn) m_saveFn();
}

void AutosaveManager::loop()
{
	while (m_running)
	{
		for (int i = 0; i < m_intervalSec && m_running; i++)
			std::this_thread::sleep_for(std::chrono::seconds(1));
		if (!m_running) break;
		if (m_saveFn) m_saveFn();
	}
}
