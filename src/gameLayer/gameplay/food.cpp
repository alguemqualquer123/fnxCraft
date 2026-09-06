#include <gameplay/food.h>
#include <tickTimer.h>


Effects getItemEffects(Item &item, PlayerInventory &inventory)
{
	auto type = item.type;

	bool fruitEffectsLonger = 0;

	for (int i = PlayerInventory::EQUIPEMENT_START_INDEX; i < PlayerInventory::EQUIPEMENT_START_INDEX +
		PlayerInventory::MAX_EQUIPEMENT_SLOTS; i++)
	{
		auto item = inventory.getItemFromIndex(i, nullptr);
		if (item->type == fruitPeeler) { fruitEffectsLonger = 1; }
	}

	Effects ret;

	int FRUIT_BASE_SATURATION = 15 * 1000;

	// Original items
	if (type == apple)
	{ ret.allEffects[Effects::Saturated].timerMs = 40 * 1000; }

	if (type == applePie)
	{
		ret.allEffects[Effects::Saturated].timerMs = 60 * 1000;
	}

	// Fruits (saturation + some have regen)
	if(type == strawberry)
	{ 
		ret.allEffects[Effects::Regeneration].timerMs = 1 * 60 * 1000 + fruitEffectsLonger * (30 * 1000); 
		ret.allEffects[Effects::Saturated].timerMs = FRUIT_BASE_SATURATION;
	}
	if (type == blackBerrie)  { ret.allEffects[Effects::Saturated].timerMs = 10 * 1000; }
	if (type == blueBerrie)   { ret.allEffects[Effects::Saturated].timerMs = 10 * 1000; }
	if (type == cherries)     { ret.allEffects[Effects::Saturated].timerMs = 15 * 1000; }
	if (type == peach)        { ret.allEffects[Effects::Saturated].timerMs = 15 * 1000; }
	if (type == pinapple)     { ret.allEffects[Effects::Saturated].timerMs = 20 * 1000; }
	if (type == grapes)       { ret.allEffects[Effects::Saturated].timerMs = 12 * 1000; }
	if (type == lime)         { ret.allEffects[Effects::Saturated].timerMs = 10 * 1000; }
	if (type == cocconut)     { ret.allEffects[Effects::Saturated].timerMs = 15 * 1000; }
	if (type == chilliPepper) { ret.allEffects[Effects::Saturated].timerMs = 8 * 1000; }

	// Cooked food (more saturation)
	if (type == cookedMeat)    { ret.allEffects[Effects::Saturated].timerMs = 50 * 1000; }
	if (type == bread)         { ret.allEffects[Effects::Saturated].timerMs = 35 * 1000; }
	if (type == stew)          { ret.allEffects[Effects::Saturated].timerMs = 60 * 1000; }
	if (type == bakedPotato)   { ret.allEffects[Effects::Saturated].timerMs = 35 * 1000; }
	if (type == roastedCorn)   { ret.allEffects[Effects::Saturated].timerMs = 25 * 1000; }
	if (type == cheese)        { ret.allEffects[Effects::Saturated].timerMs = 20 * 1000; }
	if (type == cookedChicken) { ret.allEffects[Effects::Saturated].timerMs = 45 * 1000; }
	if (type == chickenSoup)   { ret.allEffects[Effects::Saturated].timerMs = 55 * 1000; }
	if (type == cookedFish)    { ret.allEffects[Effects::Saturated].timerMs = 45 * 1000; }

	// Raw food (less saturation)
	if (type == rawMeat)       { ret.allEffects[Effects::Saturated].timerMs = 12 * 1000; }
	if (type == rawFish)       { ret.allEffects[Effects::Saturated].timerMs = 10 * 1000; }

	// Drinks (no saturation, but some have effects)
	if (type == waterBottle)   { } // just thirst
	if (type == juice)         { ret.allEffects[Effects::Regeneration].timerMs = 5000; }
	if (type == milk)          { ret.allEffects[Effects::Regeneration].timerMs = 8000; }
	if (type == coffee)        { ret.allEffects[Effects::Regeneration].timerMs = 5000; }
	if (type == tea)           { ret.allEffects[Effects::Regeneration].timerMs = 10000; }

	// Potions
	if (type == regenerationPotion)
	{ ret.allEffects[Effects::Regeneration].timerMs = 8 * 60 * 1000; } //8 minutes of regeneration

	if (type == poisonPotion)
	{ ret.allEffects[Effects::Poisoned].timerMs = 30 * 1000; } //30 secconds of poison

	if (type == shieldingPotion)
	{ ret.allEffects[Effects::Shielding].timerMs = 8 * 60 * 1000; }

	if (type == healingPotion)
	{ } // handled by getItemHealing

	if (type == manaPotion)
	{ } // mana handled elsewhere

	if (type == fireResistancePotion)
	{ ret.allEffects[Effects::Shielding].timerMs = 3 * 60 * 1000; }

	if (type == jumpBoostPotion)
	{ ret.allEffects[Effects::Regeneration].timerMs = 10000; }

	if (type == speedPotion)
	{ ret.allEffects[Effects::Regeneration].timerMs = 15000; }

	if (type == strengthPotion)
	{ ret.allEffects[Effects::Regeneration].timerMs = 20000; }

	if (type == stealthPotion)
	{ ret.allEffects[Effects::Shielding].timerMs = 60000; } 


	for (int i = PlayerInventory::EQUIPEMENT_START_INDEX; i < PlayerInventory::EQUIPEMENT_START_INDEX +
		PlayerInventory::MAX_EQUIPEMENT_SLOTS; i++)
	{
		auto item = inventory.getItemFromIndex(i, nullptr);

		if (item->type == ItemTypes::gumBox)
		{
			ret.allEffects[Effects::Saturated].timerMs -= 5 * 1000;
			ret.allEffects[Effects::Saturated].timerMs = std::max(ret.allEffects[Effects::Saturated].timerMs, 0);
		}

	}


	return ret;
}

