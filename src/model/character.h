#pragma once
#include <Arduino.h>

// Full 5e-style character sheet. Deliberately flat (no inheritance) so it
// can be serialized to/from JSON in one pass.
struct Character {
    // Identity
    String name = "New Hero";
    String playerName = "";
    String race = "";
    String charClass = "";
    String background = "";
    String alignment = "";
    int level = 1;

    // Abilities (3-30)
    int str = 10, dex = 10, con = 10, intel = 10, wis = 10, cha = 10;

    // Combat
    int maxHp = 10;
    int curHp = 10;
    int tempHp = 0;
    int armorClass = 10;
    int speed = 30;
    int initiativeBonus = 0;
    int proficiencyBonus = 2;
    int hitDiceTotal = 1;
    int hitDiceUsed = 0;
    String hitDiceType = "d8";

    // Saving throw / skill proficiency flags (bit N = ability/skill proficient)
    bool saveProf[6] = {false, false, false, false, false, false}; // str,dex,con,int,wis,cha

    // 18 standard 5e skills, proficiency + expertise flags
    static const int SKILL_COUNT = 18;
    bool skillProf[SKILL_COUNT] = {false};
    bool skillExpertise[SKILL_COUNT] = {false};

    // Currency (copper/silver/electrum/gold/platinum)
    long cp = 0, sp = 0, ep = 0, gp = 0, pp = 0;

    // Free-form
    String notes = "";
    String personalityTraits = "";
    String features = "";

    static int mod(int score) {
        int diff = score - 10;
        // floor division that works for negatives
        return (diff >= 0) ? diff / 2 : -((-diff + 1) / 2);
    }
};

extern const char *SKILL_NAMES[Character::SKILL_COUNT];
extern const int SKILL_ABILITY[Character::SKILL_COUNT]; // index into 0..5 = str,dex,con,int,wis,cha
