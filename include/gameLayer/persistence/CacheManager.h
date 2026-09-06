#pragma once
#include <filesystem>
#include <string>
#include <vector>

class CacheManager
{
public:
	explicit CacheManager(std::filesystem::path root);
	void init();
	void clear();
	bool isCacheValid() const;
	void invalidate();
	void rebuild();
	std::filesystem::path pathFor(const std::string &key) const;
	bool put(const std::string &key, const std::vector<char> &data);
	bool get(const std::string &key, std::vector<char> &out) const;
private:
	std::filesystem::path m_root;
};
