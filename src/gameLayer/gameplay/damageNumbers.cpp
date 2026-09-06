#include "gameplay/damageNumbers.h"
#include <algorithm>
#include <cmath>

void DamageNumberSystem::update(float deltaTime)
{
	for (auto &n : numbers)
	{
		n.life -= deltaTime;
		if (n.life <= 0.f) continue;

		n.velocity.y += gravity * deltaTime;
		n.position += n.velocity * deltaTime;

		float t = 1.f - (n.life / n.maxLife);
		n.color.a = 1.f - t;
		n.scale = glm::mix(1.f, 0.5f, t);
	}

	numbers.erase(
		std::remove_if(numbers.begin(), numbers.end(),
			[](const DamageNumber &n) { return n.life <= 0.f; }),
		numbers.end());
}

void DamageNumberSystem::addNumber(const glm::vec3 &pos, int damage, bool critical)
{
	DamageNumber n;
	n.position = pos;
	n.position.x += ((float)std::rand() / RAND_MAX - 0.5f) * 0.5f;
	n.position.z += ((float)std::rand() / RAND_MAX - 0.5f) * 0.5f;
	n.velocity = glm::vec3(0.f, riseSpeed, 0.f);
	n.life = 1.f;
	n.maxLife = 1.f;
	n.text = std::to_string(damage);
	n.isCritical = critical;

	if (critical)
	{
		n.color = glm::vec4(1.f, 0.2f, 0.2f, 1.f);
		n.scale = 1.5f;
		n.velocity.y *= 1.3f;
	}
	else
	{
		n.color = glm::vec4(1.f, 1.f, 1.f, 1.f);
	}

	numbers.push_back(n);
}

void DamageNumberSystem::addNumber(const glm::vec3 &pos, const std::string &text, const glm::vec4 &color)
{
	DamageNumber n;
	n.position = pos;
	n.position.x += ((float)std::rand() / RAND_MAX - 0.5f) * 0.3f;
	n.velocity = glm::vec3(0.f, riseSpeed, 0.f);
	n.life = 1.2f;
	n.maxLife = 1.2f;
	n.text = text;
	n.color = color;
	n.scale = 1.f;

	numbers.push_back(n);
}

void DamageNumberSystem::clear()
{
	numbers.clear();
}
