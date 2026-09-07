#pragma once
#include <string>
#include <vector>
#include <functional>
#include <unordered_map>
#include <future>

struct SkillContext {
    void* model = nullptr;
    void* renderer = nullptr;
    void* scene = nullptr;
    std::unordered_map<std::string, std::string> params;
};

struct SkillResult {
    bool success = false;
    std::string message;
    std::string data;
};

struct SkillMetadata {
    std::string id;
    std::string name;
    std::string version = "1.0.0";
    std::string category;
    std::vector<std::string> permissions;
    std::string description;
};

class Skill {
public:
    virtual ~Skill() = default;
    virtual SkillMetadata getMetadata() const = 0;
    virtual bool canExecute(const SkillContext& ctx) const { return true; }
    virtual SkillResult execute(SkillContext& ctx) = 0;
};

class SkillManager {
public:
    void registerSkill(std::unique_ptr<Skill> skill);
    std::vector<SkillMetadata> listSkills() const;
    Skill* find(const std::string& id);
    SkillResult execute(const std::string& id, SkillContext& ctx);
    bool hasPermission(const std::string& skillId, const std::string& perm) const;
private:
    std::unordered_map<std::string, std::unique_ptr<Skill>> skills;
};

// Built-in skills
void registerBuiltinSkills(SkillManager& sm);
