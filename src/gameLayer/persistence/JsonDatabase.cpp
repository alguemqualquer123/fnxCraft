#include <gameLayer/persistence/IDatabase.h>
#include <safeSave/include/safeSave.h>
#include <filesystem>
#include <fstream>
#include <sstream>

JsonDatabase::JsonDatabase(std::filesystem::path root) : m_root(std::move(root)) {}

static std::string escapeJson(const std::string &s)
{
	std::string o;
	o.reserve(s.size() + 4);
	for (char c : s)
	{
		if (c == '"') o += "\\\"";
		else if (c == '\\') o += "\\\\";
		else if (c == '\n') o += "\\n";
		else if (c == '\r') o += "\\r";
		else if (c == '\t') o += "\\t";
		else o += c;
	}
	return o;
}

static std::string unescapeJson(const std::string &s)
{
	std::string o;
	o.reserve(s.size());
	for (size_t i = 0; i < s.size(); ++i)
	{
		if (s[i] == '\\' && i + 1 < s.size())
		{
			char n = s[i + 1];
			if (n == '"') o += '"';
			else if (n == '\\') o += '\\';
			else if (n == 'n') o += '\n';
			else if (n == 'r') o += '\r';
			else if (n == 't') o += '\t';
			else o += n;
			++i;
		}
		else o += s[i];
	}
	return o;
}

std::filesystem::path JsonDatabase::tableFile(const std::string &table) const
{
	std::string safe = table;
	for (char &c : safe) if (c == '/' || c == '\\' || c == ':') c = '_';
	return m_root / (safe + ".json");
}

bool JsonDatabase::loadTable(const std::string &table) const
{
	if (m_cache.find(table) != m_cache.end()) return true;
	std::unordered_map<std::string, std::string> kv;
	auto path = tableFile(table);
	if (!std::filesystem::exists(path)) { m_cache[table] = std::move(kv); return true; }
	std::vector<char> data;
	if (sfs::readEntireFile(data, path.string().c_str()) != sfs::noError) return false;
	data.push_back(0);
	std::string content(data.data());
	size_t pos = 0;
	auto trim = [](const std::string &str, size_t &p) { while (p < str.size() && isspace((unsigned char)str[p])) ++p; };
	trim(content, pos);
	if (pos >= content.size() || content[pos] != '{') { m_cache[table] = std::move(kv); return true; }
	++pos;
	while (true)
	{
		trim(content, pos);
		if (pos >= content.size()) break;
		if (content[pos] == '}') { ++pos; break; }
		if (content[pos] != '"') break;
		++pos;
		std::string key;
		while (pos < content.size() && content[pos] != '"')
		{
			if (content[pos] == '\\' && pos + 1 < content.size()) { key += content[pos + 1]; pos += 2; }
			else key += content[pos++];
		}
		if (pos < content.size() && content[pos] == '"') ++pos;
		trim(content, pos);
		if (pos < content.size() && content[pos] == ':') ++pos;
		trim(content, pos);
		std::string value;
		if (pos < content.size() && content[pos] == '"')
		{
			++pos;
			while (pos < content.size() && content[pos] != '"')
			{
				if (content[pos] == '\\' && pos + 1 < content.size()) { value += content[pos]; value += content[pos + 1]; pos += 2; }
				else value += content[pos++];
			}
			if (pos < content.size() && content[pos] == '"') ++pos;
			value = unescapeJson(value);
		}
		else
		{
			size_t start = pos;
			while (pos < content.size() && content[pos] != ',' && content[pos] != '}') ++pos;
			value = content.substr(start, pos - start);
			size_t a = 0; while (a < value.size() && isspace((unsigned char)value[a])) ++a;
			size_t b = value.size(); while (b > a && isspace((unsigned char)value[b - 1])) --b;
			value = value.substr(a, b - a);
		}
		kv[std::move(key)] = std::move(value);
		trim(content, pos);
		if (pos < content.size() && content[pos] == ',') ++pos;
		else if (pos < content.size() && content[pos] == '}') { ++pos; break; }
	}
	m_cache[table] = std::move(kv);
	return true;
}

bool JsonDatabase::saveTable(const std::string &table) const
{
	auto it = m_cache.find(table);
	if (it == m_cache.end()) return true;
	std::filesystem::create_directories(m_root);
	std::ostringstream oss;
	oss << "{\n";
	bool first = true;
	for (auto &kv : it->second)
	{
		if (!first) oss << ",\n";
		first = false;
		oss << "  \"" << escapeJson(kv.first) << "\": \"" << escapeJson(kv.second) << "\"";
	}
	if (!it->second.empty()) oss << "\n";
	oss << "}\n";
	std::string str = oss.str();
	std::vector<char> data(str.begin(), str.end());
	return sfs::writeEntireFile(data, tableFile(table).string().c_str()) == sfs::noError;
}

bool JsonDatabase::connect(const std::string &connectionString)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	m_root = connectionString.empty() ? m_root : std::filesystem::path(connectionString);
	if (m_root.empty()) m_root = "data/databases";
	std::filesystem::create_directories(m_root);
	m_connected = true;
	return true;
}

bool JsonDatabase::disconnect()
{
	std::lock_guard<std::mutex> lock(m_mutex);
	if (m_inTransaction) commit();
	m_connected = false;
	return true;
}

bool JsonDatabase::execute(const std::string &sql)
{
	if (sql.empty()) return true;
	std::lock_guard<std::mutex> lock(m_mutex);
	if (sql == "FLUSH" || sql == "SYNC")
	{
		for (auto &kv : m_cache) saveTable(kv.first);
		return true;
	}
	return true;
}

bool JsonDatabase::beginTransaction()
{
	std::lock_guard<std::mutex> lock(m_mutex);
	if (m_inTransaction) return false;
	m_backup = m_cache;
	m_inTransaction = true;
	return true;
}

bool JsonDatabase::commit()
{
	std::lock_guard<std::mutex> lock(m_mutex);
	if (!m_inTransaction) return true;
	for (auto &kv : m_cache) saveTable(kv.first);
	m_backup.clear();
	m_inTransaction = false;
	return true;
}

bool JsonDatabase::rollback()
{
	std::lock_guard<std::mutex> lock(m_mutex);
	if (!m_inTransaction) return false;
	m_cache = m_backup;
	m_backup.clear();
	m_inTransaction = false;
	return true;
}

bool JsonDatabase::put(const std::string &table, const std::string &key, const std::string &json)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	loadTable(table);
	m_cache[table][key] = json;
	if (!m_inTransaction) saveTable(table);
	return true;
}

bool JsonDatabase::get(const std::string &table, const std::string &key, std::string &out) const
{
	std::lock_guard<std::mutex> lock(m_mutex);
	loadTable(table);
	auto it = m_cache.find(table);
	if (it == m_cache.end()) return false;
	auto kit = it->second.find(key);
	if (kit == it->second.end()) return false;
	out = kit->second;
	return true;
}

bool JsonDatabase::removeKey(const std::string &table, const std::string &key)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	loadTable(table);
	auto it = m_cache.find(table);
	if (it == m_cache.end()) return false;
	if (it->second.erase(key) == 0) return false;
	if (!m_inTransaction) saveTable(table);
	return true;
}

std::vector<std::string> JsonDatabase::listKeys(const std::string &table) const
{
	std::lock_guard<std::mutex> lock(m_mutex);
	loadTable(table);
	std::vector<std::string> keys;
	auto it = m_cache.find(table);
	if (it == m_cache.end()) return keys;
	for (auto &kv : it->second) keys.push_back(kv.first);
	return keys;
}
