#include <gameLayer/persistence/CacheManager.h>
#include <safeSave.h>

CacheManager::CacheManager(std::filesystem::path root) : m_root(std::move(root)) {}

void CacheManager::init()
{
	std::error_code ec;
	std::filesystem::create_directories(m_root, ec);
}

void CacheManager::clear()
{
	std::error_code ec;
	std::filesystem::remove_all(m_root, ec);
	std::filesystem::create_directories(m_root, ec);
}

bool CacheManager::isCacheValid() const { return std::filesystem::exists(m_root); }

void CacheManager::invalidate() { clear(); }

void CacheManager::rebuild() { init(); }

std::filesystem::path CacheManager::pathFor(const std::string &key) const
{
	std::string safe = key;
	for (auto &c : safe) if (c == '/' || c == '\\') c = '_';
	return m_root / safe;
}

bool CacheManager::put(const std::string &key, const std::vector<char> &data)
{
	auto p = pathFor(key);
	std::error_code ec;
	std::filesystem::create_directories(p.parent_path(), ec);
	return sfs::writeEntireFile(data.data(), data.size(), p.string().c_str()) == sfs::noError;
}

bool CacheManager::get(const std::string &key, std::vector<char> &out) const
{
	auto p = pathFor(key);
	if (!std::filesystem::exists(p)) return false;
	return sfs::readEntireFile(out, p.string().c_str()) == sfs::noError;
}
