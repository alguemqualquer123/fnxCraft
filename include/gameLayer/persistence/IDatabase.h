#pragma once
#include <string>
#include <vector>
#include <functional>

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

template<typename T>
class IRepository
{
public:
	virtual ~IRepository() = default;
	virtual bool save(const T &entity) = 0;
	virtual bool load(const std::string &id, T &out) = 0;
	virtual bool remove(const std::string &id) = 0;
};
