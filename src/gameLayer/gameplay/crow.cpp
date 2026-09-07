
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <gameplay/crow.h>
#include <rendering/model.h>
#include <unordered_set>
#include <unordered_map>
#include <fstream>
#include <multyPlayer/serverChunkStorer.h>
#include <multyPlayer/server.h>
void Crow::update(float dt, decltype(chunkGetterSignature) *cg){ updateForces(dt,true); resolveConstrainsAndUpdatePositions(cg,dt,getColliderSize()); }
glm::vec3 Crow::getColliderSize(){ return getMaxColliderSize(); }
glm::vec3 Crow::getMaxColliderSize(){ return glm::vec3(0.5,0.5,0.5); }
void CrowClient::update(float dt, decltype(chunkGetterSignature) *cg){ entityBuffered.update(dt,cg); }
void CrowClient::setEntityMatrix(glm::mat4 *m){ }
int CrowClient::getTextureIndex(){ return ModelsManager::CrowTexture; }
bool CrowServer::update(float dt, decltype(chunkGetterSignature) *cg, ServerChunkStorer &s, std::minstd_rand &rng, std::uint64_t eid, std::unordered_set<std::uint64_t> &del, std::unordered_map<std::uint64_t, std::unordered_map<glm::ivec3, PathFindingNode>> &pf, std::unordered_map<std::uint64_t, glm::dvec3> &pp, std::unordered_map<std::uint64_t, Client*> &ac){ updateAnimalBehaviour(dt,cg,s,rng); doCollisionWithOthers(getPosition(), entity.getMaxColliderSize(), entity.forces, s, eid); entity.update(dt,cg); return true; }
void CrowServer::appendDataToDisk(std::ofstream &f, std::uint64_t id){ f.write((char*)&entity,sizeof(entity)); }
