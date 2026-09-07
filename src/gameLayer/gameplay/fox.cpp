
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <gameplay/fox.h>
#include <rendering/model.h>
#include <unordered_set>
#include <unordered_map>
#include <fstream>
#include <multyPlayer/serverChunkStorer.h>
#include <multyPlayer/server.h>
void Fox::update(float dt, decltype(chunkGetterSignature) *cg){ updateForces(dt,true); resolveConstrainsAndUpdatePositions(cg,dt,getColliderSize()); }
glm::vec3 Fox::getColliderSize(){ return getMaxColliderSize(); }
glm::vec3 Fox::getMaxColliderSize(){ return glm::vec3(0.6,0.7,0.6); }
void FoxClient::update(float dt, decltype(chunkGetterSignature) *cg){ entityBuffered.update(dt,cg); }
void FoxClient::setEntityMatrix(glm::mat4 *m){ }
int FoxClient::getTextureIndex(){ return ModelsManager::FoxTexture; }
bool FoxServer::update(float dt, decltype(chunkGetterSignature) *cg, ServerChunkStorer &s, std::minstd_rand &rng, std::uint64_t eid, std::unordered_set<std::uint64_t> &del, std::unordered_map<std::uint64_t, std::unordered_map<glm::ivec3, PathFindingNode>> &pf, std::unordered_map<std::uint64_t, glm::dvec3> &pp, std::unordered_map<std::uint64_t, Client*> &ac){ updateAnimalBehaviour(dt,cg,s,rng); doCollisionWithOthers(getPosition(), entity.getMaxColliderSize(), entity.forces, s, eid); entity.update(dt,cg); return true; }
void FoxServer::appendDataToDisk(std::ofstream &f, std::uint64_t id){ f.write((char*)&entity,sizeof(entity)); }
