#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <functional>
#include <unordered_map>
#include <mutex>
#include <cstring>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>

// Event argument types for serialization
enum class EventType : uint8_t
{
	None = 0,
	Bool,
	Int,
	Float,
	Double,
	String,
	Vec3,
	Vec2,
	IntArray,
	FloatArray,
	StringArray,
};

// Serialized event argument
struct EventArg
{
	EventType type = EventType::None;
	std::vector<unsigned char> data;

	void setBool(bool v);
	void setInt(int v);
	void setFloat(float v);
	void setDouble(double v);
	void setString(const std::string &v);
	void setVec3(const glm::vec3 &v);
	void setVec2(const glm::vec2 &v);
	void setIntArray(const std::vector<int> &v);
	void setFloatArray(const std::vector<float> &v);
	void setStringArray(const std::vector<std::string> &v);

	bool getBool() const;
	int getInt() const;
	float getFloat() const;
	double getDouble() const;
	std::string getString() const;
	glm::vec3 getVec3() const;
	glm::vec2 getVec2() const;
	std::vector<int> getIntArray() const;
	std::vector<float> getFloatArray() const;
	std::vector<std::string> getStringArray() const;
};

// Serialized event with name + args
struct EventData
{
	std::string eventName;
	std::vector<EventArg> args;

	// Serialize to binary buffer
	std::vector<unsigned char> serialize() const;

	// Deserialize from binary buffer
	bool deserialize(const char *data, size_t size);
};

// Callback type for event handlers
// Return true to allow the event to continue, false to cancel
using EventCallback = std::function<bool(const std::string &eventName,
	const std::vector<EventArg> &args, std::uint64_t senderCid)>;

// Event handler registration
struct EventHandler
{
	std::string eventName;
	EventCallback callback;
	int priority = 0; // lower = called first
};

// Client-side event dispatcher
class ClientEventSystem
{
public:
	// Register a handler for a server event
	void on(const std::string &eventName, EventCallback callback, int priority = 0);

	// Remove all handlers for an event
	void off(const std::string &eventName);

	// Trigger a server event (client -> server)
	void triggerServer(const std::string &eventName,
		const std::vector<EventArg> &args = {});

	// Internal: called when receiving a server event
	void handleServerEvent(const std::string &eventName,
		const std::vector<EventArg> &args);

private:
	std::unordered_map<std::string, std::vector<EventHandler>> handlers;
	mutable std::mutex mutex;

	// Pending events to send to server
	struct PendingEvent
	{
		std::string eventName;
		std::vector<EventArg> args;
	};
	std::vector<PendingEvent> pendingEvents;
	mutable std::mutex pendingMutex;
};

// Server-side event dispatcher
class ServerEventSystem
{
public:
	// Register a handler for a client event
	void on(const std::string &eventName, EventCallback callback, int priority = 0);

	// Remove all handlers for an event
	void off(const std::string &eventName);

	// Trigger a client event (server -> one client)
	void triggerClient(std::uint64_t targetCid, const std::string &eventName,
		const std::vector<EventArg> &args = {});

	// Trigger a client event (server -> all clients)
	void triggerAllClients(const std::string &eventName,
		const std::vector<EventArg> &args = {});

	// Internal: called when receiving a client event
	bool handleClientEvent(const std::string &eventName,
		const std::vector<EventArg> &args, std::uint64_t senderCid);

private:
	std::unordered_map<std::string, std::vector<EventHandler>> handlers;
	mutable std::mutex mutex;
};

// Serialization helpers
void serializeString(std::vector<unsigned char> &buf, const std::string &s);
bool deserializeString(const char *&ptr, const char *end, std::string &s);
void serializeVec3(std::vector<unsigned char> &buf, const glm::vec3 &v);
bool deserializeVec3(const char *&ptr, const char *end, glm::vec3 &v);
void serializeVec2(std::vector<unsigned char> &buf, const glm::vec2 &v);
bool deserializeVec2(const char *&ptr, const char *end, glm::vec2 &v);
