#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include "gameplay/player.h"
#include <iostream>
#include <rendering/model.h>

static bool animatorInitialized = false;

void Player::flyFPS(glm::vec3 direction, glm::vec3 lookDirection)
{
	lookDirection.y = 0;
	float l = glm::length(lookDirection);

	if (!l) { return; }

	lookDirection /= l;

	//forward
	float forward = -direction.z;
	float leftRight = direction.x;
	float upDown = direction.y;

	glm::vec3 move = {};

	move += glm::vec3(0, 1, 0) * upDown;
	move += glm::normalize(glm::cross(lookDirection, glm::vec3(0, 1, 0))) * leftRight;
	move += lookDirection * forward;

	//applyImpulse(this->forces, move);
	this->position += move;
}


void Player::moveFPS(glm::vec3 direction, glm::vec3 lookDirection, float deltaTime)
{
	lookDirection.y = 0;
	lookDirection = glm::normalize(lookDirection);

	//forward
	float forward = -direction.z;
	float leftRight = direction.x;

	glm::vec3 move = {};

	move += glm::normalize(glm::cross(lookDirection, glm::vec3(0, 1, 0))) * leftRight;
	move += lookDirection * forward;

	this->moveDynamic({move.x, move.z}, deltaTime);
}

glm::vec3 Player::getColliderSize()
{
	if(isProne) return glm::vec3(0.8,0.6,0.8);
	if(isCrouching) return glm::vec3(0.8,1.2,0.8);
	return getMaxColliderSize();
}

void Player::update(float deltaTime, decltype(chunkGetterSignature) *chunkGetter)
{
	glm::dvec3 waterCheck = position + glm::dvec3(0, 0.6, 0);
	glm::dvec3 footCheck = position + glm::dvec3(0, 0.15, 0);
	glm::dvec3 headCheck = position + glm::dvec3(0, 1.4, 0);
	bool midWater = isPositionInWater(waterCheck, chunkGetter);
	bool footWater = isPositionInWater(footCheck, chunkGetter);
	bool headWater = isPositionInWater(headCheck, chunkGetter);
	bool inWater = midWater || footWater;
	isSwimmingAnim = inWater;

	if (inWater && !fly)
	{
		// In water: apply water physics with buoyancy
		PhysicalSettings ps;
		ps.gravityModifier = 0.3f;  // Reduced gravity in water

		applyWaterPhysics(forces, position, deltaTime, ps, true, WATER_BUOYANCY_FORCE);

		updateForces(deltaTime, true, ps);
		if(headWater) forces.velocity.y = glm::max(forces.velocity.y, -1.2f);
	}
	else
	{
		updateForces(deltaTime, !fly);
	}

	resolveConstrainsAndUpdatePositions(chunkGetter, deltaTime, getColliderSize());
}

glm::vec3 Player::getMaxColliderSize()
{
	return glm::vec3(0.8, 1.8, 0.8);
}



//todo move update here
void PlayerClient::update(float deltaTime, decltype(chunkGetterSignature) *chunkGetter)
{
	lastDeltaTime = deltaTime;
	entityBuffered.update(deltaTime, chunkGetter);
	particles.update(deltaTime);
	damageNumbers.update(deltaTime);
	swingTrails.update(deltaTime);
}

void PlayerClient::setEntityMatrix(glm::mat4 *m)
{
	if (!animatorInitialized)
	{
		animator.init();
		animatorInitialized = true;
	}

	auto &e = entityBuffered;

	animator.isAttacking = animationStateClient.isAttacking;
	animator.isGrounded = e.forces.colidesBottom();
	animator.isSwimming = e.isSwimmingAnim;
	animator.isCrouching = e.isCrouching;
	animator.isProne = e.isProne;
	animator.isFlying = e.fly;
	animator.currentSpeed = glm::length(glm::vec2(e.forces.velocity.x, e.forces.velocity.z));

	animator.update(lastDeltaTime);

	auto &pose = animator.skeleton.bones;
	if (m[0].length() > 0 && pose.size() > 4) {
		glm::quat lookRot = glm::quat(1,0,0,0);
		glm::vec3 lookDir = e.lookDirectionAnimation;
		if (glm::length(lookDir) > 0.01f) {
			lookDir = glm::normalize(lookDir);
			float yaw = atan2(lookDir.x, lookDir.z);
			float pitch = asin(glm::clamp(-lookDir.y, -1.f, 1.f));
			lookRot = glm::angleAxis(yaw, glm::vec3(0,1,0)) * glm::angleAxis(pitch, glm::vec3(1,0,0));
		}
		m[0] = m[0] * glm::mat4_cast(lookRot * pose[4].rotation);
	}
	if (m[1].length() > 0 && pose.size() > 2)
		m[1] = m[1] * glm::translate(pose[2].position) * glm::mat4_cast(pose[2].rotation);
	else if (m[1].length() > 0 && pose.size() > 1)
		m[1] = m[1] * glm::translate(pose[0].position) * glm::mat4_cast(pose[0].rotation);
	if (m[2].length() > 0 && pose.size() > 16)
		m[2] = m[2] * glm::mat4_cast(pose[16].rotation);
	if (m[3].length() > 0 && pose.size() > 13)
		m[3] = m[3] * glm::mat4_cast(pose[13].rotation);
	if (m[4].length() > 0 && pose.size() > 10)
		m[4] = m[4] * glm::mat4_cast(pose[10].rotation);
	if (m[5].length() > 0 && pose.size() > 6)
		m[5] = m[5] * glm::mat4_cast(pose[6].rotation);

	float jumpSquash = 0.f;
	if (!e.forces.colidesBottom() && !e.isSwimmingAnim)
	{
		jumpSquash = glm::clamp(e.forces.velocity.y * 0.04f, -0.25f, 0.25f);
	}

	if (e.isProne)
	{
		glm::mat4 rot = glm::rotate(glm::radians(85.f), glm::vec3(1, 0, 0));
		glm::mat4 tr = glm::translate(glm::vec3(0, -0.7f, 0.3f));
		for (int i = 0; i < 6; i++)
			if (m[i].length() > 0) m[i] = tr * rot * m[i];
	}
	else if (e.isCrouching)
	{
		glm::mat4 tr = glm::translate(glm::vec3(0, -0.25f, 0));
		glm::mat4 sc = glm::scale(glm::vec3(1, 0.85f, 1));
		for (int i = 0; i < 6; i++)
			if (m[i].length() > 0) m[i] = tr * sc * m[i];
	}
	else if (jumpSquash != 0.f)
	{
		glm::mat4 sc = glm::scale(glm::vec3(1.f - jumpSquash, 1.f + jumpSquash, 1.f - jumpSquash));
		for (int i = 0; i < 6; i++)
			if (m[i].length() > 0) m[i] = m[i] * sc;
	}
}

