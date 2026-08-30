#pragma once
#include <Arduino.h>
#include <vector>

struct KnownSpell {
    String slug;
    String name;
    int level = 0;
    bool prepared = false;
};

class Spellbook {
public:
    std::vector<KnownSpell> known;

    // Slots per spell level 1..9 (index 0 unused, index 1..9 used).
    int slotsTotal[10] = {0};
    int slotsUsed[10] = {0};

    bool useSlot(int level) {
        if (level < 1 || level > 9) return false;
        if (slotsUsed[level] >= slotsTotal[level]) return false;
        slotsUsed[level]++;
        return true;
    }

    void restoreSlot(int level) {
        if (level < 1 || level > 9) return;
        if (slotsUsed[level] > 0) slotsUsed[level]--;
    }

    void longRest() {
        for (int i = 0; i < 10; i++) slotsUsed[i] = 0;
    }

    void addSpell(const KnownSpell &s) { known.push_back(s); }

    void removeSpell(size_t index) {
        if (index < known.size()) known.erase(known.begin() + index);
    }
};
