#pragma once
#include <Arduino.h>
#include <vector>

struct WeaponDef {
    String slug, name, category, cost, damage, damageType, weight;
    std::vector<String> properties;
};

struct ArmorDef {
    String slug, name, category, cost, weight;
    int baseAc = 10;
    bool dexMod = false;
    int dexMax = -1; // -1 = unlimited
    int strReq = 0;
    bool stealthDisadvantage = false;
};

struct MagicItemIndexEntry {
    String slug, name, rarity;
};

struct MagicItemDetail {
    String name, type, rarity, attune, desc;
};

// Loads the small SRD equipment catalog used when adding items to the
// inventory. Weapons/armor are few enough to keep fully in RAM; magic
// items (237 entries with long descriptions) stay as an index + on-demand
// per-item file, same pattern as the bestiary/spell stores.
class ItemCatalog {
public:
    bool begin();

    const std::vector<WeaponDef> &weapons() const { return weapons_; }
    const std::vector<ArmorDef> &armor() const { return armor_; }

    size_t magicCount() const { return magicIndex_.size(); }
    const MagicItemIndexEntry &magicAt(size_t i) const { return magicIndex_[i]; }
    std::vector<size_t> magicSearch(const String &query) const;
    bool loadMagicDetail(const String &slug, MagicItemDetail &out) const;

private:
    std::vector<WeaponDef> weapons_;
    std::vector<ArmorDef> armor_;
    std::vector<MagicItemIndexEntry> magicIndex_;
};
