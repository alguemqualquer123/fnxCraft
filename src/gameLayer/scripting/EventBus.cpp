#include <scripting/EventBus.h>

EventBus &EventBus::instance()
{
	static EventBus instance;
	return instance;
}

void EventBus::trigger(const std::string &event)
{
	auto it = callbacks.find(event);
	if (it == callbacks.end()) return;

	//copy so callbacks can subscribe/unsubscribe while events are fired
	auto handlers = it->second;
	for (auto &handler : handlers)
	{
		if (handler) handler();
	}
}

void EventBus::subscribe(const std::string &event, std::function<void()> callback)
{
	callbacks[event].push_back(std::move(callback));
}
void EventBus::addEventHandler(const std::string &event, std::function<void()> callback)
{
	callbacks[event].push_back(std::move(callback));
}