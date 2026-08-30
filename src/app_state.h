#pragma once
#include "data/bestiary_store.h"
#include "data/spell_store.h"
#include "data/item_catalog.h"
#include "model/character.h"
#include "model/inventory.h"
#include "model/spellbook.h"

// Single shared instance of every piece of application state, so UI screens
// don't have to pass pointers around. Initialized once in main.cpp::setup().
struct AppState {
    BestiaryStore bestiary;
    SpellStore spells;
    ItemCatalog items;

    Character character;
    Inventory inventory;
    Spellbook spellbook;

    bool dataLoaded = false;

    bool begin();     // mounts stores from SD, loads or creates the save file
    void persist();   // writes character/inventory/spellbook back to SD
};

extern AppState App;
