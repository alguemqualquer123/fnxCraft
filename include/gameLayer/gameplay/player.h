#pragma once
#include <gameplay/entity.h>
#include <multyPlayer/server.h>
#include <gameplay/items.h>
#include <gl2d/gl2d.h>
#include <gameplay/life.h>
#include <gameplay/gameplayRules.h>
#include <gameplay/effects.h>
#include <gameplay/animationSystem.h>
#include <gameplay/particleSystem.h>
#include <gameplay/damageNumbers.h>
#include <gameplay/swingTrail.h>

EntityStats getPlayerStats(struct PlayerInventory &inventory);

#define PLAYER_DEFAULT_LIFE Life(100)

// Survival mode constants
constexpr static float HUNGER_MAX = 100.f;
constexpr static float THIRST_MAX = 100.f;
constexpr static float HUNGER_DEPLETION_RATE = 0.5f;    // Points per second (lasts ~3.3 min)
constexpr static float THIRST_DEPLETION_RATE = 0.8f;    // Points per second (lasts ~2 min)
constexpr static float HUNGER_DAMAGE_RATE = 2.f;         // Damage per second when starving
constexpr static float THIRST_DAMAGE_RATE = 3.f;         // Damage per second when dehydrating
constexpr static float FOOD_SATIATION_BONUS = 15.f;      // Bonus to hunger when eating

//this is the shared data
struct Player : public PhysicalEntity, public CollidesWithPlacedBlocks,
	public CanPushOthers, public CanBeKilled, public CanBeAttacked,
	public CanHaveEffects, public HasOrientationAndHeadTurnDirection, 
	public HasEyesAndPupils<EYE_ANIMATION_TYPE_PLAYER>,
	public MovementSpeedForLegsAnimations,
	public Animatable
{

	//todo use mem compare
	bool operator== (Player & other)
	{
		if(
			lookDirectionAnimation == other.lookDirectionAnimation &&
			bodyOrientation == other.bodyOrientation &&
			fly == other.fly
			)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	bool operator!= (Player & other)
	{
		return !(*this == other);
	}

	void flyFPS(glm::vec3 direction, glm::vec3 lookDirection);

	void moveFPS(glm::vec3 direction, glm::vec3 lookDirection, float deltaTime);

	int chunkDistance = 10; //TODO remove this from here!

	glm::vec3 getColliderSize();
	glm::vec3 getColliderOffset() { return {0, 0, 0}; }

	//todo implement!
	void update(float deltaTime, decltype(chunkGetterSignature) *chunkGetter);


	static glm::vec3 getMaxColliderSize();

	bool fly = 0;
	bool isCrouching = 0;
	bool isProne = 0;
	bool isRunning = 0;
	bool isSwimmingAnim = 0;
	float crouchTransition = 0.f;
};

//here we store things like gamemode
struct OtherPlayerSettings
{
	constexpr static int SURVIVAL = 0;
	constexpr static int CREATIVE = 1;

	unsigned char gameMode = 0;
	char commandPermisionLevel = 1; //0 nothing, 1 Player, 2 Moderator, 3 Operator (owner)
};

//this is the player struct when playing locally
struct LocalPlayer
{
	PlayerInventory inventory;

	Player entity = {};

	std::uint64_t entityId = 0;

	OtherPlayerSettings otherPlayerSettings = {};
	//dodo add some other data here like inventory

	glm::ivec3 currentBlockInteractWith = {0,-1,0};
	unsigned char isInteractingWithBlock = 0;

	Life life = PLAYER_DEFAULT_LIFE;
	Life lastLife = PLAYER_DEFAULT_LIFE;
	float justHealedTimer = 0;
	float justRecievedDamageTimer = 0;

	Effects effects;

	// Swimming and drowning
	bool isInWater = false;
	bool isSwimming = false;
	float drowningTimer = DROWNING_MAX_TIME;  // Time left before drowning damage starts
	float drowningDamageTimer = 0;            // Timer for repeated drowning damage

	// Survival: hunger and thirst
	float hunger = HUNGER_MAX;
	float thirst = THIRST_MAX;
	float hungerDamageTimer = 0;
	float thirstDamageTimer = 0;

	ParticleSystem particles;
	SwingTrailSystem swingTrails;
};


//the other players locally
struct PlayerClient: public ClientEntity<Player, PlayerClient>
{

	//todo other player settings here!

	PlayerAnimator animator;
	ParticleSystem particles;
	DamageNumberSystem damageNumbers;
	SwingTrailSystem swingTrails;
	float lastDeltaTime = 1.f / 60.f;

	void update(float deltaTime, decltype(chunkGetterSignature) *chunkGetter);
	void setEntityMatrix(glm::mat4 *skinningMatrix);

	int getTextureIndex();

};


//todo update function
struct PlayerServer: public ServerEntity<Player>
{

	OtherPlayerSettings otherPlayerSettings = {};

	PlayerInventory inventory;


	//this also represents the interaction type
	unsigned char interactingWithBlock = 0;
	unsigned char revisionNumberInteraction = 0;
	glm::ivec3 currentBlockInteractWithPosition = {0, -1, 0};

	Life lifeLastFrame = PLAYER_DEFAULT_LIFE;
	Life newLife = PLAYER_DEFAULT_LIFE;
	bool forceUpdateLife = 0;

	//we update the effects every 20 ticks or if we set it
	// as dirty by setting this to 0
	short updateEffectsTicksTimer = 20;

	void applyDamageOrLife(short difference)
	{
		if (otherPlayerSettings.gameMode == OtherPlayerSettings::CREATIVE) { return; }

		int life = newLife.life;
		life += difference;

		if (life > newLife.maxLife) { life = newLife.maxLife; }

		newLife.life = life;
		if (difference < 0)
		{
			healingDelayCounterSecconds = 0;
		}
	}

	//todo move to server entity
	struct EffectsTimer
	{
		float regen = 0;
		float poison = 0;


	}effectsTimers;

	bool killed = 0;

	// Survival: hunger and thirst
	float hunger = HUNGER_MAX;
	float thirst = THIRST_MAX;
	float hungerDamageTimer = 0;
	float thirstDamageTimer = 0;
	float survivalTickTimer = 0;  // Timer for survival depletion ticks

	//used for life regeneration
	float notIncreasedLifeSinceTimeSecconds = 0;
	float healingDelayCounterSecconds = BASE_HEALTH_DELAY_TIME;

	void kill();

	Armour getArmour() 
	{
		Armour rez{};
		EntityStats s = getPlayerStats(inventory);
		rez.armour = s.armour;
		rez.armour += effects.getArmour();
		rez.normalize();
		return rez;
	};

	glm::ivec2 lastChunkPositionWhenAnUpdateWasSent = {};

	float calculateHealingDelayTime();
	float calculateHealingRegenTime();

	bool isUnaware() { return false; }

	void signalHit(glm::vec3 d) {};
};


EntityStats getPlayerStats(PlayerInventory &inventory);