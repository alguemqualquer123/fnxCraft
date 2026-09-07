#include "SkillManager.h"
#include <algorithm>

// Example skills
#include "core/Model.h"
#include "core/Diagnostics.h"

class InspectModelSkill : public Skill {
public:
    SkillMetadata getMetadata() const override {
        return {"model.inspect", "Inspect Model", "1.0.0", "inspect", {"scene.read"}, "Inspeciona modelo"};
    }
    SkillResult execute(SkillContext& ctx) override {
        return {true, "Modelo inspecionado", ""};
    }
};
class PerformanceAnalyzerSkill : public Skill {
public:
    SkillMetadata getMetadata() const override {
        return {"model.performance.analyzer", "Performance Analyzer", "1.0.0", "diagnostics", {"scene.read","renderer.read"}, "Analisa performance"};
    }
    SkillResult execute(SkillContext& ctx) override {
        return {true, "Performance analisada", ""};
    }
};
class OptimizeModelSkill : public Skill {
public:
    SkillMetadata getMetadata() const override {
        return {"model.optimize", "Optimize Model", "1.0.0", "optimization", {"scene.read","scene.write"}, "Otimiza modelo"};
    }
    SkillResult execute(SkillContext& ctx) override {
        return {true, "Modelo otimizado (preview)", ""};
    }
};

void SkillManager::registerSkill(std::unique_ptr<Skill> s) {
    auto id = s->getMetadata().id;
    skills[id] = std::move(s);
}
std::vector<SkillMetadata> SkillManager::listSkills() const {
    std::vector<SkillMetadata> r;
    for(auto &kv: skills) r.push_back(kv.second->getMetadata());
    return r;
}
Skill* SkillManager::find(const std::string& id) {
    auto it = skills.find(id);
    return it==skills.end()?nullptr:it->second.get();
}
SkillResult SkillManager::execute(const std::string& id, SkillContext& ctx) {
    auto* s = find(id);
    if(!s) return {false, "Skill não encontrada: "+id};
    if(!s->canExecute(ctx)) return {false, "Sem permissão"};
    return s->execute(ctx);
}
bool SkillManager::hasPermission(const std::string& id, const std::string& perm) const {
    auto it = skills.find(id);
    if(it==skills.end()) return false;
    auto perms = it->second->getMetadata().permissions;
    return std::find(perms.begin(), perms.end(), perm) != perms.end();
}
void registerBuiltinSkills(SkillManager& sm) {
    sm.registerSkill(std::make_unique<InspectModelSkill>());
    sm.registerSkill(std::make_unique<PerformanceAnalyzerSkill>());
    sm.registerSkill(std::make_unique<OptimizeModelSkill>());
    // Add more skills here without modifying core
    struct ValidateSkill : Skill {
        SkillMetadata getMetadata() const override { return {"validate.gltf","Validate GLTF","1.0.0","diagnostics",{"scene.read"},"Valida GLTF"}; }
        SkillResult execute(SkillContext& ctx) override { return {true,"GLTF válido",""}; }
    };
    sm.registerSkill(std::make_unique<ValidateSkill>());
}
