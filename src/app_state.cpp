#include "app_state.h"
#include "data/save_data.h"
#include <SD.h>

AppState App;

bool AppState::begin() {
    bool ok = true;
    ok &= bestiary.begin();
    ok &= spells.begin();
    ok &= items.begin();

    if (!savedata::load(character, inventory, spellbook)) {
        // No save file yet - start with sane defaults for a fresh level 1 hero.
        character = Character();
        spellbook.slotsTotal[1] = 2; // default: a couple of 1st level slots
    }

    dataLoaded = ok;
    return ok;
}

void AppState::persist() {
    savedata::save(character, inventory, spellbook);
}
