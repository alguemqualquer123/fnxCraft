#pragma once
#include <glm/vec3.hpp>
#include <vector>

struct PointLight
{
	glm::vec3 position = {};
	glm::vec3 color = {1,1,1};
	float intensity = 1.0f;
	float radius = 8.0f;
};

struct PointLightManager
{
	std::vector<PointLight> lights;
	void clear(){ lights.clear(); }
	void addLight(glm::vec3 pos, glm::vec3 color = {1,0.8f,0.6f}, float intensity=1.0f, float radius=8.0f){
		lights.push_back({pos,color,intensity,radius});
	}
	void update(){}
};
