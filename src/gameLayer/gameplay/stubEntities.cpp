//Stub entity implementations.
//
//These entities are declared in headers (shared across server tick and client
//sync) but had no .cpp, so the game didn't link. This file gives every stub a
//baseline behaviour: gravity + world collision, client interpolation/leg
//animation via the shared ClientEntity base, and a plain server update with
//entity-vs-entity collision. Per-entity AI/skills can be layered on top later.

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/glm.hpp>

//stub headers reference these types without including them
#include <unordered_map>
#include <unordered_set>
#include <fstream>
#include <string>
struct WorldSaver;

#include <gameplay/armoredBoar.h>
#include <gameplay/bee.h>
#include <gameplay/blacksmithVillager.h>
#include <gameplay/capybaraChef.h>
#include <gameplay/caveSpider.h>
#include <gameplay/cow.h>
#include <gameplay/creeper.h>
#include <gameplay/crow.h>
#include <gameplay/crystalBat.h>
#include <gameplay/crystalGolem.h>
#include <gameplay/crystalSentinel.h>
#include <gameplay/enderling.h>
#include <gameplay/fox.h>
#include <gameplay/herbalistVillager.h>
#include <gameplay/hermitCrab.h>
#include <gameplay/honeyBear.h>
#include <gameplay/hydra.h>
#include <gameplay/juvenileDragon.h>
#include <gameplay/lavaSlug.h>
#include <gameplay/lightFairy.h>
#include <gameplay/manatee.h>
#include <gameplay/mimicChest.h>
#include <gameplay/mistGhost.h>
#include <gameplay/nomadTrader.h>
#include <gameplay/queenBee.h>
#include <gameplay/riverGuardian.h>
#include <gameplay/sandSerpent.h>
#include <gameplay/sheep.h>
#include <gameplay/skeleton.h>
#include <gameplay/skeletonPirate.h>
#include <gameplay/slime.h>
#include <gameplay/stoneGolem.h>
#include <gameplay/treeEnt.h>
#include <gameplay/wolf.h>

#include <multyPlayer/tick.h>
#include <chunkSystem.h>
#include <rendering/model.h>

namespace
{
	template <class E>
	void stubPhysicsUpdate(E &e, float deltaTime, decltype(chunkGetterSignature) *chunkGetter)
	{
		PhysicalSettings ps;
		ps.gravityModifier = 0.9f;
		e.updateForces(deltaTime, true, ps);
		e.resolveConstrainsAndUpdatePositions(chunkGetter, deltaTime, e.getColliderSize());
	}

	template <class E, class SERVER>
	bool stubServerUpdate(SERVER &s, float deltaTime, decltype(chunkGetterSignature) *chunkGetter,
		ServerChunkStorer &serverChunkStorer, std::uint64_t yourEID)
	{
		doCollisionWithOthers(s.getPosition(), s.entity.getMaxColliderSize(), s.entity.forces,
			serverChunkStorer, yourEID);

		s.entity.update(deltaTime, chunkGetter);
		return true;
	}

	void stubApplyLegRotation(glm::mat4 *skinningMatrix, float legAngle)
	{
		//the shared ClientEntity base tracks legAngle from movement speed, rotate
		//the two front/two back legs (bones 2..5) like the pig/cat models do.
		skinningMatrix[2] = skinningMatrix[2] * glm::rotate(legAngle, glm::vec3{1, 0, 0});
		skinningMatrix[3] = skinningMatrix[3] * glm::rotate(-legAngle, glm::vec3{1, 0, 0});
		skinningMatrix[4] = skinningMatrix[4] * glm::rotate(legAngle, glm::vec3{1, 0, 0});
		skinningMatrix[5] = skinningMatrix[5] * glm::rotate(-legAngle, glm::vec3{1, 0, 0});
	}
}

