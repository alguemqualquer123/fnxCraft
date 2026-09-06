#pragma once
#include <gameplay/zombie.h>
#include <gameplay/droppedItem.h>
#include <gameplay/pig.h>
#include <gameplay/player.h>
#include <gameplay/cat.h>
#include <gameplay/goblin.h>
#include <gameplay/trainingDummy.h>
#include <gameplay/scareCrow.h>
#include <gameplay/fish.h>
#include <gameplay/bee.h>
#include <gameplay/queenBee.h>
#include <gameplay/slime.h>
#include <gameplay/creeper.h>
#include <gameplay/enderling.h>
#include <gameplay/skeleton.h>
#include <gameplay/stoneGolem.h>
#include <gameplay/wolf.h>
#include <gameplay/fox.h>
#include <gameplay/crow.h>
#include <gameplay/manatee.h>
#include <gameplay/caveSpider.h>
#include <gameplay/crystalBat.h>
#include <gameplay/capybaraChef.h>
#include <gameplay/riverGuardian.h>
#include <gameplay/treeEnt.h>
#include <gameplay/nomadTrader.h>
#include <gameplay/mimicChest.h>
#include <gameplay/lightFairy.h>
#include <gameplay/armoredBoar.h>
#include <gameplay/sandSerpent.h>
#include <gameplay/mistGhost.h>
#include <gameplay/hermitCrab.h>
#include <gameplay/honeyBear.h>
#include <gameplay/lavaSlug.h>
#include <gameplay/crystalSentinel.h>
#include <gameplay/blacksmithVillager.h>
#include <gameplay/herbalistVillager.h>
#include <gameplay/skeletonPirate.h>
#include <gameplay/juvenileDragon.h>
#include <gameplay/crystalGolem.h>
#include <gameplay/hydra.h>
#include <gameplay/sheep.h>
#include <gameplay/cow.h>
#include "repeat.h"


//!!!!!!!!!!! DONT FORGET TO ALSO UPDATE THIS ONE
#define EntitiesTypesCountMACRO 43
constexpr static unsigned int EntitiesTypesCount = EntitiesTypesCountMACRO;
#define REPEAT_FOR_ALL_ENTITIES(FN) REPEAT_43(FN)
//!!!!!!!!!!! ^ ALSO THIS ONE            ^^^^^
#define REPEAT_FOR_ALL_ENTITIES_NO_PLAYERS(FN) REPEAT_NO_0_43(FN)
//!!!!!!!!!!! ^ ALSO THIS ONE!							^^^^^

//CHECK ALL OF THIS FILE FOR CHANGES

//!!!!!!!!!!! v ALSO THIS ONE!							vvvvv
namespace EntityType
{
	enum
	{
		player = 0,
		droppedItems,
		zombies,
		pigs,
		cats,
		goblins,
		trainingDummy,
		scareCrow,
		fish,
		bees,
		queenBees,
		slime,
		creepers,
		enderlings,
		skeletons,
		stoneGolems,
		wolves,
		foxes,
		crows,
		manatees,
		caveSpiders,
		crystalBats,
		capybaraChefs,
		riverGuardians,
		treeEnts,
		nomadTraders,
		mimicChests,
		lightFairies,
		armoredBoars,
		sandSerpents,
		mistGhosts,
		hermitCrabs,
		honeyBears,
		lavaSlugs,
		crystalSentinels,
		blacksmithVillagers,
		herbalistVillagers,
		skeletonPirates,
		juvenileDragons,
		crystalGolems,
		hydras,
		sheeps,
		cows,
	};
};

inline unsigned char getEntityTypeFromEID(std::uint64_t eid)
{
	return (eid >> 56);
}

inline unsigned char getOnlyIdFromEID(std::uint64_t eid)
{
	return ((eid << 6) >> 6);
}

template<typename B>
struct EntityGetter
{

