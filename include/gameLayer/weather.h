#pragma once

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/vec3.hpp>
#include <glm/gtx/norm.hpp>
#include <vector>
#include <deque>

//used by /weather command: 0 = clear, 1 = rain, 2 = storm, 3 = snow
enum class WeatherType : int
{
	Weather_Clear = 0,
	Weather_Rain = 1,
	Weather_Storm = 2,
	Weather_Snow = 3,
};

//one rain drop / snow flake / vfx particle
struct WeatherParticle
{
	glm::vec3 position = {};
	glm::vec3 velocity = {};
	float life = 0.f;
	float maxLife = 1.f;
	float size = 0.1f;
	float swayPhase = 0.f;
	bool active() const { return life > 0.f; }
	void deactivate() { life = 0.f; }
};

//the currently displayed lightning bolt
struct LightningBolt
{
	std::vector<glm::vec3> points;
	float life = 0.f;
	float maxLife = 0.45f;
	bool alive = false;
};

struct WeatherState
{
	static constexpr int FREEZE_DAMAGE = 2;

	WeatherType type = WeatherType::Weather_Clear;
	float time = 0.f;

	float intensity = 0.f; //0..1 current weather strength
	float targetIntensity = 0.f;
	float wetness = 0.f; //0..1, feeds ground wetness + lens drops
	bool raining = false;

	float windX = 0.f;
	float windZ = 0.f;

	float ambientFlash = 0.f; //0..1 screen flash on lightning
	LightningBolt currentBolt;

	std::vector<WeatherParticle> particles;

	float lightningStrikeRadius = 6.f;
	float lightningStrikeDamage = 7.f;

	void setWeather(WeatherType t);
	void update(float dt, glm::vec3 playerPos, bool playerInWater);
	bool consumeLightningStrike(glm::vec3 &outPos);
	bool consumeFreezingDamage();

	// internals
	float freezeTimer = 0.f;
	bool freezePending = false;
	float flashTimer = 12.f; //countdown to the next lightning strike
	std::deque<glm::vec3> pendingStrikes;

	glm::vec2 getWind() const { return {windX, windZ}; }
	float getWindStrength() const { return glm::length(glm::vec2{windX, windZ}); }

	static constexpr int MAX_PARTICLES = 1400;

private:

	void spawnParticle(glm::vec3 playerPos, bool firstFill);
	void strikeLightning(glm::vec3 playerPos);
	void updateParticles(float dt, glm::vec3 playerPos);
	void updateWind(float dt);
};