// NAME, WIDTH, HEIGHT, DEPTH, TEXTURE (ModelsManager::TexturesLoaded)
#define DEFINE_STUB_ENTITY(NAME, W, H, D, TEX) \
	void NAME::update(float deltaTime, decltype(chunkGetterSignature) *chunkGetter) \
	{ \
		stubPhysicsUpdate(*this, deltaTime, chunkGetter); \
	} \
	glm::vec3 NAME::getColliderSize() \
	{ \
		return getMaxColliderSize(); \
	} \
	glm::vec3 NAME::getMaxColliderSize() \
	{ \
		return glm::vec3(W, H, D); \
	} \
	void NAME##Client::update(float deltaTime, decltype(chunkGetterSignature) *chunkGetter) \
	{ \
		entityBuffered.update(deltaTime, chunkGetter); \
	} \
	void NAME##Client::setEntityMatrix(glm::mat4 *skinningMatrix) \
	{ \
		stubApplyLegRotation(skinningMatrix, getLegsAngle()); \
	} \
	int NAME##Client::getTextureIndex() \
	{ \
		return ModelsManager::TexturesLoaded::TEX; \
	} \
	bool NAME##Server::update(float deltaTime, decltype(chunkGetterSignature) *chunkGetter, \
		ServerChunkStorer &serverChunkStorer, std::minstd_rand &rng, std::uint64_t yourEID, \
		std::unordered_set<std::uint64_t> &othersDeleted, \
		std::unordered_map<std::uint64_t, std::unordered_map<glm::ivec3, PathFindingNode>> &pathFinding, \
		std::unordered_map<std::uint64_t, glm::dvec3> &playersPosition, \
		std::unordered_map<std::uint64_t, Client *> &allClients) \
	{ \
		(void)rng; (void)othersDeleted; (void)pathFinding; (void)playersPosition; (void)allClients; \
		return stubServerUpdate<NAME>(*this, deltaTime, chunkGetter, serverChunkStorer, yourEID); \
	} \
	void NAME##Server::appendDataToDisk(std::ofstream &, std::uint64_t) \
	{ \
	}

DEFINE_STUB_ENTITY(ArmoredBoar, 0.9f, 1.1f, 0.9f, DefaultTexture)
DEFINE_STUB_ENTITY(Bee, 0.5f, 0.5f, 0.5f, DefaultTexture)
DEFINE_STUB_ENTITY(BlacksmithVillager, 0.6f, 1.8f, 0.6f, DefaultTexture)
DEFINE_STUB_ENTITY(CapybaraChef, 0.7f, 0.8f, 0.7f, DefaultTexture)
DEFINE_STUB_ENTITY(CaveSpider, 0.8f, 0.5f, 0.8f, DefaultTexture)
DEFINE_STUB_ENTITY(Cow, 0.9f, 1.3f, 0.9f, CowTexture)
DEFINE_STUB_ENTITY(Creeper, 0.6f, 1.7f, 0.6f, CreeperTexture)
DEFINE_STUB_ENTITY(Crow, 0.5f, 0.5f, 0.5f, CrowTexture)
DEFINE_STUB_ENTITY(CrystalBat, 0.5f, 0.5f, 0.5f, DefaultTexture)
DEFINE_STUB_ENTITY(CrystalGolem, 1.2f, 2.4f, 1.2f, DefaultTexture)
DEFINE_STUB_ENTITY(CrystalSentinel, 1.0f, 2.0f, 1.0f, DefaultTexture)
DEFINE_STUB_ENTITY(Enderling, 0.6f, 1.8f, 0.6f, DefaultTexture)
DEFINE_STUB_ENTITY(Fox, 0.6f, 0.7f, 0.6f, FoxTexture)
DEFINE_STUB_ENTITY(HerbalistVillager, 0.6f, 1.8f, 0.6f, DefaultTexture)
DEFINE_STUB_ENTITY(HermitCrab, 0.9f, 0.8f, 0.9f, DefaultTexture)
DEFINE_STUB_ENTITY(HoneyBear, 0.9f, 1.2f, 0.9f, DefaultTexture)
DEFINE_STUB_ENTITY(JuvenileDragon, 1.0f, 1.4f, 1.0f, DefaultTexture)
DEFINE_STUB_ENTITY(LavaSlug, 0.8f, 0.6f, 0.8f, DefaultTexture)
DEFINE_STUB_ENTITY(LightFairy, 0.5f, 1.2f, 0.5f, DefaultTexture)
DEFINE_STUB_ENTITY(Manatee, 1.2f, 1.0f, 1.2f, DefaultTexture)
DEFINE_STUB_ENTITY(MimicChest, 1.0f, 1.0f, 1.0f, DefaultTexture)
DEFINE_STUB_ENTITY(MistGhost, 0.8f, 1.8f, 0.8f, DefaultTexture)
DEFINE_STUB_ENTITY(NomadTrader, 0.6f, 1.8f, 0.6f, DefaultTexture)
DEFINE_STUB_ENTITY(QueenBee, 0.8f, 0.8f, 0.8f, DefaultTexture)
DEFINE_STUB_ENTITY(RiverGuardian, 1.0f, 1.6f, 1.0f, DefaultTexture)
DEFINE_STUB_ENTITY(SandSerpent, 1.2f, 1.2f, 1.2f, DefaultTexture)
DEFINE_STUB_ENTITY(Sheep, 0.9f, 1.1f, 0.9f, SheepTexture)
DEFINE_STUB_ENTITY(Skeleton, 0.6f, 1.9f, 0.6f, SkeletonTexture)
DEFINE_STUB_ENTITY(SkeletonPirate, 0.6f, 1.9f, 0.6f, DefaultTexture)
DEFINE_STUB_ENTITY(StoneGolem, 1.2f, 2.6f, 1.2f, DefaultTexture)
DEFINE_STUB_ENTITY(TreeEnt, 1.0f, 2.4f, 1.0f, DefaultTexture)
DEFINE_STUB_ENTITY(Wolf, 0.7f, 0.9f, 0.7f, WolfTexture)

