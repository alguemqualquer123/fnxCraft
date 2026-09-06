#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <vector>

struct SwingTrailPoint
{
	glm::vec3 position = glm::vec3(0.f);
	float time = 0.f;
	float width = 0.1f;
};

struct SwingTrail
{
	std::vector<SwingTrailPoint> points;
	float duration = 0.3f;
	float width = 0.15f;
	glm::vec4 color = glm::vec4(1.f, 1.f, 1.f, 0.8f);
	bool active = false;
	float time = 0.f;

	void start(const glm::vec3 &startPos);
	void update(const glm::vec3 &currentPos, float deltaTime);
	void stop();
	void getVertices(std::vector<glm::vec3> &verts, std::vector<glm::vec4> &colors) const;
};

struct SwingTrailSystem
{
	std::vector<SwingTrail> trails;
	int maxTrails = 8;

	void update(float deltaTime);
	SwingTrail &beginTrail(const glm::vec3 &startPos);
	void getRenderData(std::vector<glm::vec3> &verts, std::vector<glm::vec4> &colors) const;
};
