#pragma once
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include "weather.h"

struct Renderer;
struct ProgramData;
struct Camera;

struct WeatherRenderer
{
	void init();
	void loadShaders();
	void create();
	void update(float dt);
	void render(Renderer &renderer, ProgramData &programData, Camera &c, WeatherState &weatherState, float dt);

	glm::vec3 windDirection = {1,0,0};
	float windStrength = 0.5f;

	glm::ivec2 screenSize = {};
	float flashAlpha = 0.f;
	float lensDistort = 0.f;
	float boltLife = 0.f;

	glm::vec3 boltStart = {};
	glm::vec3 boltEnd = {};

	bool needsFlash = false;
	bool needsLens = false;
	bool needsBolt = false;

private:
	void renderRain(Renderer &renderer, ProgramData &programData, Camera &c, WeatherState &state, float dt);
	void renderSnow(Renderer &renderer, ProgramData &programData, Camera &c, WeatherState &state, float dt);
	void renderBolt(Renderer &renderer, ProgramData &programData, Camera &c, WeatherState &state);
	void renderFlash(ProgramData &programData, WeatherState &state);
	void renderLens(ProgramData &programData, WeatherState &state);
};