#undef DEFINE_STUB_ENTITY

//Slime: collider scales with slimeSize
void Slime::update(float deltaTime, decltype(chunkGetterSignature) *chunkGetter)
{
	stubPhysicsUpdate(*this, deltaTime, chunkGetter);
}

glm::vec3 Slime::getColliderSize()
{
	float s = slimeSize > 1 ? 1.0f : 0.6f;
	return glm::vec3(s, s * 0.75f, s);
}

glm::vec3 Slime::getMaxColliderSize()
{
	return glm::vec3(1.0f, 0.8f, 1.0f);
}

void SlimeClient::update(float deltaTime, decltype(chunkGetterSignature) *chunkGetter)
{
	entityBuffered.update(deltaTime, chunkGetter);
}

void SlimeClient::setEntityMatrix(glm::mat4 *skinningMatrix)
{
	stubApplyLegRotation(skinningMatrix, getLegsAngle());
}

int SlimeClient::getTextureIndex()
{
	return ModelsManager::TexturesLoaded::SlimeTexture;
}

bool SlimeServer::update(float deltaTime, decltype(chunkGetterSignature) *chunkGetter,
	ServerChunkStorer &serverChunkStorer, std::minstd_rand &rng, std::uint64_t yourEID,
	std::unordered_set<std::uint64_t> &othersDeleted,
	std::unordered_map<std::uint64_t, std::unordered_map<glm::ivec3, PathFindingNode>> &pathFinding,
	std::unordered_map<std::uint64_t, glm::dvec3> &playersPosition,
	std::unordered_map<std::uint64_t, Client *> &allClients)
{
	(void)rng; (void)othersDeleted; (void)pathFinding; (void)playersPosition; (void)allClients;
	return stubServerUpdate<Slime>(*this, deltaTime, chunkGetter, serverChunkStorer, yourEID);
}

void SlimeServer::appendDataToDisk(std::ofstream &, std::uint64_t)
{
}

//Hydra: big boss, texture depends on the variant
void Hydra::update(float deltaTime, decltype(chunkGetterSignature) *chunkGetter)
{
	stubPhysicsUpdate(*this, deltaTime, chunkGetter);
}

glm::vec3 Hydra::getColliderSize()
{
	return getMaxColliderSize();
}

glm::vec3 Hydra::getMaxColliderSize()
{
	return glm::vec3(2.2f, 3.0f, 2.2f);
}

void HydraClient::update(float deltaTime, decltype(chunkGetterSignature) *chunkGetter)
{
	entityBuffered.update(deltaTime, chunkGetter);
}

void HydraClient::setEntityMatrix(glm::mat4 *skinningMatrix)
{
	stubApplyLegRotation(skinningMatrix, getLegsAngle());
}

int HydraClient::getTextureIndex()
{
	switch (entityBuffered.variant)
	{
	case HydraVariantFrost: return ModelsManager::TexturesLoaded::HydraIceTexture;
	case HydraVariantVenom: return ModelsManager::TexturesLoaded::HydraPoisonTexture;
	default: return ModelsManager::TexturesLoaded::HydraFireTexture;
	}
}

bool HydraServer::update(float deltaTime, decltype(chunkGetterSignature) *chunkGetter,
	ServerChunkStorer &serverChunkStorer, std::minstd_rand &rng, std::uint64_t yourEID,
	std::unordered_set<std::uint64_t> &othersDeleted,
	std::unordered_map<std::uint64_t, std::unordered_map<glm::ivec3, PathFindingNode>> &pathFinding,
	std::unordered_map<std::uint64_t, glm::dvec3> &playersPosition,
	std::unordered_map<std::uint64_t, Client *> &allClients)
{
	(void)rng; (void)othersDeleted; (void)pathFinding; (void)playersPosition; (void)allClients;
	return stubServerUpdate<Hydra>(*this, deltaTime, chunkGetter, serverChunkStorer, yourEID);
}

void HydraServer::appendDataToDisk(std::ofstream &, std::uint64_t)
{
}
