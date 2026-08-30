#include <gameLayer/persistence/PlayerStorage.h>
#include <safeSave.h>
#include <filesystem>

PlayerStorage::PlayerStorage(std::filesystem::path root) : m_root(std::move(root))
{
	std::error_code ec;
	std::filesystem::create_directories(m_root, ec);
}

std::optional<PlayerData> PlayerStorage::load(const PlayerId &id)
{
	auto path = fileFor(id);
	if (!std::filesystem::exists(path)) return std::nullopt;
	std::vector<char> data;
	if (sfs::readEntireFile(data, path.string().c_str()) != sfs::noError) return std::nullopt;
	PlayerData pd;
	pd.id = id;
	pd.blob = std::move(data);
	return pd;
}

bool PlayerStorage::save(const PlayerData &data)
{
	std::lock_guard<std::mutex> l(m_mutex);
	auto path = fileFor(data.id);
	std::error_code ec;
	std::filesystem::create_directories(path.parent_path(), ec);
	std::string tmp = path.string() + ".tmp";
	if (sfs::writeEntireFile(data.blob.data(), data.blob.size(), tmp.c_str()) != sfs::noError) return false;
	std::filesystem::rename(tmp, path, ec);
	return !ec;
}

bool PlayerStorage::remove(const PlayerId &id)
{
	std::error_code ec;
	std::filesystem::remove(fileFor(id), ec);
	return !ec;
}

bool PlayerStorage::exists(const PlayerId &id) const
{
	return std::filesystem::exists(fileFor(id));
}

std::vector<PlayerId> PlayerStorage::listAll() const
{
	std::vector<PlayerId> out;
	std::error_code ec;
	for (auto &e : std::filesystem::directory_iterator(m_root, ec))
	{
		if (e.path().extension() == ".dat")
			out.push_back(e.path().stem().string());
	}
	return out;
}

std::filesystem::path PlayerStorage::fileFor(const PlayerId &id) const
{
	std::string safe = id;
	for (auto &c : safe) if (c == '/' || c == '\\' || c == '.') c = '_';
	return m_root / (safe + ".dat");
}
