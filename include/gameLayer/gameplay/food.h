#pragma once
#include <gameplay/effects.h>
#include <gameplay/items.h>


Effects getItemEffects(Item &item, PlayerInventory &inventory);
int getItemHealing(Item &item, PlayerInventory &inventory);
float getItemHungerRestoration(Item &item);
float getItemThirstRestoration(Item &item);




