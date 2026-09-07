#pragma once

#include <stdint.h>

namespace AudioEngine
{

	void init();

	void loadAllMusic();

	void startPlayingMusicAtIndex(int index);

	void update();

	void playRandomNightMusic();

	void playTitleMusic();

	void stopAllMusicAndSounds();

	void playSound(int sound, float level);
	void playSoundWithPitch(int sound, float level, float pitch = 1.f);

	void playHitSound();

	void playHurtSound();

	float &getMasterVolume();
	float &getMusicVolume();
	float &getUIVolume();
	float &getSoundsVolume();

	void loadSettingsOrSetToDefaultIfFail();

	void saveSettings();

	bool isMusicPlaying();

	enum sounds
	{
		none = 0,
		
		grass,
		dirt,
		stone,
		sand,
		wood,
		glassBreak,
		glassStep,
		leaves,
		snow,
		metal,
		wool,
		clay,
		sandStone,
		bricks,
		gravel,
		ice,
		iceBreak,
		volcanicRockActive,
		volcanicRockInActive,

		toolBreakingWood,
		toolBreakingStone,
		toolBreakingIron,

		crackStone,

		hit,
		fallLow,
		fallMedium,
		fallHigh,
		hurt,

		uiButtonPress,
		uiButtonBack,
		uiOn,
		uiOff,
		uiSlider,

		// Water sounds
		waterSplash,
		waterSwim,
		waterExit,
		waterIn,
		waterOut,

		// UI checkbox sounds
		uiCheckBoxOn,
		uiCheckBoxOff,

		// Additional step sounds
		lavaSizzle,
		netherrack,

		// Weather sounds
		rainAmbient,
		thunder,
		wind,

		LAST_SOUND
	};

};


// NOTE: use uint16_t (BlockType) so the symbols match the definitions in
// shared/blocks.cpp — `unsigned int` here creates undefined overloads at link
// time (BlockType is `using BlockType = uint16_t`).
bool isAnyDirtBlock(uint16_t type);
bool isAnyClayBlock(uint16_t type);
bool isAnySandyBlock(uint16_t type);
bool isAnyWoodenBlock(uint16_t type);
bool isAnySemiHardBlock(uint16_t type);
bool isAnyStone(uint16_t type);
bool isAnyPlant(uint16_t type);
bool isAnyLeaves(uint16_t type);
bool isAnyWool(uint16_t type);
bool isAnyUnbreakable(uint16_t type);
bool isTriviallyBreakable(uint16_t type);
bool isBricksSound(uint16_t type);
bool isVolcanicActiveSound(uint16_t type);
bool isVolcanicInActiveSound(uint16_t type);

int getSoundForBlockBreaking(unsigned int blockType);

int getSoundForBlockStepping(unsigned int blockType);

constexpr static float MINING_BLOCK_SOUND_VOLUME = 0.8;
constexpr static float PLACED_BLOCK_SOUND_VOLUME = 0.9;
constexpr static float BREAKED_BLOCK_SOUND_VOLUME = 1.0;
constexpr static float STEPPING_SOUND_VOLUME = 0.75;
constexpr static float HIT_SOUND_VOLUME = 0.8;
constexpr static float FALL_SOUND_VOLUME = 0.65;
constexpr static float UI_SOUND_VOLUME = 0.9;
