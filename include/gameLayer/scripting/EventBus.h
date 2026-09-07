#pragma once
#include <string>
#include <functional>
#include <unordered_map>
#include <vector>

struct EventBus
{
	static EventBus& instance();
	void trigger(const std::string& event);
	void subscribe(const std::string& event, std::function<void()> callback);
	void addEventHandler(const std::string& event, std::function<void()> callback);
	std::unordered_map<std::string, std::vector<std::function<void()>>> callbacks;
};
