#pragma once
#include <glm/glm.hpp>

struct Effect
{
	int timerMs = 0; 
	//todo change to unsigned short? but make sure the calculations dont overflow!!

	float getTimerInSecconds();
};

struct Effects
{
	enum EffectsNames
	{
		Saturated,
		Regeneration,
		Poisoned,
		Shielding,
		Speed,
		Slowness,
		Strength,
		Weakness,
		FireResistance,
		NightVision,
		Invisibility,

		Effects_Count
	};

	Effect allEffects[Effects_Count] = {};

	void passTimeMs(int ms);

	void applyEffects(Effects &other);

	int getArmour() 
	{
		if (allEffects[Shielding].timerMs > 0)
		{
			return 8;
		}

		return 0;
	}

	float getSpeedMultiplier()
	{
		float speed = 1.f;
		if (allEffects[Speed].timerMs > 0) speed += 0.3f;
		if (allEffects[Slowness].timerMs > 0) speed -= 0.3f;
		return glm::max(speed, 0.1f);
	}

	float getDamageMultiplier()
	{
		float damage = 1.f;
		if (allEffects[Strength].timerMs > 0) damage += 0.5f;
		if (allEffects[Weakness].timerMs > 0) damage -= 0.5f;
		return glm::max(damage, 0.1f);
	}

	bool isInvisible()
	{
		return allEffects[Invisibility].timerMs > 0;
	}

	bool hasNightVision()
	{
		return allEffects[NightVision].timerMs > 0;
	}

	bool isFireResistant()
	{
		return allEffects[FireResistance].timerMs > 0;
	}
};


