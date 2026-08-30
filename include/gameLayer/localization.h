#pragma once

#include <string>

enum class Language : int
{
	English = 0,
	Portuguese_BR,
	
	LanguageCount
};

Language &getCurrentLanguage();
void setCurrentLanguage(Language lang);
const char *getLanguageName(Language lang);

// Load/save language from player settings
void loadLanguageSettings();
void saveLanguageSettings();

// ============================================================
// Main menu
// ============================================================
const char *loc_JoinGame();
const char *loc_Settings();
const char *loc_Play();
const char *loc_Exit();

// ============================================================
// In-game menu
// ============================================================
const char *loc_GameMenu();
const char *loc_BackToGame();
const char *loc_BackToMenu();
const char *loc_Chat();
const char *loc_DeathMessage();
const char *loc_Respawn();

// ============================================================
// Status / common
// ============================================================
const char *loc_FPS();
const char *loc_IP();
const char *loc_CouldntJoinServer();
const char *loc_EnterIP();

// ============================================================
// Settings menus
// ============================================================
const char *loc_Rendering();
const char *loc_Volume();
const char *loc_AudioSettings();
const char *loc_TexturesPacks();
const char *loc_Skin();
const char *loc_ChangeSkin();
const char *loc_Language();
const char *loc_LanguageName();

// ============================================================
// Render settings labels
// ============================================================
const char *loc_RenderingSettings();
const char *loc_ViewDistance();
const char *loc_LodStrength();
const char *loc_Tonemapper();
const char *loc_Shadows();
const char *loc_WaterType();
const char *loc_WaterSettings();
const char *loc_WaterColor();
const char *loc_UnderWaterColor();
const char *loc_UnderwaterFogStrength();
const char *loc_UnderwaterFogDistance();
const char *loc_UnderwaterFogGradient();
const char *loc_PBR();
const char *loc_SSR();
const char *loc_SSRSettings();
const char *loc_MaxLights();
const char *loc_MaxLightsShort();
const char *loc_UseLights();
const char *loc_LightsSettings();
const char *loc_LightsStrength();
const char *loc_LightsStrengthShort();
const char *loc_Bloom();
const char *loc_BloomSettings();
const char *loc_BloomMultiplier();
const char *loc_BloomThreshold();
const char *loc_FXAA();
const char *loc_Exposure();
const char *loc_FogGradient();
const char *loc_FogGradientTooltip();
const char *loc_ColorPostProcessing();
const char *loc_ResetSettings();
const char *loc_Vignette();
const char *loc_Saturation();
const char *loc_Vibrance();
const char *loc_Gamma();
const char *loc_ShadowBoost();
const char *loc_HighlightBoost();
const char *loc_Lift();
const char *loc_Gain();
const char *loc_ChunkBuildingThreads();
const char *loc_ShadowsPerformanceTip();

// ============================================================
// Volume settings
// ============================================================
const char *loc_MasterVolume();
const char *loc_MusicVolume();
const char *loc_UIVolume();
const char *loc_SoundsVolume();

// ============================================================
// World selector
// ============================================================
const char *loc_SelectWorld();
const char *loc_CreateNewWorld();

// ============================================================
// Confirmations
// ============================================================
const char *loc_AreYouSureExit();
const char *loc_AreYouSureLeave();
const char *loc_Yes();
const char *loc_No();

// ============================================================
// Survival
// ============================================================
const char *loc_SurvivalMode();
const char *loc_CreativeMode();

// ============================================================
// Item names (indexed by item id)
// ============================================================
const char *loc_ItemName(int itemId);
