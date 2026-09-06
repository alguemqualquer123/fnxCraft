#include "gameplay/particleSystem.h"
#include <algorithm>
#include <cstdlib>
#include <cmath>

void ParticleSystem::update(float deltaTime)
{
	for (auto &emitter : emitters)
	{
		if (!emitter.active) continue;

		emitter.emissionTimer += deltaTime;
		float emitInterval = 1.f / emitter.rate;

		while (emitter.emissionTimer >= emitInterval && (int)emitter.particles.size() < maxParticles)
		{
			emitter.emissionTimer -= emitInterval;

			Particle p;
			p.position = emitter.position;

			float rx = ((float)std::rand() / RAND_MAX - 0.5f) * 2.f;
			float ry = ((float)std::rand() / RAND_MAX - 0.5f) * 2.f;
			float rz = ((float)std::rand() / RAND_MAX - 0.5f) * 2.f;

			p.velocity = emitter.direction;
			p.velocity.x += rx * emitter.spread;
			p.velocity.y += ry * emitter.spread;
			p.velocity.z += rz * emitter.spread;

			p.color = emitter.startColor;
			p.size = emitter.startSize;
			p.life = emitter.lifetime;
			p.maxLife = emitter.lifetime;
			p.gravity = emitter.gravity;
			p.active = true;

			emitter.particles.push_back(p);
		}

		for (auto &p : emitter.particles)
		{
			if (!p.active) continue;

			p.life -= deltaTime;
			if (p.life <= 0.f)
			{
				p.active = false;
				continue;
			}

			float t = 1.f - (p.life / p.maxLife);
			p.velocity.y += p.gravity * deltaTime;
			p.position += p.velocity * deltaTime;

			p.size = glm::mix(emitter.startSize, emitter.endSize, t);
			p.color = glm::mix(emitter.startColor, emitter.endColor, t);
		}

		emitter.particles.erase(
			std::remove_if(emitter.particles.begin(), emitter.particles.end(),
				[](const Particle &p) { return !p.active; }),
			emitter.particles.end());
	}
}

void ParticleSystem::render()
{
	// Render particles using points or quads
	// This is a placeholder - actual rendering depends on the renderer
	for (auto &emitter : emitters)
	{
		for (auto &p : emitter.particles)
		{
			if (!p.active) continue;
			// p.position, p.size, p.color would be sent to GPU
		}
	}
}

void ParticleSystem::emit(const glm::vec3 &pos, const glm::vec3 &dir, float spread,
	float rate, float lifetime, float size,
	const glm::vec4 &startColor, const glm::vec4 &endColor,
	float gravity)
{
	ParticleEmitter e;
	e.position = pos;
	e.direction = dir;
	e.spread = spread;
	e.rate = rate;
	e.lifetime = lifetime;
	e.startSize = size;
	e.endSize = size * 0.3f;
	e.startColor = startColor;
	e.endColor = endColor;
	e.gravity = gravity;
	e.active = true;
	emitters.push_back(e);
}

void ParticleSystem::emitBurst(const glm::vec3 &pos, int count, float lifetime,
	float size, const glm::vec4 &color, float speed)
{
	ParticleEmitter e;
	e.position = pos;
	e.direction = glm::vec3(0.f, 1.f, 0.f);
	e.spread = 1.f;
	e.rate = count * 10.f;
	e.lifetime = lifetime;
	e.startSize = size;
	e.endSize = size * 0.2f;
	e.startColor = color;
	e.endColor = glm::vec4(color.r, color.g, color.b, 0.f);
	e.gravity = -5.f;
	e.active = true;

	for (int i = 0; i < count; i++)
	{
		Particle p;
		p.position = pos;

		float rx = ((float)std::rand() / RAND_MAX - 0.5f) * 2.f;
		float ry = ((float)std::rand() / RAND_MAX);
		float rz = ((float)std::rand() / RAND_MAX - 0.5f) * 2.f;

		p.velocity = glm::vec3(rx, ry, rz) * speed;
		p.color = color;
		p.size = size;
		p.life = lifetime;
		p.maxLife = lifetime;
		p.gravity = -5.f;
		p.active = true;
		e.particles.push_back(p);
	}

	emitters.push_back(e);
}

void ParticleSystem::clear()
{
	emitters.clear();
}

void ParticlePresets::dust(ParticleEmitter &e, const glm::vec3 &pos)
{
	e.position = pos;
	e.direction = glm::vec3(0.f, 0.5f, 0.f);
	e.spread = 0.8f;
	e.rate = 15.f;
	e.lifetime = 0.6f;
	e.startSize = 0.15f;
	e.endSize = 0.05f;
	e.startColor = glm::vec4(0.6f, 0.5f, 0.3f, 0.8f);
	e.endColor = glm::vec4(0.6f, 0.5f, 0.3f, 0.f);
	e.gravity = 1.f;
	e.active = true;
}

void ParticlePresets::hit(ParticleEmitter &e, const glm::vec3 &pos)
{
	e.position = pos;
	e.direction = glm::vec3(0.f, 1.f, 0.f);
	e.spread = 1.5f;
	e.rate = 30.f;
	e.lifetime = 0.3f;
	e.startSize = 0.08f;
	e.endSize = 0.02f;
	e.startColor = glm::vec4(1.f, 0.8f, 0.2f, 1.f);
	e.endColor = glm::vec4(1.f, 0.3f, 0.f, 0.f);
	e.gravity = 8.f;
	e.active = true;
}

void ParticlePresets::death(ParticleEmitter &e, const glm::vec3 &pos)
{
	e.position = pos;
	e.direction = glm::vec3(0.f, 1.f, 0.f);
	e.spread = 2.f;
	e.rate = 40.f;
	e.lifetime = 1.f;
	e.startSize = 0.2f;
	e.endSize = 0.05f;
	e.startColor = glm::vec4(1.f, 1.f, 1.f, 1.f);
	e.endColor = glm::vec4(0.8f, 0.8f, 0.8f, 0.f);
	e.gravity = -2.f;
	e.active = true;
}

void ParticlePresets::waterSplash(ParticleEmitter &e, const glm::vec3 &pos)
{
	e.position = pos;
	e.direction = glm::vec3(0.f, 1.f, 0.f);
	e.spread = 0.5f;
	e.rate = 25.f;
	e.lifetime = 0.5f;
	e.startSize = 0.1f;
	e.endSize = 0.03f;
	e.startColor = glm::vec4(0.3f, 0.6f, 1.f, 0.8f);
	e.endColor = glm::vec4(0.3f, 0.6f, 1.f, 0.f);
	e.gravity = 10.f;
	e.active = true;
}

void ParticlePresets::fire(ParticleEmitter &e, const glm::vec3 &pos)
{
	e.position = pos;
	e.direction = glm::vec3(0.f, 1.f, 0.f);
	e.spread = 0.3f;
	e.rate = 20.f;
	e.lifetime = 0.8f;
	e.startSize = 0.15f;
	e.endSize = 0.02f;
	e.startColor = glm::vec4(1.f, 0.6f, 0.1f, 1.f);
	e.endColor = glm::vec4(1.f, 0.1f, 0.f, 0.f);
	e.gravity = -3.f;
	e.active = true;
}
