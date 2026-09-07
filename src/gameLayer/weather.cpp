#include <weather.h>
#include <glm/gtc/random.hpp>
#include <cmath>
#include <algorithm>

void WeatherState::setWeather(WeatherType t)
{
	type = t;
	switch (t)
	{
	case WeatherType::Weather_Clear:
		targetIntensity = 0.f;
		raining = false;
		break;
	case WeatherType::Weather_Rain:
		targetIntensity = 0.7f;
		raining = true;
		break;
	case WeatherType::Weather_Storm:
		targetIntensity = 1.f;
		raining = true;
		break;
	case WeatherType::Weather_Snow:
		targetIntensity = 0.6f;
		raining = true;
		break;
	}
	flashTimer = 2.f + (float)(rand() % 5);
	pendingStrikes.clear();
}

void WeatherState::update(float dt, glm::vec3 playerPos, bool playerInWater)
{
	time += dt;
	intensity = glm::mix(intensity, targetIntensity, dt * 0.5f);
	if (intensity < 0.01f) intensity = 0.f;
	raining = (type != WeatherType::Weather_Clear && intensity > 0.05f);

	wetness = glm::mix(wetness, intensity, dt * 0.2f);
	if (!raining)
		wetness = glm::max(0.f, wetness - dt * 0.5f);

	updateWind(dt);
	updateParticles(dt, playerPos);

	if (type == WeatherType::Weather_Storm || type == WeatherType::Weather_Snow)
	{
		flashTimer -= dt;
		if (flashTimer <= 0.f)
		{
			strikeLightning(playerPos);
			flashTimer = 2.f + (float)(rand() % 8);
		}
	}
	else
	{
		flashTimer = 12.f;
		ambientFlash = 0.f;
	}

	if (freezePending)
	{
		freezeTimer -= dt;
		if (freezeTimer <= 0.f)
		{
			freezePending = false;
			if (playerInWater)
			{
				// handled elsewhere
			}
		}
	}
}

bool WeatherState::consumeLightningStrike(glm::vec3 &outPos)
{
	if (pendingStrikes.empty())
		return false;
	outPos = pendingStrikes.front();
	pendingStrikes.pop_front();
	return true;
}

bool WeatherState::consumeFreezingDamage()
{
	if (!freezePending)
		return false;
	freezePending = false;
	return true;
}

void WeatherState::spawnParticle(glm::vec3 playerPos, bool firstFill)
{
	if (particles.size() >= MAX_PARTICLES)
		return;

	WeatherParticle p;
	p.life = p.maxLife;
	p.size = 0.1f;
	p.swayPhase = (float)(rand() % 628) / 100.f;

	float spread = 60.f;
	p.position = playerPos + glm::vec3(
		(float)(rand() % 200 - 100) / 10.f * spread,
		(float)(rand() % 100) / 10.f * spread + 20.f,
		(float)(rand() % 200 - 100) / 10.f * spread);

	if (type == WeatherType::Weather_Snow)
	{
		p.velocity = glm::vec3(
			windX * 0.3f + (float)(rand() % 20 - 10) / 100.f,
			-0.5f,
			windZ * 0.3f + (float)(rand() % 20 - 10) / 100.f);
		p.maxLife = 3.f + (float)(rand() % 300) / 100.f;
		p.size = 0.05f + (float)(rand() % 50) / 100.f;
		p.life = p.maxLife;
	}
	else
	{
		p.velocity = glm::vec3(
			windX * 0.5f + (float)(rand() % 10 - 5) / 100.f,
			-8.f,
			windZ * 0.5f + (float)(rand() % 10 - 5) / 100.f);
		p.maxLife = 1.f + (float)(rand() % 50) / 100.f;
		p.size = 0.015f + (float)(rand() % 10) / 1000.f;
		p.life = p.maxLife;
	}

	p.swayPhase = (float)(rand() % 628) / 100.f;
	particles.push_back(p);
}

void WeatherState::strikeLightning(glm::vec3 playerPos)
{
	currentBolt.points.clear();
	currentBolt.life = currentBolt.maxLife;
	currentBolt.alive = true;
	currentBolt.points.push_back(glm::vec3(playerPos.x + (float)(rand() % 20 - 10), 40.f + (float)(rand() % 30), playerPos.z + (float)(rand() % 20 - 10)));

	glm::vec3 target = playerPos + glm::vec3(
		(float)(rand() % 20 - 10) * 0.5f,
		-5.f - (float)(rand() % 15),
		(float)(rand() % 20 - 10) * 0.5f);
	currentBolt.points.push_back(target);

	for (int i = 0; i < 3; i++)
	{
		glm::vec3 prev = currentBolt.points.back();
		glm::vec3 next = prev + glm::vec3(
			(float)(rand() % 20 - 10) / 5.f,
			-2.f - (float)(rand() % 10),
			(float)(rand() % 20 - 10) / 5.f);
		currentBolt.points.push_back(next);
	}

	ambientFlash = 0.8f;
	pendingStrikes.push_back(target);
}

void WeatherState::updateParticles(float dt, glm::vec3 playerPos)
{
	float spawnRate = (type == WeatherType::Weather_Snow) ? 0.02f : 0.05f;
	int desiredCount = (int)(intensity * MAX_PARTICLES);

	if (particles.size() < (size_t)desiredCount)
	{
		int toSpawn = std::min((int)(desiredCount - particles.size()), (int)(spawnRate * 60.f));
		for (int i = 0; i < toSpawn; i++)
			spawnParticle(playerPos, particles.empty());
	}

	auto it = particles.begin();
	while (it != particles.end())
	{
		it->life -= dt;
		if (it->life <= 0.f)
		{
			it = particles.erase(it);
			continue;
		}

		it->position = it->position + it->velocity * dt;
		it->swayPhase += dt * 3.f;

		if (type == WeatherType::Weather_Snow)
		{
			it->velocity.x += windX * 0.01f * dt;
			it->velocity.z += windZ * 0.01f * dt;
			it->velocity.x += glm::sin(it->swayPhase) * 0.005f;
			it->position.x += glm::sin(it->swayPhase) * 0.01f * dt;
		}
		else
		{
			it->velocity.x += windX * 0.02f * dt;
			it->velocity.z += windZ * 0.02f * dt;
		}

		if (it->position.y < -10.f ||
			glm::abs(it->position.x - playerPos.x) > 80.f ||
			glm::abs(it->position.z - playerPos.z) > 80.f)
		{
			it = particles.erase(it);
		}
		else
		{
			++it;
		}
	}
}

void WeatherState::updateWind(float dt)
{
	windX = glm::sin(time * 0.05f) * 1.f + glm::sin(time * 0.12f) * 0.5f;
	windZ = glm::cos(time * 0.07f) * 0.75f + glm::cos(time * 0.15f) * 0.25f;
	if (type == WeatherType::Weather_Clear)
	{
		windX *= 0.2f;
		windZ *= 0.2f;
	}
}