int PlayerClient::getTextureIndex()
{
	return ModelsManager::TexturesLoaded::SteveTexture;
}

void PlayerServer::kill()
{
	killed = true;
	effects = {};
	newLife.life = 0;
	lifeLastFrame.life = 0;
	notIncreasedLifeSinceTimeSecconds = 0;
	interactingWithBlock = 0;
	revisionNumberInteraction = 0;
	
	effectsTimers = {};
}

float PlayerServer::calculateHealingDelayTime()
{
	float rez = BASE_HEALTH_DELAY_TIME;

	for (int i = PlayerInventory::EQUIPEMENT_START_INDEX; i < PlayerInventory::EQUIPEMENT_START_INDEX +
		PlayerInventory::MAX_EQUIPEMENT_SLOTS; i++)
	{
		auto item = inventory.getItemFromIndex(i, 0);

		if (item->type == ItemTypes::bandage)
		{
			rez -= 5;
		}
	}


	return std::max(rez, 0.f);
}

float PlayerServer::calculateHealingRegenTime()
{
	return BASE_HEALTH_REGEN_TIME;
}

EntityStats getPlayerStats(PlayerInventory &inventory)
{
	EntityStats rez;
	rez.armour = 0;
	rez.runningSpeed = 8;
	auto add = [&](Item &it){ if(it.type){ EntityStats s=it.getItemStats(); rez.add(s); } };
	add(inventory.headArmour);
	add(inventory.chestArmour);
	add(inventory.bootsArmour);
	for(int i=PlayerInventory::EQUIPEMENT_START_INDEX;i<PlayerInventory::EQUIPEMENT_START_INDEX+PlayerInventory::MAX_EQUIPEMENT_SLOTS;i++){
		auto *it = inventory.getItemFromIndex(i,nullptr);
		if(it && it->type){ EntityStats s=it->getItemStats(); rez.add(s); }
	}
	auto isLeather = [](Item&a,Item&b,Item&c){return a.type==leatherHelmet&&b.type==leatherChestPlate&&c.type==leatherBoots;};
	auto isCopper = [](Item&a,Item&b,Item&c){return a.type==copperHelmet&&b.type==copperChestPlate&&c.type==copperBoots;};
	auto isLead = [](Item&a,Item&b,Item&c){return a.type==leadHelmet&&b.type==leadChestPlate&&c.type==leadBoots;};
	auto isIron = [](Item&a,Item&b,Item&c){return a.type==ironHelmet&&b.type==ironChestPlate&&c.type==ironBoots;};
	auto isSilver = [](Item&a,Item&b,Item&c){return a.type==silverHelmet&&b.type==silverChestPlate&&c.type==silverBoots;};
	auto isGold = [](Item&a,Item&b,Item&c){return a.type==goldHelmet&&b.type==goldChestPlate&&c.type==goldBoots;};
	if(isLeather(inventory.headArmour,inventory.chestArmour,inventory.bootsArmour)) rez.armour += 1;
	else if(isCopper(inventory.headArmour,inventory.chestArmour,inventory.bootsArmour)) rez.armour += 2;
	else if(isLead(inventory.headArmour,inventory.chestArmour,inventory.bootsArmour)) rez.armour += 1;
	else if(isIron(inventory.headArmour,inventory.chestArmour,inventory.bootsArmour)) rez.armour += 1;
	else if(isSilver(inventory.headArmour,inventory.chestArmour,inventory.bootsArmour)) rez.armour += 2;
	else if(isGold(inventory.headArmour,inventory.chestArmour,inventory.bootsArmour)) rez.armour += 2;
	rez.normalize();
	return rez;
}
