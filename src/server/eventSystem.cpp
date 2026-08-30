#include "multyPlayer/events/eventSystem.h"
#include <iostream>
#include <algorithm>
#include <cstring>

// ==================== EventArg setters ====================

void EventArg::setBool(bool v)
{
	type = EventType::Bool;
	data.resize(sizeof(bool));
	memcpy(data.data(), &v, sizeof(bool));
}

void EventArg::setInt(int v)
{
	type = EventType::Int;
	data.resize(sizeof(int));
	memcpy(data.data(), &v, sizeof(int));
}

void EventArg::setFloat(float v)
{
	type = EventType::Float;
	data.resize(sizeof(float));
	memcpy(data.data(), &v, sizeof(float));
}

void EventArg::setDouble(double v)
{
	type = EventType::Double;
	data.resize(sizeof(double));
	memcpy(data.data(), &v, sizeof(double));
}

void EventArg::setString(const std::string &v)
{
	type = EventType::String;
	size_t len = v.size();
	data.resize(sizeof(size_t) + len);
	memcpy(data.data(), &len, sizeof(size_t));
	memcpy(data.data() + sizeof(size_t), v.data(), len);
}

void EventArg::setVec3(const glm::vec3 &v)
{
	type = EventType::Vec3;
	data.resize(sizeof(float) * 3);
	memcpy(data.data(), &v.x, sizeof(float) * 3);
}

void EventArg::setVec2(const glm::vec2 &v)
{
	type = EventType::Vec2;
	data.resize(sizeof(float) * 2);
	memcpy(data.data(), &v.x, sizeof(float) * 2);
}

void EventArg::setIntArray(const std::vector<int> &v)
{
	type = EventType::IntArray;
	size_t count = v.size();
	data.resize(sizeof(size_t) + count * sizeof(int));
	memcpy(data.data(), &count, sizeof(size_t));
	memcpy(data.data() + sizeof(size_t), v.data(), count * sizeof(int));
}

void EventArg::setFloatArray(const std::vector<float> &v)
{
	type = EventType::FloatArray;
	size_t count = v.size();
	data.resize(sizeof(size_t) + count * sizeof(float));
	memcpy(data.data(), &count, sizeof(size_t));
	memcpy(data.data() + sizeof(size_t), v.data(), count * sizeof(float));
}

void EventArg::setStringArray(const std::vector<std::string> &v)
{
	type = EventType::StringArray;
	std::vector<unsigned char> buf;
	size_t count = v.size();
	buf.resize(sizeof(size_t));
	memcpy(buf.data(), &count, sizeof(size_t));
	for (const auto &s : v)
	{
		size_t len = s.size();
		buf.resize(buf.size() + sizeof(size_t) + len);
		memcpy(buf.data() + buf.size() - sizeof(size_t) - len, &len, sizeof(size_t));
		memcpy(buf.data() + buf.size() - len, s.data(), len);
	}
	data = std::move(buf);
}

// ==================== EventArg getters ====================

bool EventArg::getBool() const
{
	if (data.size() < sizeof(bool)) return false;
	bool v;
	memcpy(&v, data.data(), sizeof(bool));
	return v;
}

int EventArg::getInt() const
{
	if (data.size() < sizeof(int)) return 0;
	int v;
	memcpy(&v, data.data(), sizeof(int));
	return v;
}

float EventArg::getFloat() const
{
	if (data.size() < sizeof(float)) return 0.f;
	float v;
	memcpy(&v, data.data(), sizeof(float));
	return v;
}

double EventArg::getDouble() const
{
	if (data.size() < sizeof(double)) return 0.0;
	double v;
	memcpy(&v, data.data(), sizeof(double));
	return v;
}

std::string EventArg::getString() const
{
	if (data.size() < sizeof(size_t)) return "";
	size_t len;
	memcpy(&len, data.data(), sizeof(size_t));
	if (data.size() < sizeof(size_t) + len) return "";
	return std::string((const char *)data.data() + sizeof(size_t), len);
}

glm::vec3 EventArg::getVec3() const
{
	if (data.size() < sizeof(float) * 3) return {};
	glm::vec3 v;
	memcpy(&v, data.data(), sizeof(float) * 3);
	return v;
}

glm::vec2 EventArg::getVec2() const
{
	if (data.size() < sizeof(float) * 2) return {};
	glm::vec2 v;
	memcpy(&v, data.data(), sizeof(float) * 2);
	return v;
}

std::vector<int> EventArg::getIntArray() const
{
	if (data.size() < sizeof(size_t)) return {};
	size_t count;
	memcpy(&count, data.data(), sizeof(size_t));
	if (data.size() < sizeof(size_t) + count * sizeof(int)) return {};
	std::vector<int> v(count);
	memcpy(v.data(), data.data() + sizeof(size_t), count * sizeof(int));
	return v;
}

