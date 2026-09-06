#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <gameplay/fish.h>
#include <iostream>
#include <glm/glm.hpp>
#include <multyPlayer/serverChunkStorer.h>
#include <chunkSystem.h>
#include <rendering/model.h>

void Fish::update(float deltaTime, decltype(chunkGetterSignature) *chunkGetter)
{
	// Fish physics - lightweight with buoyancy
	PhysicalSettings ps;
	ps.gravityModifier = 0.2f;  // Fish are buoyant
	ps.sideFriction = 0.5f;

	// Check if fish is in water
	bool inWater = isPositionInWater(position, chunkGetter);

	if (inWater)
	{
		// Apply water physics for swimming
		applyWaterPhysics(forces, position, deltaTime, ps, true, WATER_SWIM_IMPULSE * 0.5f);
	}
	else
	{
		// If not in water, apply some gravity but with friction
		ps.gravityModifier = 0.8f;
	}

	updateForces(deltaTime, true, ps);
	resolveConstrainsAndUpdatePositions(chunkGetter, deltaTime, getColliderSize(), ps);
}

glm::vec3 Fish::getColliderSize()
{
	return getMaxColliderSize();
}

glm::vec3 Fish::getMaxColliderSize()
{
	return glm::vec3(0.5, 0.3, 0.5);  // Fish are small and flat
}

void FishClient::update(float deltaTime, decltype(chunkGetterSignature) *chunkGetter)
{
	// Update swimming animation
	entityBuffered.swimTimer += deltaTime;
	
	// Animate swimming motion
	if (entityBuffered.swimTimer > 0.1f)
	{
		entityBuffered.swimTimer = 0;
	}
	
	entityBuffered.update(deltaTime, chunkGetter);
}

void FishClient::setEntityMatrix(glm::mat4 *skinningMatrix)
{
	// Simple swimming animation - slight body wiggle
	float swimAngle = sin(entityBuffered.swimTimer * 10.0f) * 0.2f;
	
	// Apply rotation to body (assuming bone index 0 is body)
	if (skinningMatrix)
	{
		skinningMatrix[0] = skinningMatrix[0] * glm::rotate(swimAngle, glm::vec3(0, 0, 1));
	}
}

int FishClient::getTextureIndex()
{
	// Return a texture index for fish - we'll need to add this to the model manager
	// For now, return 0 or a placeholder
	return 0;
}

bool FishServer::update(float deltaTime, decltype(chunkGetterSignature) *chunkGetter,
	ServerChunkStorer &serverChunkStorer, std::minstd_rand &rng, std::uint64_t yourEID,
	std::unordered_set<std::uint64_t> &othersDeleted,
	std::unordered_map<std::uint64_t, std::unordered_map<glm::ivec3, PathFindingNode>> &pathFinding,
	std::unordered_map<std::uint64_t, glm::dvec3> &playersPosition,
	std::unordered_map < std::uint64_t, Client *> &allClients
	)
{
	// Check if fish is in water
	bool inWater = isPositionInWater(entity.position, chunkGetter);

	// Fish behavior - random swimming
	swimTimer -= deltaTime;
	directionChangeTimer -= deltaTime;
	surfaceCheckTimer -= deltaTime;

	// Change direction periodically
	if (directionChangeTimer <= 0)
	{
		directionChangeTimer = getRandomNumberFloat(rng, 2.0f, 5.0f);
		
		// Random direction change
		float angle = getRandomNumberFloat(rng, 0.0f, glm::two_pi<float>());
		entity.bodyOrientation = glm::vec2(cos(angle), sin(angle));
		
		// Occasionally swim towards surface
		if (getRandomChance(rng, 0.3f))
		{
			// Swim up slightly
			entity.forces.velocity.y += getRandomNumberFloat(rng, 0.5f, 1.5f);
		}
	}

	// Swimming movement
	if (swimTimer <= 0)
	{
		swimTimer = getRandomNumberFloat(rng, 0.5f, 1.5f);
		
		// Apply swimming force
		float speed = getRandomNumberFloat(rng, 1.0f, 3.0f);
		glm::vec3 moveDir = glm::vec3(entity.bodyOrientation.x, 0, entity.bodyOrientation.y) * speed;
		
		entity.forces.velocity += moveDir;
	}

	// Keep fish in water
	if (!inWater)
	{
		// If fish is out of water, push it back down
		entity.forces.velocity.y -= 2.0f * deltaTime;
		
		// Fish takes damage out of water
		entity.life.life -= (int)(10.0f * deltaTime);
		if (entity.life.life <= 0)
		{
			return false;  // Fish died
		}
	}

	// Limit fish speed
	float maxSpeed = 5.0f;
	if (glm::length(entity.forces.velocity) > maxSpeed)
	{
		entity.forces.velocity = glm::normalize(entity.forces.velocity) * maxSpeed;
	}

	// Update animation timer
	entity.swimTimer += deltaTime;

	// Update entity physics
	entity.update(deltaTime, chunkGetter);

	return true;
}

void FishServer::appendDataToDisk(std::ofstream &f, std::uint64_t eId)
{
	// Save fish data if needed
}
