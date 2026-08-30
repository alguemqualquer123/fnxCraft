#pragma once
#include <gameplay/entity.h>
#include <gameplay/life.h>
#include <random>
#include <unordered_map>
#include <unordered_set>

struct Fish: public PhysicalEntity, public HasOrientationAndHeadTurnDirection,
	public MovementSpeedForLegsAnimations, public CollidesWithPlacedBlocks,
	public CanBeKilled, public CanBeAttacked
{
	void update(float deltaTime, decltype(chunkGetterSignature) *chunkGetter);

	glm::vec3 getColliderSize();
	static glm::vec3 getMaxColliderSize();

	// Fish specific properties
	float swimTimer = 0;
	float swimDirectionChangeTimer = 0;
	glm::vec2 swimDirection = {0, -1};
	bool isNearSurface = false;

	// Fish types
	enum FishType : unsigned char
	{
		BASIC_FISH = 0,
		TROPICAL_FISH,
		PUFFERFISH,
		COD,
		SALMON
	};

	FishType fishType = BASIC_FISH;
	
	Life life{30};  // Fish have less health

	Armour getArmour() { return {0}; }
};

struct FishClient: public ClientEntity<Fish, FishClient>
{
	void update(float deltaTime, decltype(chunkGetterSignature) *chunkGetter);
	void setEntityMatrix(glm::mat4 *skinningMatrix);

	int getTextureIndex();
};

struct FishServer: public ServerEntity<Fish>
{
	float swimTimer = 0;
	float directionChangeTimer = 0;
	float surfaceCheckTimer = 0;

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
