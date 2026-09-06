#pragma once
#include <gameplay/entity.h>
#include <gameplay/life.h>
#include <vector>
#include <unordered_map>

struct FishingManager
{
void update(float deltaTime, glm::dvec3 playerPos, bool playerInWater);
void castLine(glm::vec3 from, glm::vec3 dir, float power);
bool canCatch();
void reel(bool state);
void reset();

struct Bobber {
	glm::vec3 pos;
	float stateTimer = 0.f;
	bool active = false;
};
Bobber bobber;

struct FishingLine
{
	glm::vec3 position;
	float life = 0.f;
	void update(float dt);
};