	template<unsigned int I>
	auto entityGetter()
	{
		B *baseClass = (B*)this;

		if constexpr (I == 0)
		{
			return &baseClass->players;
		}
		else if constexpr (I == 1)
		{
			return &baseClass->droppedItems;
		}
		else if constexpr (I == 2)
		{
			return &baseClass->zombies;
		}
		else if constexpr (I == 3)
		{
			return &baseClass->pigs;
		}
		else if constexpr (I == 4)
		{
			return &baseClass->cats;
		}
		else if constexpr (I == 5)
		{
			return &baseClass->goblins;
		}
		else if constexpr (I == 6)
		{
			return &baseClass->trainingDummy;
		}
		else if constexpr (I == 7)
		{
			return &baseClass->scareCrows;
		}
		else if constexpr (I == 8)
		{
			return &baseClass->fish;
		}
		else if constexpr (I == 9)
		{
			return &baseClass->bees;
		}
		else if constexpr (I == 10)
		{
			return &baseClass->queenBees;
		}
		else if constexpr (I == 11)
		{
			return &baseClass->slime;
		}
		else if constexpr (I == 12)
		{
			return &baseClass->creepers;
		}
		else if constexpr (I == 13)
		{
			return &baseClass->enderlings;
		}
		else if constexpr (I == 14)
		{
			return &baseClass->skeletons;
		}
		else if constexpr (I == 15)
		{
			return &baseClass->stoneGolems;
		}
		else if constexpr (I == 16)
		{
			return &baseClass->wolves;
		}
		else if constexpr (I == 17)
		{
			return &baseClass->foxes;
		}
		else if constexpr (I == 18)
		{
			return &baseClass->crows;
		}
		else if constexpr (I == 19)
		{
			return &baseClass->manatees;
		}
		else if constexpr (I == 20)
		{
			return &baseClass->caveSpiders;
		}
		else if constexpr (I == 21)
		{
			return &baseClass->crystalBats;
		}
		else if constexpr (I == 22)
		{
			return &baseClass->capybaraChefs;
		}
		else if constexpr (I == 23)
		{
			return &baseClass->riverGuardians;
		}
		else if constexpr (I == 24)
		{
			return &baseClass->treeEnts;
		}
		else if constexpr (I == 25)
		{
			return &baseClass->nomadTraders;
		}
		else if constexpr (I == 26)
		{
			return &baseClass->mimicChests;
		}
		else if constexpr (I == 27)
		{
			return &baseClass->lightFairies;
		}
		else if constexpr (I == 28)
		{
			return &baseClass->armoredBoars;
		}
		else if constexpr (I == 29)
		{
			return &baseClass->sandSerpents;
		}
		else if constexpr (I == 30)
		{
			return &baseClass->mistGhosts;
		}
		else if constexpr (I == 31)
		{
			return &baseClass->hermitCrabs;
		}
		else if constexpr (I == 32)
		{
			return &baseClass->honeyBears;
		}
		else if constexpr (I == 33)
		{
			return &baseClass->lavaSlugs;
		}
		else if constexpr (I == 34)
		{
			return &baseClass->crystalSentinels;
		}
		else if constexpr (I == 35)
		{
			return &baseClass->blacksmithVillagers;
		}
		else if constexpr (I == 36)
		{
			return &baseClass->herbalistVillagers;
		}
		else if constexpr (I == 37)
		{
			return &baseClass->skeletonPirates;
		}
		else if constexpr (I == 38)
		{
			return &baseClass->juvenileDragons;
		}
		else if constexpr (I == 39)
		{
			return &baseClass->crystalGolems;
		}
		else if constexpr (I == 40)
		{
			return &baseClass->hydras;
		}
		else if constexpr (I == 41)
		{
			return &baseClass->sheeps;
		}
		else if constexpr (I == 42)
		{
			return &baseClass->cows;
		}
	}

template<unsigned int I>
	bool removeEntityBasedOnBlockPosition(int x, unsigned char y, int z)
	{
		auto &container = *entityGetter<I>();
		auto id = fromBlockPosToEntityID(x, y, z, I);
		
		auto found = container.find(id);
		if (found != container.end())
		{
			container.erase(found);
			return true;
		}

		return false;
	};

	template<unsigned int I>
	void addEmptyEntityBasedOnBlockPosition(int x, unsigned char y, int z)
	{
		auto &container = *entityGetter<I>();
		auto id = fromBlockPosToEntityID(x, y, z, I);

		container[id] = {};
	};

};

//server
struct EntityData: public EntityGetter<EntityData>
{
	std::unordered_map <std::uint64_t, PlayerServer*> players;

	std::unordered_map<std::uint64_t, DroppedItemServer> droppedItems;
	std::unordered_map<std::uint64_t, ZombieServer> zombies;
	std::unordered_map<std::uint64_t, PigServer> pigs;
	std::unordered_map<std::uint64_t, CatServer> cats;
	std::unordered_map<std::uint64_t, GoblinServer> goblins;
	std::unordered_map<std::uint64_t, TrainingDummyServer> trainingDummy;
	std::unordered_map<std::uint64_t, ScareCrowServer> scareCrows;
	std::unordered_map<std::uint64_t, FishServer> fish;
	std::unordered_map<std::uint64_t, BeeServer> bees;
	std::unordered_map<std::uint64_t, QueenBeeServer> queenBees;
	std::unordered_map<std::uint64_t, SlimeServer> slime;
	std::unordered_map<std::uint64_t, CreeperServer> creepers;
	std::unordered_map<std::uint64_t, EnderlingServer> enderlings;
	std::unordered_map<std::uint64_t, SkeletonServer> skeletons;
	std::unordered_map<std::uint64_t, StoneGolemServer> stoneGolems;
	std::unordered_map<std::uint64_t, WolfServer> wolves;
	std::unordered_map<std::uint64_t, FoxServer> foxes;
	std::unordered_map<std::uint64_t, CrowServer> crows;
	std::unordered_map<std::uint64_t, ManateeServer> manatees;
	std::unordered_map<std::uint64_t, CaveSpiderServer> caveSpiders;
	std::unordered_map<std::uint64_t, CrystalBatServer> crystalBats;
	std::unordered_map<std::uint64_t, CapybaraChefServer> capybaraChefs;
	std::unordered_map<std::uint64_t, RiverGuardianServer> riverGuardians;
	std::unordered_map<std::uint64_t, TreeEntServer> treeEnts;
	std::unordered_map<std::uint64_t, NomadTraderServer> nomadTraders;
	std::unordered_map<std::uint64_t, MimicChestServer> mimicChests;
	std::unordered_map<std::uint64_t, LightFairyServer> lightFairies;
	std::unordered_map<std::uint64_t, ArmoredBoarServer> armoredBoars;
	std::unordered_map<std::uint64_t, SandSerpentServer> sandSerpents;
	std::unordered_map<std::uint64_t, MistGhostServer> mistGhosts;
	std::unordered_map<std::uint64_t, HermitCrabServer> hermitCrabs;
	std::unordered_map<std::uint64_t, HoneyBearServer> honeyBears;
	std::unordered_map<std::uint64_t, LavaSlugServer> lavaSlugs;
	std::unordered_map<std::uint64_t, CrystalSentinelServer> crystalSentinels;
	std::unordered_map<std::uint64_t, BlacksmithVillagerServer> blacksmithVillagers;
	std::unordered_map<std::uint64_t, HerbalistVillagerServer> herbalistVillagers;
	std::unordered_map<std::uint64_t, SkeletonPirateServer> skeletonPirates;
	std::unordered_map<std::uint64_t, JuvenileDragonServer> juvenileDragons;
	std::unordered_map<std::uint64_t, CrystalGolemServer> crystalGolems;
	std::unordered_map<std::uint64_t, HydraServer> hydras;
	std::unordered_map<std::uint64_t, SheepServer> sheeps;
	std::unordered_map<std::uint64_t, CowServer> cows;

};


