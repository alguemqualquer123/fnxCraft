#include <gameLayer/persistence/FileWorldStorage.h>
#include <safeSave.h>
#include <iostream>

FileWorldStorage::FileWorldStorage(std::filesystem::path worldRoot) : m_worldRoot(std::move(worldRoot)) {}

bool FileWorldStorage::loadWorld(const std::string &worldName)
{
	std::error_code ec;
	std::filesystem::create_directories(m_worldRoot / worldName, ec);
	return std::filesystem::exists(m_worldRoot / worldName);
}

bool FileWorldStorage::saveWorld(const std::string &worldName)
{
	return flush();
}

bool FileWorldStorage::loadChunk(const ChunkId &id, std::vector<char> &outData)
{
	auto path = chunkPath(id);
	if (!std::filesystem::exists(path)) return false;
	return sfs::readEntireFile(outData, path.string().c_str()) == sfs::noError;
}

bool FileWorldStorage::saveChunk(const ChunkId &id, const std::vector<char> &data)
{
	if (!isDirty(id)) return true;
	auto path = chunkPath(id);
	std::error_code ec;
	std::filesystem::create_directories(path.parent_path(), ec);
	std::string tmp = path.string() + ".tmp";
	if (sfs::writeEntireFile(data.data(), data.size(), tmp.c_str()) != sfs::noError) return false;
	std::error_code ec2;
	std::filesystem::rename(tmp, path, ec2);
	if (ec2) return false;
	markClean(id);
	return true;
}

bool FileWorldStorage::unloadChunk(const ChunkId &id) { return true; }

bool FileWorldStorage::flush()
{
	return true;
}

bool FileWorldStorage::isDirty(const ChunkId &id) const
{
	std::lock_guard<std::mutex> l(m_mutex);
	return m_dirty.find(key(id)) != m_dirty.end();
}

void FileWorldStorage::markDirty(const ChunkId &id)
{
	std::lock_guard<std::mutex> l(m_mutex);
	m_dirty.insert(key(id));
}

void FileWorldStorage::markClean(const ChunkId &id)
{
	std::lock_guard<std::mutex> l(m_mutex);
	m_dirty.erase(key(id));
}

std::filesystem::path FileWorldStorage::chunkPath(const ChunkId &id) const
{
	return m_worldRoot / ("chunk_" + std::to_string(id.x) + "_" + std::to_string(id.z) + ".bin");
}
