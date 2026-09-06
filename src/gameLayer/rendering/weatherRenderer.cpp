#include <rendering/weatherRenderer.h>
#include <rendering/renderer.h>
#include <gamePlayLogic.h>
#include <iostream>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

void WeatherRenderer::init()
{
}

void WeatherRenderer::loadShaders()
{
}

void WeatherRenderer::create()
{
	screenSize = {};
	flashAlpha = 0.f;
	lensDistort = 0.f;
	boltLife = 0.f;
	needsFlash = false;
	needsLens = false;
	needsBolt = false;
}

void WeatherRenderer::update(float dt)
{
	if (flashAlpha > 0.f)
		flashAlpha = glm::max(0.f, flashAlpha - dt * 4.f);
	if (lensDistort > 0.f)
		lensDistort = glm::max(0.f, lensDistort - dt * 3.f);
	if (boltLife > 0.f)
		boltLife = glm::max(0.f, boltLife - dt);
}

void WeatherRenderer::render(Renderer &renderer, ProgramData &programData, Camera &c, WeatherState &weatherState, float dt)
{
	if (!weatherState.raining && !weatherState.currentBolt.alive && weatherState.ambientFlash < 0.01f)
		return;

	update(dt);

	if (weatherState.type == WeatherType::Weather_Rain || weatherState.type == WeatherType::Weather_Storm)
		renderRain(renderer, programData, c, weatherState, dt);
	else if (weatherState.type == WeatherType::Weather_Snow)
		renderSnow(renderer, programData, c, weatherState, dt);

	if (weatherState.currentBolt.alive && weatherState.currentBolt.points.size() >= 2)
		renderBolt(renderer, programData, c, weatherState);

	if (weatherState.ambientFlash > 0.05f)
		renderFlash(programData, weatherState);

	if (weatherState.wetness > 0.1f && weatherState.raining)
		renderLens(programData, weatherState);
}

void WeatherRenderer::renderRain(Renderer &renderer, ProgramData &programData, Camera &c, WeatherState &state, float dt)
{
	auto &ui = programData.ui.renderer2d;
	ui.pushCamera();

	glm::vec3 camPos = c.position;
	float windStr = glm::length(glm::vec2(state.windX, state.windZ));

	for (auto &p : state.particles)
	{
		if (!p.active()) continue;
		float alpha = glm::pow(p.life / p.maxLife, 0.5f);
		glm::vec3 pos = p.position;
		glm::vec3 vel = p.velocity;

		float streakLen = 0.3f + windStr * 0.15f;
		glm::vec3 end = pos + vel * streakLen * 0.5f;

		float size = p.size * 80.f;
		float fade = glm::clamp(1.f - glm::length(pos - camPos) / 100.f, 0.1f, 1.f) * alpha;
		if (fade < 0.01f) continue;

		glm::vec3 viewDir = glm::normalize(camPos - pos);
		glm::vec3 right = glm::normalize(glm::cross(glm::vec3(0,1,0), vel));
		glm::vec3 up = glm::cross(vel, right);

		float w = size * 0.5f;
		float h = streakLen * size * 2.f;

		glm::vec3 center = (pos + end) * 0.5f;
		glm::quat rot = glm::rotation(glm::vec3(0,0,-1), glm::normalize(end - pos));
		glm::vec3 fwd = glm::vec3(rot * glm::vec4(0,0,-1,0));
		glm::vec3 r = glm::vec3(rot * glm::vec4(right,0));
		glm::vec3 u = glm::vec3(rot * glm::vec4(up,0));

		glm::vec4 color = glm::vec4(0.65f, 0.75f, 0.9f, fade * 0.5f);
		ui.renderRectangle({center.x - w, center.y - h/2, w*2, h}, color);
	}

	ui.popCamera();
}

void WeatherRenderer::renderSnow(Renderer &renderer, ProgramData &programData, Camera &c, WeatherState &state, float dt)
{
	auto &ui = programData.ui.renderer2d;
	ui.pushCamera();

	glm::vec3 camPos = c.position;

	for (auto &p : state.particles)
	{
		if (!p.active()) continue;
		float alpha = glm::pow(p.life / p.maxLife, 0.3f);
		glm::vec3 pos = p.position;
		float size = p.size * 150.f;

		float fade = glm::clamp(1.f - glm::length(pos - camPos) / 80.f, 0.1f, 1.f) * alpha;
		if (fade < 0.01f) continue;

		glm::vec4 color = glm::vec4(0.9f, 0.95f, 1.f, fade * 0.6f);
		ui.renderRectangle({pos.x - size/2, pos.y - size/2, size, size}, color);
	}

	ui.popCamera();
}

void WeatherRenderer::renderBolt(Renderer &renderer, ProgramData &programData, Camera &c, WeatherState &state)
{
	if (state.currentBolt.points.size() < 2) return;

	glm::vec3 boltColor = glm::vec3(0.8f, 0.9f, 1.f);
	(void)renderer;

	for (size_t i = 1; i < state.currentBolt.points.size(); i++)
	{
		glm::vec3 a = state.currentBolt.points[i-1];
		glm::vec3 b = state.currentBolt.points[i];
		float dist = glm::length(glm::vec3(c.position) - (a+b)*0.5f);
		float alpha = glm::clamp(1.f / (dist * dist) * 1000.f, 0.1f, 0.9f);

		glm::vec3 mid = (a + b) * 0.5f;
		float screenSize = glm::clamp(8.f / dist, 0.1f, 15.f);

		glm::vec3 dir = b - a;
		float len = glm::length(dir);
		glm::vec3 perp = glm::normalize(glm::cross(glm::vec3(0,1,0), dir));
		glm::vec3 halfW = perp * screenSize * 0.5f;

		glm::vec4 color = glm::vec4(boltColor, alpha * 0.7f);
		auto &ui = programData.ui.renderer2d;
		ui.renderRectangle({mid.x - screenSize, mid.y - len*0.5f*screenSize*0.3f, screenSize*2, len*screenSize*0.6f}, color);
	}
}

void WeatherRenderer::renderFlash(ProgramData &programData, WeatherState &state)
{
	float alpha = state.ambientFlash * 0.45f;
	if (alpha < 0.01f) return;
	programData.ui.renderer2d.renderRectangle(
		{0,0, programData.ui.renderer2d.windowW, programData.ui.renderer2d.windowH},
		glm::vec4(1.f, 1.f, 1.f, alpha));
}

void WeatherRenderer::renderLens(ProgramData &programData, WeatherState &state)
{
	float wet = state.wetness;
	if (wet < 0.1f) return;
	int w = programData.ui.renderer2d.windowW;
	int h = programData.ui.renderer2d.windowH;

	float size = 200.f * wet;
	glm::vec4 tint = glm::vec4(0.5f, 0.6f, 0.8f, wet * 0.08f);
	for (int i = 0; i < 4; i++)
	{
		glm::vec2 corner = (i == 0) ? glm::vec2(0,0) : (i == 1) ? glm::vec2(w,0) : (i == 2) ? glm::vec2(0,h) : glm::vec2(w,h);
		programData.ui.renderer2d.renderRectangle({corner.x, corner.y, size, size}, tint);
	}
}
