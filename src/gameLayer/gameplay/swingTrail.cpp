#include "gameplay/swingTrail.h"
#include <algorithm>
#include <cmath>

void SwingTrail::start(const glm::vec3 &startPos)
{
	points.clear();
	SwingTrailPoint p;
	p.position = startPos;
	p.time = 0.f;
	p.width = width;
	points.push_back(p);
	active = true;
	time = 0.f;
}

void SwingTrail::update(const glm::vec3 &currentPos, float deltaTime)
{
	if (!active) return;

	time += deltaTime;

	for (auto &p : points)
	{
		p.time += deltaTime;
	}

	points.erase(
		std::remove_if(points.begin(), points.end(),
			[this](const SwingTrailPoint &p) { return p.time > duration; }),
		points.end());

	if (points.size() > 0 && points.back().time < deltaTime * 2.f)
	{
		SwingTrailPoint p;
		p.position = currentPos;
		p.time = 0.f;
		p.width = width;
		points.push_back(p);
	}

	if (points.empty())
	{
		active = false;
	}
}

void SwingTrail::stop()
{
	active = false;
}

void SwingTrail::getVertices(std::vector<glm::vec3> &verts, std::vector<glm::vec4> &colors) const
{
	if (points.size() < 2) return;

	for (size_t i = 0; i < points.size() - 1; i++)
	{
		const auto &p0 = points[i];
		const auto &p1 = points[i + 1];

		float t0 = 1.f - (p0.time / duration);
		float t1 = 1.f - (p1.time / duration);

		glm::vec3 dir = glm::normalize(p1.position - p0.position);
		glm::vec3 up = glm::vec3(0.f, 1.f, 0.f);
		glm::vec3 right = glm::normalize(glm::cross(dir, up));

		float w0 = p0.width * t0;
		float w1 = p1.width * t1;

		verts.push_back(p0.position - right * w0);
		verts.push_back(p0.position + right * w0);
		verts.push_back(p1.position + right * w1);

		verts.push_back(p0.position - right * w0);
		verts.push_back(p1.position + right * w1);
		verts.push_back(p1.position - right * w1);

		glm::vec4 c0 = color;
		c0.a *= t0;
		glm::vec4 c1 = color;
		c1.a *= t1;

		for (int j = 0; j < 6; j++)
		{
			colors.push_back(j < 3 ? c0 : c1);
		}
	}
}

void SwingTrailSystem::update(float deltaTime)
{
	for (auto &trail : trails)
	{
		if (trail.active)
		{
			trail.update(trail.points.empty() ? glm::vec3(0.f) : trail.points.back().position, deltaTime);
		}
	}

	trails.erase(
		std::remove_if(trails.begin(), trails.end(),
			[](const SwingTrail &t) { return !t.active && t.points.empty(); }),
		trails.end());
}

SwingTrail &SwingTrailSystem::beginTrail(const glm::vec3 &startPos)
{
	if ((int)trails.size() >= maxTrails)
	{
		for (auto &t : trails)
		{
			if (!t.active)
			{
				t.start(startPos);
				return t;
			}
		}
	}

	trails.emplace_back();
	trails.back().start(startPos);
	return trails.back();
}

void SwingTrailSystem::getRenderData(std::vector<glm::vec3> &verts, std::vector<glm::vec4> &colors) const
{
	for (const auto &trail : trails)
	{
		trail.getVertices(verts, colors);
	}
}
