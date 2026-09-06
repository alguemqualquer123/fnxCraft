#pragma once
#include "IWorldStorage.h"
#include <unordered_set>
#include <unordered_map>
#include <mutex>

class FileWorldStorage : public IWorldStorage
{
public:
	explicit FileWorldStorage(std::filesystem::path worldRoot);
	bool loadWorld(const std::string &worldName) override;
	bool saveWorld(const std::string &worldName) override;
	bool loadChunk(const ChunkId &id, std::vector<char> &outData) override;
	bool saveChunk(const ChunkId &id, const std::vector<char> &data) override;
	bool unloadChunk(const ChunkId &id) override;
	bool flush() override;
	bool isDirty(const ChunkId &id) const override;
	void markDirty(const ChunkId &id) override;
	void markClean(const ChunkId &id) override;

private:
	std::filesystem::path chunkPath(const ChunkId &id) const;
	std::filesystem::path m_worldRoot;
	mutable std::mutex m_mutex;
	std::unordered_set<int64_t> m_dirty;
	static int64_t key(const ChunkId &id) { return (int64_t(id.x) << 32) | uint32_t(id.z); }
};
