#pragma once
#include <gameplay/entity.h>
#include <gameplay/life.h>
struct Crow: public PhysicalEntity, public HasOrientationAndHeadTurnDirection,
	public MovementSpeedForLegsAnimations, public CollidesWithPlacedBlocks,
	public CanBeKilled, public CanBeAttacked
{
	void update(float deltaTime, decltype(chunkGetterSignature) *chunkGetter);
	glm::vec3 getColliderSize();
	static glm::vec3 getMaxColliderSize();
	Life life{30};
	Armour getArmour() { return {0}; }
};
struct CrowClient: public ClientEntity<Crow, CrowClient>
{
	void update(float deltaTime, decltype(chunkGetterSignature) *chunkGetter);
	void setEntityMatrix(glm::mat4 *skinningMatrix);
	int getTextureIndex();
};
struct CrowServer: public ServerEntity<Crow>
{
	bool update(float deltaTime, decltype(chunkGetterSignature) *chunkGetter,
		ServerChunkStorer &serverChunkStorer, std::minstd_rand &rng, std::uint64_t yourEID,
		std::unordered_set<std::uint64_t> &othersDeleted,
		std::unordered_map<std::uint64_t, std::unordered_map<glm::ivec3, PathFindingNode>> &pathFinding,
		std::unordered_map<std::uint64_t, glm::dvec3> &playersPosition,
		std::unordered_map < std::uint64_t, Client *> &allClients
		);
	void appendDataToDisk(std::ofstream &f, std::uint64_t eId);
	bool isUnaware() { return false; }
	void signalHit(glm::vec3 direction) {};
	LootTable &getLootTable() { return getEmptyLootTable(); }
};