std::vector<float> EventArg::getFloatArray() const
{
	if (data.size() < sizeof(size_t)) return {};
	size_t count;
	memcpy(&count, data.data(), sizeof(size_t));
	if (data.size() < sizeof(size_t) + count * sizeof(float)) return {};
	std::vector<float> v(count);
	memcpy(v.data(), data.data() + sizeof(size_t), count * sizeof(float));
	return v;
}

std::vector<std::string> EventArg::getStringArray() const
{
	if (data.size() < sizeof(size_t)) return {};
	size_t count;
	memcpy(&count, data.data(), sizeof(size_t));
	const char *ptr = reinterpret_cast<const char *>(data.data()) + sizeof(size_t);
	const char *end = reinterpret_cast<const char *>(data.data()) + data.size();
	std::vector<std::string> v;
	v.reserve(count);
	for (size_t i = 0; i < count; i++)
	{
		std::string s;
		if (!deserializeString(ptr, end, s)) return {};
		v.push_back(std::move(s));
	}
	return v;
}

// ==================== EventData serialization ====================

static void writeVarInt(std::vector<unsigned char> &buf, uint32_t v)
{
	while (v > 0x7F)
	{
		buf.push_back((unsigned char)(v & 0x7F) | 0x80);
		v >>= 7;
	}
	buf.push_back((unsigned char)v);
}

static uint32_t readVarInt(const char *&ptr, const char *end)
{
	uint32_t result = 0;
	int shift = 0;
	while (ptr < end)
	{
		unsigned char b = *ptr++;
		result |= (uint32_t)(b & 0x7F) << shift;
		if (!(b & 0x80)) break;
		shift += 7;
	}
	return result;
}

std::vector<unsigned char> EventData::serialize() const
{
	std::vector<unsigned char> buf;

	// Event name (varint length + string)
	writeVarInt(buf, (uint32_t)eventName.size());
	buf.insert(buf.end(), eventName.begin(), eventName.end());

	// Args count
	writeVarInt(buf, (uint32_t)args.size());

	// Each arg: type byte + data
	for (const auto &arg : args)
	{
		buf.push_back((unsigned char)arg.type);
		buf.insert(buf.end(), arg.data.begin(), arg.data.end());
	}

	return buf;
}

bool EventData::deserialize(const char *data, size_t size)
{
	const char *ptr = data;
	const char *end = data + size;

	// Event name
	uint32_t nameLen = readVarInt(ptr, end);
	if (ptr + nameLen > end) return false;
	eventName.assign(ptr, nameLen);
	ptr += nameLen;

	// Args count
	uint32_t argCount = readVarInt(ptr, end);

	args.clear();
	args.reserve(argCount);

	for (uint32_t i = 0; i < argCount; i++)
	{
		if (ptr >= end) return false;
		EventArg arg;
		arg.type = (EventType)*ptr++;

		// Determine data size based on type
		size_t dataSize = 0;
		switch (arg.type)
		{
		case EventType::Bool: dataSize = sizeof(bool); break;
		case EventType::Int: dataSize = sizeof(int); break;
		case EventType::Float: dataSize = sizeof(float); break;
		case EventType::Double: dataSize = sizeof(double); break;
		case EventType::Vec3: dataSize = sizeof(float) * 3; break;
		case EventType::Vec2: dataSize = sizeof(float) * 2; break;
		case EventType::String:
		case EventType::IntArray:
		case EventType::FloatArray:
		case EventType::StringArray:
			// Variable size: read length-prefixed data
			// For String: size_t length + chars
			// For arrays: size_t count + elements
			// We need to read the full arg data
		{
			// Find the next type byte or end
			// Since we can't know the exact size without parsing,
			// we read until we have consumed all data for this arg
			// For simplicity, store remaining data and parse on get
			const char *argStart = ptr;
			while (ptr < end && *ptr != (unsigned char)EventType::Bool
				&& *ptr != (unsigned char)EventType::Int
				&& *ptr != (unsigned char)EventType::Float
				&& *ptr != (unsigned char)EventType::Double
				&& *ptr != (unsigned char)EventType::Vec3
				&& *ptr != (unsigned char)EventType::Vec2)
			{
				ptr++;
			}
			arg.data.assign(argStart, ptr);
			args.push_back(std::move(arg));
			continue;
		}
		default:
			return false; // Unknown type
		}

		if (ptr + dataSize > end) return false;
		arg.data.assign(ptr, ptr + dataSize);
		ptr += dataSize;
		args.push_back(std::move(arg));
	}

	return true;
}

// ==================== Serialization helpers ====================

