#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <vector>
#include <functional>

struct Particle
{
	glm::vec3 position = glm::vec3(0.f);
	glm::vec3 velocity = glm::vec3(0.f);
	glm::vec4 color = glm::vec4(1.f);
	float size = 1.f;
	float life = 1.f;
	float maxLife = 1.f;
	float gravity = -9.8f;
	bool active = true;
};

struct ParticleEmitter
{
	glm::vec3 position = glm::vec3(0.f);
	glm::vec3 direction = glm::vec3(0.f, 1.f, 0.f);
	float spread = 0.5f;
	float rate = 10.f;
	float lifetime = 1.f;
	float startSize = 0.1f;
	float endSize = 0.02f;
	glm::vec4 startColor = glm::vec4(1.f);
	glm::vec4 endColor = glm::vec4(1.f, 1.f, 1.f, 0.f);
	float gravity = -9.8f;
	float emissionTimer = 0.f;
	bool active = true;
	std::vector<Particle> particles;
};

struct ParticleSystem
{
	std::vector<ParticleEmitter> emitters;
	int maxParticles = 1000;

	void update(float deltaTime);
	void render();
	void emit(const glm::vec3 &pos, const glm::vec3 &dir, float spread,
		float rate, float lifetime, float size,
		const glm::vec4 &startColor, const glm::vec4 &endColor,
		float gravity = -9.8f);
	void emitBurst(const glm::vec3 &pos, int count, float lifetime,
		float size, const glm::vec4 &color, float speed = 5.f);
	void clear();
};

struct ParticlePresets
{
	static void dust(ParticleEmitter &e, const glm::vec3 &pos);
	static void hit(ParticleEmitter &e, const glm::vec3 &pos);
	static void death(ParticleEmitter &e, const glm::vec3 &pos);
static void waterSplash(ParticleEmitter &e, const glm::vec3 &pos);
	static void fire(ParticleEmitter &e, const glm::vec3 &pos);
};
