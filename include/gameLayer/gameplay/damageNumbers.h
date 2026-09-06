#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <vector>
#include <string>

struct DamageNumber
{
	glm::vec3 position = glm::vec3(0.f);
	glm::vec3 velocity = glm::vec3(0.f);
	float life = 1.f;
	float maxLife = 1.f;
	std::string text;
	glm::vec4 color = glm::vec4(1.f, 1.f, 1.f, 1.f);
	float scale = 1.f;
	bool isCritical = false;
};

struct DamageNumberSystem
{
	std::vector<DamageNumber> numbers;
	float gravity = -3.f;
	float riseSpeed = 2.f;
	float fadeSpeed = 1.f;

	void update(float deltaTime);
	void addNumber(const glm::vec3 &pos, int damage, bool critical = false);
	void addNumber(const glm::vec3 &pos, const std::string &text, const glm::vec4 &color = glm::vec4(1.f));
	void clear();
};
