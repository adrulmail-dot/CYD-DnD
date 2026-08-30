#include "save_data.h"
#include "sd_json.h"
#include <SD.h>
#include <ArduinoJson.h>

namespace savedata {

bool save(const Character &c, const Inventory &inv, const Spellbook &sb) {
    DynamicJsonDocument doc(16384);

    JsonObject ch = doc.createNestedObject("character");
    ch["name"] = c.name;
    ch["playerName"] = c.playerName;
    ch["race"] = c.race;
    ch["class"] = c.charClass;
    ch["background"] = c.background;
    ch["alignment"] = c.alignment;
    ch["level"] = c.level;
    ch["str"] = c.str; ch["dex"] = c.dex; ch["con"] = c.con;
    ch["int"] = c.intel; ch["wis"] = c.wis; ch["cha"] = c.cha;
    ch["maxHp"] = c.maxHp; ch["curHp"] = c.curHp; ch["tempHp"] = c.tempHp;
    ch["ac"] = c.armorClass; ch["speed"] = c.speed;
    ch["initBonus"] = c.initiativeBonus;
    ch["profBonus"] = c.proficiencyBonus;
    ch["hitDiceTotal"] = c.hitDiceTotal; ch["hitDiceUsed"] = c.hitDiceUsed;
    ch["hitDiceType"] = c.hitDiceType;
    JsonArray saveProf = ch.createNestedArray("saveProf");
    for (int i = 0; i < 6; i++) saveProf.add(c.saveProf[i]);
    JsonArray skillProf = ch.createNestedArray("skillProf");
    JsonArray skillExp = ch.createNestedArray("skillExpertise");
    for (int i = 0; i < Character::SKILL_COUNT; i++) {
        skillProf.add(c.skillProf[i]);
        skillExp.add(c.skillExpertise[i]);
    }
    ch["cp"] = c.cp; ch["sp"] = c.sp; ch["ep"] = c.ep; ch["gp"] = c.gp; ch["pp"] = c.pp;
    ch["notes"] = c.notes;
    ch["personality"] = c.personalityTraits;
    ch["features"] = c.features;

    JsonArray items = doc.createNestedArray("inventory");
    for (const auto &it : inv.items) {
        JsonObject o = items.createNestedObject();
        o["name"] = it.name;
        o["slug"] = it.catalogSlug;
        o["kind"] = it.kind;
        o["weight"] = it.weight;
        o["qty"] = it.quantity;
        o["equipped"] = it.equipped;
        o["notes"] = it.notes;
    }

    JsonObject spellbook = doc.createNestedObject("spellbook");
    JsonArray slotsTotal = spellbook.createNestedArray("slotsTotal");
    JsonArray slotsUsed = spellbook.createNestedArray("slotsUsed");
    for (int i = 0; i < 10; i++) {
        slotsTotal.add(sb.slotsTotal[i]);
        slotsUsed.add(sb.slotsUsed[i]);
    }
    JsonArray known = spellbook.createNestedArray("known");
    for (const auto &s : sb.known) {
        JsonObject o = known.createNestedObject();
        o["slug"] = s.slug;
        o["name"] = s.name;
        o["level"] = s.level;
        o["prepared"] = s.prepared;
    }

    if (!SD.exists("/save")) SD.mkdir("/save");
    File f = SD.open(kSavePath, FILE_WRITE);
    if (!f) return false;
    serializeJson(doc, f);
    f.close();
    return true;
}

bool load(Character &c, Inventory &inv, Spellbook &sb) {
    DynamicJsonDocument doc(16384);
    char *buf = nullptr;
    if (!sdjson::loadInto(kSavePath, doc, buf)) return false;

    JsonObjectConst ch = doc["character"];
    c.name = String(ch["name"] | "New Hero");
    c.playerName = String(ch["playerName"] | "");
    c.race = String(ch["race"] | "");
    c.charClass = String(ch["class"] | "");
    c.background = String(ch["background"] | "");
    c.alignment = String(ch["alignment"] | "");
    c.level = ch["level"] | 1;
    c.str = ch["str"] | 10; c.dex = ch["dex"] | 10; c.con = ch["con"] | 10;
    c.intel = ch["int"] | 10; c.wis = ch["wis"] | 10; c.cha = ch["cha"] | 10;
    c.maxHp = ch["maxHp"] | 10; c.curHp = ch["curHp"] | 10; c.tempHp = ch["tempHp"] | 0;
    c.armorClass = ch["ac"] | 10; c.speed = ch["speed"] | 30;
    c.initiativeBonus = ch["initBonus"] | 0;
    c.proficiencyBonus = ch["profBonus"] | 2;
    c.hitDiceTotal = ch["hitDiceTotal"] | 1; c.hitDiceUsed = ch["hitDiceUsed"] | 0;
    c.hitDiceType = String(ch["hitDiceType"] | "d8");
    JsonArrayConst saveProf = ch["saveProf"];
    for (int i = 0; i < 6 && i < (int)saveProf.size(); i++) c.saveProf[i] = saveProf[i];
    JsonArrayConst skillProf = ch["skillProf"];
    JsonArrayConst skillExp = ch["skillExpertise"];
    for (int i = 0; i < Character::SKILL_COUNT; i++) {
        if (i < (int)skillProf.size()) c.skillProf[i] = skillProf[i];
        if (i < (int)skillExp.size()) c.skillExpertise[i] = skillExp[i];
    }
    c.cp = ch["cp"] | 0; c.sp = ch["sp"] | 0; c.ep = ch["ep"] | 0;
    c.gp = ch["gp"] | 0; c.pp = ch["pp"] | 0;
    c.notes = String(ch["notes"] | "");
    c.personalityTraits = String(ch["personality"] | "");
    c.features = String(ch["features"] | "");

    inv.items.clear();
    for (JsonObjectConst o : doc["inventory"].as<JsonArrayConst>()) {
        InventoryItem it;
        it.name = String(o["name"] | "");
        it.catalogSlug = String(o["slug"] | "");
        it.kind = String(o["kind"] | "");
        it.weight = o["weight"] | 0.0f;
        it.quantity = o["qty"] | 1;
        it.equipped = o["equipped"] | false;
        it.notes = String(o["notes"] | "");
        inv.items.push_back(it);
    }

    JsonObjectConst spellbook = doc["spellbook"];
    JsonArrayConst slotsTotal = spellbook["slotsTotal"];
    JsonArrayConst slotsUsed = spellbook["slotsUsed"];
    for (int i = 0; i < 10; i++) {
        if (i < (int)slotsTotal.size()) sb.slotsTotal[i] = slotsTotal[i];
        if (i < (int)slotsUsed.size()) sb.slotsUsed[i] = slotsUsed[i];
    }
    sb.known.clear();
    for (JsonObjectConst o : spellbook["known"].as<JsonArrayConst>()) {
        KnownSpell s;
        s.slug = String(o["slug"] | "");
        s.name = String(o["name"] | "");
        s.level = o["level"] | 0;
        s.prepared = o["prepared"] | false;
        sb.known.push_back(s);
    }

    delete[] buf;
    return true;
}

} // namespace savedata