int getItemHealing(Item &item, PlayerInventory &inventory)
{
	auto type = item.type;

	int rez = 0;
	bool hasFruitPeeler = 0;

	for (int i = PlayerInventory::EQUIPEMENT_START_INDEX; i < PlayerInventory::EQUIPEMENT_START_INDEX +
		PlayerInventory::MAX_EQUIPEMENT_SLOTS; i++)
	{
		auto item = inventory.getItemFromIndex(i, nullptr);

		if (item->type == ItemTypes::vitamins)
		{
			rez += 15;
		}

		if (item->type == fruitPeeler) { hasFruitPeeler = 1; }


	}

	// Original items
	if (type == apple)             { rez += 25; if (hasFruitPeeler) rez += 10; }	if (type == applePie)
	{
		rez += 50;
	}

	// Potions with healing
	if (type == healingPotion)
	{
		rez += 80;
	}

	if (type == regenerationPotion)
	{
		rez += 40;
	}

	// Fruits
	if (type == strawberry)        { rez += 8;  if (hasFruitPeeler) rez += 3; }
	if (type == blackBerrie)       { rez += 5;  if (hasFruitPeeler) rez += 2; }
	if (type == blueBerrie)        { rez += 5;  if (hasFruitPeeler) rez += 2; }
	if (type == cherries)          { rez += 10; if (hasFruitPeeler) rez += 4; }
	if (type == peach)             { rez += 10; if (hasFruitPeeler) rez += 4; }
	if (type == pinapple)          { rez += 12; if (hasFruitPeeler) rez += 5; }
	if (type == grapes)            { rez += 8;  if (hasFruitPeeler) rez += 3; }
	if (type == lime)              { rez += 5;  if (hasFruitPeeler) rez += 2; }
	if (type == cocconut)          { rez += 10; if (hasFruitPeeler) rez += 4; }

	// Cooked food
	if (type == cookedMeat)        { rez += 35; }
	if (type == bread)             { rez += 20; }
	if (type == stew)              { rez += 45; }
	if (type == bakedPotato)       { rez += 20; }
	if (type == roastedCorn)       { rez += 15; }
	if (type == cheese)            { rez += 12; }
	if (type == cookedChicken)     { rez += 30; }
	if (type == chickenSoup)       { rez += 40; }
	if (type == cookedFish)        { rez += 30; }

	// Raw food (less healing)
	if (type == rawMeat)           { rez += 5; }
	if (type == rawFish)           { rez += 3; }

	return std::max(0, rez);
}

// Returns how much hunger this food restores
float getItemHungerRestoration(Item &item)
{
	auto type = item.type;

	// Raw food: less hunger restored
	if (type == apple)             { return 20.f; }
	if (type == applePie)          { return 40.f; }
	if (type == strawberry)        { return 8.f; }
	if (type == blackBerrie)       { return 6.f; }
	if (type == blueBerrie)        { return 6.f; }
	if (type == cherries)          { return 10.f; }
	if (type == peach)             { return 10.f; }
	if (type == pinapple)          { return 12.f; }
	if (type == grapes)            { return 8.f; }
	if (type == lime)              { return 6.f; }
	if (type == cocconut)          { return 10.f; }

	// Cooked food: more hunger restored
	if (type == bread)             { return 25.f; }
	if (type == cookedMeat)        { return 35.f; }
	if (type == rawMeat)           { return 10.f; }
	if (type == stew)              { return 45.f; }
	if (type == bakedPotato)       { return 25.f; }
	if (type == roastedCorn)       { return 20.f; }
	if (type == cheese)            { return 15.f; }
	if (type == cookedChicken)     { return 30.f; }
	if (type == chickenSoup)       { return 40.f; }
	if (type == cookedFish)        { return 30.f; }
	if (type == rawFish)           { return 8.f; }

	return 0.f;
}

// Returns how much thirst this drink restores
float getItemThirstRestoration(Item &item)
{
	auto type = item.type;

	if (type == waterBottle)       { return 40.f; }
	if (type == juice)             { return 30.f; }
	if (type == milk)              { return 35.f; }
	if (type == coffee)            { return 25.f; }
	if (type == tea)               { return 30.f; }

	return 0.f;
}
