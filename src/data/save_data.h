#pragma once
#include "../model/character.h"
#include "../model/inventory.h"
#include "../model/spellbook.h"

// Persists the player's character sheet, inventory and spellbook to a single
// JSON save file on the microSD card so progress survives a power cycle.
namespace savedata {

static const char *kSavePath = "/save/character.json";

bool save(const Character &c, const Inventory &inv, const Spellbook &sb);
bool load(Character &c, Inventory &inv, Spellbook &sb);

} // namespace savedata
