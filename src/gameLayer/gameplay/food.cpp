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

	if (type == apple)
	{ ret.allEffects[Effects::Saturated].timerMs = 40 * 1000; }

	if (type == applePie)
	{
		ret.allEffects[Effects::Saturated].timerMs = 60 * 1000;
	}

	if (type == regenerationPotion)
	{ ret.allEffects[Effects::Regeneration].timerMs = 8 * 60 * 1000; } //8 minutes of regeneration
	
	if(type == strawberry)
	{ 
		ret.allEffects[Effects::Regeneration].timerMs = 1 * 60 * 1000
			+ fruitEffectsLonger * (30 * 1000); 
		ret.allEffects[Effects::Saturated].timerMs = FRUIT_BASE_SATURATION;
	}

	if (type == poisonPotion)
	{ ret.allEffects[Effects::Poisoned].timerMs = 30 * 1000; } //30 secconds of poison

	if (type == shieldingPotion)
	{ ret.allEffects[Effects::Shielding].timerMs = 8 * 60 * 1000; } 


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

	if (type == apple)
	{
		rez += 25;

		if (hasFruitPeeler)
		{
			rez += 10;
		}
	}

	if (type == applePie)
	{
		rez += 50;
	}

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
