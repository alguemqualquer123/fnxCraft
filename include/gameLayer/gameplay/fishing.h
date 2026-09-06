#pragma once
#include <gameplay/entity.h>
#include <gameplay/life.h>
#include <glm/glm.hpp>
#include <algorithm>
#include <cmath>

//bobber/line states. gamePlayLogic shows a HUD bar per state.
enum class FishingState : int
{
	Waiting = 0,	//fish not interested yet, bar fills with waterTimer/3
	Nibbling,		//fish approaching, bar fills with fishApproach
	Biting,			//hook it! click again fast while progress lasts
};

struct FishingManager
{
	void startCast(glm::vec3 from, glm::vec3 dir, float power);
	void update(float deltaTime, glm::dvec3 playerPos, bool playerInWater);
	bool canCatch();
	void reel(bool caught);
	void reset();

	struct Bobber
	{
		glm::vec3 pos = {};
		float stateTimer = 0.f;
		float waterTimer = 0.f;		//time floating on water (Waiting)
		float fishApproach = 0.f;	//0..1 how close the fish is (Nibbling)
		float progress = 0.f;		//remaining time to click (Biting), 1..0
		float floatBaseY = 0.f;		//water surface height where it landed
		float bobPhase = 0.f;
		bool hitWater = false;
		bool active = false;
		FishingState state = FishingState::Waiting;
	};
	Bobber bobber;
};

inline void FishingManager::startCast(glm::vec3 from, glm::vec3 dir, float power)
{
	glm::vec3 d = glm::normalize(dir);
	//bobber flies ~4 to ~18 blocks, then sinks until it finds water
	bobber.pos = from + d * (4.f + 14.f * std::clamp(power, 0.f, 1.f));
	bobber.floatBaseY = bobber.pos.y;
	bobber.hitWater = false;
	bobber.bobPhase = 0.f;
	bobber.waterTimer = 0.f;
	bobber.fishApproach = 0.f;
	bobber.progress = 0.f;
	bobber.stateTimer = 0.f;
	bobber.state = FishingState::Waiting;
	bobber.active = true;
}

inline void FishingManager::update(float deltaTime, glm::dvec3 playerPos, bool playerInWater)
{
	if (!bobber.active) return;

	bobber.stateTimer += deltaTime;
	bobber.bobPhase += deltaTime;

	if (!playerInWater)
	{
		//still flying/sinking towards the water. keep dropping so a cast
		//aimed at a pond lower than the player lands on its surface.
		bobber.pos.y -= 2.5f * deltaTime;
		//never found water -> reel the line back automatically
		if (bobber.pos.y < (float)playerPos.y - 12.f)
		{
			reset();
		}
		return;
	}

	//first contact with water: remember the surface height
	if (!bobber.hitWater)
	{
		bobber.hitWater = true;
		bobber.floatBaseY = bobber.pos.y;
		bobber.waterTimer = 0.f;
		bobber.fishApproach = 0.f;
		bobber.progress = 0.f;
		bobber.state = FishingState::Waiting;
	}

	//bob slightly (never above the surface so the water check keeps passing)
	bobber.pos.y = bobber.floatBaseY - std::max(0.f, std::sin(bobber.bobPhase * 3.f)) * 0.04f;

	switch (bobber.state)
	{
	case FishingState::Waiting:
		bobber.waterTimer += deltaTime;
		if (bobber.waterTimer >= 3.f)
		{
			bobber.state = FishingState::Nibbling;
			bobber.fishApproach = 0.f;
			bobber.stateTimer = 0.f;
		}
		break;
	case FishingState::Nibbling:
		bobber.fishApproach += deltaTime / 1.5f;
		if (bobber.fishApproach >= 1.f)
		{
			bobber.state = FishingState::Biting;
			bobber.progress = 1.f;
			bobber.stateTimer = 0.f;
		}
		break;
	case FishingState::Biting:
		//the player has ~2.5s to click before the fish escapes
		bobber.progress -= deltaTime / 2.5f;
		if (bobber.progress <= 0.f)
		{
			//fish got away: wait for the next one
			bobber.state = FishingState::Waiting;
			bobber.waterTimer = 0.f;
			bobber.fishApproach = 0.f;
			bobber.progress = 0.f;
			bobber.stateTimer = 0.f;
		}
		break;
	}
}

inline bool FishingManager::canCatch()
{
	return bobber.active && bobber.state == FishingState::Biting && bobber.progress > 0.f;
}

inline void FishingManager::reel(bool /*caught*/)
{
	//the item (rawFish) is granted by the caller on canCatch(); we just reel in.
	reset();
}

inline void FishingManager::reset()
{
	bobber = Bobber{};
}
