#pragma once
#include <string>
#include <vector>
#include <filesystem>
#include <cstdint>

struct ChunkId { int x = 0; int z = 0; bool operator==(const ChunkId &o) const { return x==o.x && z==o.z; } };

class IWorldStorage
{
public:
	virtual ~IWorldStorage() = default;
	virtual bool loadWorld(const std::string &worldName) = 0;
	virtual bool saveWorld(const std::string &worldName) = 0;
	virtual bool loadChunk(const ChunkId &id, std::vector<char> &outData) = 0;
	virtual bool saveChunk(const ChunkId &id, const std::vector<char> &data) = 0;
	virtual bool unloadChunk(const ChunkId &id) = 0;
	virtual bool flush() = 0;
	virtual bool isDirty(const ChunkId &id) const = 0;
	virtual void markDirty(const ChunkId &id) = 0;
	virtual void markClean(const ChunkId &id) = 0;
};
