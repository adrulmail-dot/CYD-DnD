#pragma once
#include <Arduino.h>
#include <vector>

struct BestiaryIndexEntry {
    String slug;
    String name;
    String type;
    String cr;
    String size;
    String img; // illustration slug, "" if none
};

struct StatBlock {
    String name;
    String type, subtype, size, alignment;
    int ac = 0;
    String armorDesc;
    int hp = 0;
    String hitDice;
    String speed;
    int str = 10, dex = 10, con = 10, intel = 10, wis = 10, cha = 10;
    String senses, languages, cr;
    int xp = 0;
    String vulnerabilities, resistances, immunities, conditionImmunities;
    String legendaryDesc;
    String img;
    String savesText;  // e.g. "Dex +5, Con +7"
    String skillsText; // e.g. "Perception +9, Stealth +3"

    struct Entry { String name, desc; };
    std::vector<Entry> traits, actions, bonusActions, reactions, legendary;
    std::vector<String> tips;
};

class BestiaryStore {
public:
    bool begin(); // loads bestiary_index.jsonl into RAM
    size_t count() const { return index_.size(); }
    const BestiaryIndexEntry &at(size_t i) const { return index_[i]; }

    // Returns indices into the index matching the (case-insensitive)
    // substring `query`, optionally restricted to `typeFilter` (empty = any).
    std::vector<size_t> search(const String &query, const String &typeFilter) const;

    // Loads the full stat block for a monster by slug. Returns true on success.
    bool loadDetail(const String &slug, StatBlock &out) const;

private:
    std::vector<BestiaryIndexEntry> index_;
};