struct EntityDataClient : public EntityGetter<EntityDataClient>
{
	std::unordered_map<std::uint64_t, PlayerClient> players;

	std::unordered_map<std::uint64_t, DroppedItemClient> droppedItems;
	std::unordered_map<std::uint64_t, ZombieClient> zombies;
	std::unordered_map<std::uint64_t, PigClient> pigs;
	std::unordered_map<std::uint64_t, CatClient> cats;
	std::unordered_map<std::uint64_t, GoblinClient> goblins;
	std::unordered_map<std::uint64_t, TrainingDummyClient> trainingDummy;
	std::unordered_map<std::uint64_t, ScareCrowClient> scareCrows;
	std::unordered_map<std::uint64_t, FishClient> fish;
	std::unordered_map<std::uint64_t, BeeClient> bees;
	std::unordered_map<std::uint64_t, QueenBeeClient> queenBees;
	std::unordered_map<std::uint64_t, SlimeClient> slime;
	std::unordered_map<std::uint64_t, CreeperClient> creepers;
	std::unordered_map<std::uint64_t, EnderlingClient> enderlings;
	std::unordered_map<std::uint64_t, SkeletonClient> skeletons;
	std::unordered_map<std::uint64_t, StoneGolemClient> stoneGolems;
	std::unordered_map<std::uint64_t, WolfClient> wolves;
	std::unordered_map<std::uint64_t, FoxClient> foxes;
	std::unordered_map<std::uint64_t, CrowClient> crows;
	std::unordered_map<std::uint64_t, ManateeClient> manatees;
	std::unordered_map<std::uint64_t, CaveSpiderClient> caveSpiders;
	std::unordered_map<std::uint64_t, CrystalBatClient> crystalBats;
	std::unordered_map<std::uint64_t, CapybaraChefClient> capybaraChefs;
	std::unordered_map<std::uint64_t, RiverGuardianClient> riverGuardians;
	std::unordered_map<std::uint64_t, TreeEntClient> treeEnts;
	std::unordered_map<std::uint64_t, NomadTraderClient> nomadTraders;
	std::unordered_map<std::uint64_t, MimicChestClient> mimicChests;
	std::unordered_map<std::uint64_t, LightFairyClient> lightFairies;
	std::unordered_map<std::uint64_t, ArmoredBoarClient> armoredBoars;
	std::unordered_map<std::uint64_t, SandSerpentClient> sandSerpents;
	std::unordered_map<std::uint64_t, MistGhostClient> mistGhosts;
	std::unordered_map<std::uint64_t, HermitCrabClient> hermitCrabs;
	std::unordered_map<std::uint64_t, HoneyBearClient> honeyBears;
	std::unordered_map<std::uint64_t, LavaSlugClient> lavaSlugs;
	std::unordered_map<std::uint64_t, CrystalSentinelClient> crystalSentinels;
	std::unordered_map<std::uint64_t, BlacksmithVillagerClient> blacksmithVillagers;
	std::unordered_map<std::uint64_t, HerbalistVillagerClient> herbalistVillagers;
	std::unordered_map<std::uint64_t, SkeletonPirateClient> skeletonPirates;
	std::unordered_map<std::uint64_t, JuvenileDragonClient> juvenileDragons;
	std::unordered_map<std::uint64_t, CrystalGolemClient> crystalGolems;
	std::unordered_map<std::uint64_t, HydraClient> hydras;
	std::unordered_map<std::uint64_t, SheepClient> sheeps;
	std::unordered_map<std::uint64_t, CowClient> cows;

};

bool canEntityBeHit(unsigned char entityType);