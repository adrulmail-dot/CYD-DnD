#pragma once
#include <Arduino.h>
#include <vector>

struct SpellIndexEntry {
    String slug;
    String name;
    int level = 0;
    String school;
    String classes;
};

struct SpellDetail {
    String name;
    int level = 0;
    String school;
    String castingTime, range, duration;
    bool verbal = false, somatic = false, material = false;
    String materialDesc;
    bool concentration = false, ritual = false;
    String classes;
    String desc;
    String higherLevel;
};

class SpellStore {
public:
    bool begin();
    size_t count() const { return index_.size(); }
    const SpellIndexEntry &at(size_t i) const { return index_[i]; }

    // level = -1 means "any level"
    std::vector<size_t> search(const String &query, int level) const;

    bool loadDetail(const String &slug, SpellDetail &out) const;

private:
    std::vector<SpellIndexEntry> index_;
};
