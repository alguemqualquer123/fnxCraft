
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <gameplay/sheep.h>
#include <rendering/model.h>
#include <unordered_set>
#include <unordered_map>
#include <fstream>
#include <multyPlayer/serverChunkStorer.h>
#include <multyPlayer/server.h>
void Sheep::update(float dt, decltype(chunkGetterSignature) *cg){ updateForces(dt,true); resolveConstrainsAndUpdatePositions(cg,dt,getColliderSize()); }
glm::vec3 Sheep::getColliderSize(){ return getMaxColliderSize(); }
glm::vec3 Sheep::getMaxColliderSize(){ return glm::vec3(0.9,1.1,0.9); }
void SheepClient::update(float dt, decltype(chunkGetterSignature) *cg){ entityBuffered.update(dt,cg); }
void SheepClient::setEntityMatrix(glm::mat4 *m){ }
int SheepClient::getTextureIndex(){ return ModelsManager::SheepTexture; }
bool SheepServer::update(float dt, decltype(chunkGetterSignature) *cg, ServerChunkStorer &s, std::minstd_rand &rng, std::uint64_t eid, std::unordered_set<std::uint64_t> &del, std::unordered_map<std::uint64_t, std::unordered_map<glm::ivec3, PathFindingNode>> &pf, std::unordered_map<std::uint64_t, glm::dvec3> &pp, std::unordered_map<std::uint64_t, Client*> &ac){ entity.update(dt,cg); return false; }
void SheepServer::appendDataToDisk(std::ofstream &f, std::uint64_t id){ f.write((char*)&entity,sizeof(entity)); }
