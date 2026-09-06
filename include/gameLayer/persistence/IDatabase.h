#pragma once
#include <string>
#include <vector>
#include <functional>
#include <unordered_map>
#include <filesystem>
#include <fstream>
#include <mutex>

class IDatabase
{
public:
	virtual ~IDatabase() = default;
	virtual bool connect(const std::string &connectionString) = 0;
	virtual bool disconnect() = 0;
	virtual bool execute(const std::string &sql) = 0;
	virtual bool beginTransaction() = 0;
	virtual bool commit() = 0;
	virtual bool rollback() = 0;
	virtual bool isConnected() const = 0;
};

class NullDatabase : public IDatabase
{
public:
	bool connect(const std::string &) override { return true; }
	bool disconnect() override { return true; }
	bool execute(const std::string &) override { return true; }
	bool beginTransaction() override { return true; }
	bool commit() override { return true; }
	bool rollback() override { return true; }
	bool isConnected() const override { return true; }
};

class JsonDatabase : public IDatabase
{
public:
	explicit JsonDatabase(std::filesystem::path root = {});
	bool connect(const std::string &connectionString) override;
	bool disconnect() override;
	bool execute(const std::string &sql) override;
	bool beginTransaction() override;
	bool commit() override;
	bool rollback() override;
	bool isConnected() const override { return m_connected; }
	bool put(const std::string &table, const std::string &key, const std::string &json);
	bool get(const std::string &table, const std::string &key, std::string &out) const;
	bool removeKey(const std::string &table, const std::string &key);
	std::vector<std::string> listKeys(const std::string &table) const;
private:
	bool loadTable(const std::string &table) const;
	bool saveTable(const std::string &table) const;
	std::filesystem::path tableFile(const std::string &table) const;
	std::filesystem::path m_root;
	bool m_connected = false;
	bool m_inTransaction = false;
	mutable std::mutex m_mutex;
	mutable std::unordered_map<std::string, std::unordered_map<std::string, std::string>> m_cache;
	std::unordered_map<std::string, std::unordered_map<std::string, std::string>> m_backup;
};

template<typename T>
class IRepository
{
public:
	virtual ~IRepository() = default;
	virtual bool save(const T &entity) = 0;
	virtual bool load(const std::string &id, T &out) = 0;
	virtual bool remove(const std::string &id) = 0;
	virtual std::vector<std::string> listIds() { return {}; }
};

template<typename T>
class JsonRepository : public IRepository<T>
{
public:
	using ToJsonFn = std::function<std::string(const T&)>;
	using FromJsonFn = std::function<bool(const std::string&, T&)>;
	using IdFn = std::function<std::string(const T&)>;

	JsonRepository(JsonDatabase *db, std::string table, IdFn idFn, ToJsonFn toFn, FromJsonFn fromFn)
		: m_db(db), m_table(std::move(table)), m_idFn(std::move(idFn)), m_toFn(std::move(toFn)), m_fromFn(std::move(fromFn)) {}

	bool save(const T &entity) override
	{
		if (!m_db || !m_idFn || !m_toFn) return false;
		return m_db->put(m_table, m_idFn(entity), m_toFn(entity));
	}
	bool load(const std::string &id, T &out) override
	{
		if (!m_db || !m_fromFn) return false;
		std::string json;
		if (!m_db->get(m_table, id, json)) return false;
		return m_fromFn(json, out);
	}
	bool remove(const std::string &id) override
	{
		if (!m_db) return false;
		return m_db->removeKey(m_table, id);
	}
	std::vector<std::string> listIds() override
	{
		if (!m_db) return {};
		return m_db->listKeys(m_table);
	}
private:
	JsonDatabase *m_db = nullptr;
	std::string m_table;
	IdFn m_idFn;
	ToJsonFn m_toFn;
	FromJsonFn m_fromFn;
};