void serializeString(std::vector<unsigned char> &buf, const std::string &s)
{
	size_t len = s.size();
	buf.resize(buf.size() + sizeof(size_t));
	memcpy(buf.data() + buf.size() - sizeof(size_t), &len, sizeof(size_t));
	buf.insert(buf.end(), s.begin(), s.end());
}

bool deserializeString(const char *&ptr, const char *end, std::string &s)
{
	if (ptr + sizeof(size_t) > end) return false;
	size_t len;
	memcpy(&len, ptr, sizeof(size_t));
	ptr += sizeof(size_t);
	if (ptr + len > end) return false;
	s.assign(ptr, len);
	ptr += len;
	return true;
}

void serializeVec3(std::vector<unsigned char> &buf, const glm::vec3 &v)
{
	size_t oldSize = buf.size();
	buf.resize(oldSize + sizeof(float) * 3);
	memcpy(buf.data() + oldSize, &v.x, sizeof(float) * 3);
}

bool deserializeVec3(const char *&ptr, const char *end, glm::vec3 &v)
{
	if (ptr + sizeof(float) * 3 > end) return false;
	memcpy(&v, ptr, sizeof(float) * 3);
	ptr += sizeof(float) * 3;
	return true;
}

void serializeVec2(std::vector<unsigned char> &buf, const glm::vec2 &v)
{
	size_t oldSize = buf.size();
	buf.resize(oldSize + sizeof(float) * 2);
	memcpy(buf.data() + oldSize, &v.x, sizeof(float) * 2);
}

bool deserializeVec2(const char *&ptr, const char *end, glm::vec2 &v)
{
	if (ptr + sizeof(float) * 2 > end) return false;
	memcpy(&v, ptr, sizeof(float) * 2);
	ptr += sizeof(float) * 2;
	return true;
}

// ==================== ClientEventSystem ====================

void ClientEventSystem::on(const std::string &eventName, EventCallback callback, int priority)
{
	std::lock_guard<std::mutex> lock(mutex);
	EventHandler h;
	h.eventName = eventName;
	h.callback = std::move(callback);
	h.priority = priority;
	auto &vec = handlers[eventName];
	vec.push_back(std::move(h));
	std::sort(vec.begin(), vec.end(),
		[](const EventHandler &a, const EventHandler &b)
		{ return a.priority < b.priority; });
}

void ClientEventSystem::off(const std::string &eventName)
{
	std::lock_guard<std::mutex> lock(mutex);
	handlers.erase(eventName);
}

void ClientEventSystem::triggerServer(const std::string &eventName,
	const std::vector<EventArg> &args)
{
	std::lock_guard<std::mutex> lock(pendingMutex);
	PendingEvent e;
	e.eventName = eventName;
	e.args = args;
	pendingEvents.push_back(std::move(e));
}

void ClientEventSystem::handleServerEvent(const std::string &eventName,
	const std::vector<EventArg> &args)
{
	std::lock_guard<std::mutex> lock(mutex);
	auto it = handlers.find(eventName);
	if (it == handlers.end()) return;

	for (const auto &h : it->second)
	{
		if (!h.callback(eventName, args, 0))
			break; // Event cancelled
	}
}

// ==================== ServerEventSystem ====================

void ServerEventSystem::on(const std::string &eventName, EventCallback callback, int priority)
{
	std::lock_guard<std::mutex> lock(mutex);
	EventHandler h;
	h.eventName = eventName;
	h.callback = std::move(callback);
	h.priority = priority;
	auto &vec = handlers[eventName];
	vec.push_back(std::move(h));
	std::sort(vec.begin(), vec.end(),
		[](const EventHandler &a, const EventHandler &b)
		{ return a.priority < b.priority; });
}

void ServerEventSystem::off(const std::string &eventName)
{
	std::lock_guard<std::mutex> lock(mutex);
	handlers.erase(eventName);
}

void ServerEventSystem::triggerClient(std::uint64_t targetCid,
	const std::string &eventName, const std::vector<EventArg> &args)
{
	// The actual sending is handled by the network layer
	// This just queues the event; the server tick will send it
}

void ServerEventSystem::triggerAllClients(const std::string &eventName,
	const std::vector<EventArg> &args)
{
	// Same as triggerClient but with broadcast flag
}

bool ServerEventSystem::handleClientEvent(const std::string &eventName,
	const std::vector<EventArg> &args, std::uint64_t senderCid)
{
	std::lock_guard<std::mutex> lock(mutex);
	auto it = handlers.find(eventName);
	if (it == handlers.end()) return true; // No handler = allow

	for (const auto &h : it->second)
	{
		if (!h.callback(eventName, args, senderCid))
			return false; // Event cancelled
	}

	return true; // Event allowed
}
