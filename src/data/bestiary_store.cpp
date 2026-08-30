#include "bestiary_store.h"
#include "sd_json.h"
#include <ArduinoJson.h>

static const char *kIndexPath = "/bestiary_index.jsonl";
static const char *kDetailDir = "/bestiary/";

bool BestiaryStore::begin() {
    index_.clear();
    index_.reserve(340);
    return sdjson::forEachLine(kIndexPath, 256, [this](JsonObjectConst o) {
        BestiaryIndexEntry e;
        e.slug = String(o["s"].as<const char *>());
        e.name = String(o["n"].as<const char *>());
        e.type = String(o["ty"] | "");
        e.cr = String(o["cr"] | "");
        e.size = String(o["sz"] | "");
        e.img = String(o["img"] | "");
        index_.push_back(e);
    });
}

std::vector<size_t> BestiaryStore::search(const String &query, const String &typeFilter) const {
    std::vector<size_t> out;
    String q = query;
    q.toLowerCase();
    for (size_t i = 0; i < index_.size(); i++) {
        if (typeFilter.length() && index_[i].type != typeFilter) continue;
        if (q.length()) {
            String n = index_[i].name;
            n.toLowerCase();
            if (n.indexOf(q) < 0) continue;
        }
        out.push_back(i);
    }
    return out;
}

static void readEntries(JsonArrayConst arr, std::vector<StatBlock::Entry> &out) {
    for (JsonObjectConst o : arr) {
        StatBlock::Entry e;
        e.name = String(o["n"] | "");
        e.desc = String(o["d"] | "");
        out.push_back(e);
    }
}

bool BestiaryStore::loadDetail(const String &slug, StatBlock &out) const {
    String path = String(kDetailDir) + slug + ".json";
    DynamicJsonDocument doc(24576);
    char *buf = nullptr;
    if (!sdjson::loadInto(path, doc, buf)) return false;

    out.name = String(doc["n"] | "");
    out.type = String(doc["ty"] | "");
    out.subtype = String(doc["sub"] | "");
    out.size = String(doc["sz"] | "");
    out.alignment = String(doc["al"] | "");
    out.ac = doc["ac"] | 10;
    out.armorDesc = String(doc["acd"] | "");
    out.hp = doc["hp"] | 1;
    out.hitDice = String(doc["hd"] | "");
    out.speed = String(doc["sp"] | "");
    out.str = doc["str"] | 10;
    out.dex = doc["dex"] | 10;
    out.con = doc["con"] | 10;
    out.intel = doc["int"] | 10;
    out.wis = doc["wis"] | 10;
    out.cha = doc["cha"] | 10;
    out.senses = String(doc["senses"] | "");
    out.languages = String(doc["langs"] | "");
    out.cr = String(doc["cr"] | "");
    out.vulnerabilities = String(doc["vuln"] | "");
    out.resistances = String(doc["res"] | "");
    out.immunities = String(doc["imm"] | "");
    out.conditionImmunities = String(doc["cimm"] | "");
    out.legendaryDesc = String(doc["legendary_desc"] | "");
    out.img = String(doc["img"] | "");

    out.savesText = "";
    for (JsonPairConst kv : doc["saves"].as<JsonObjectConst>()) {
        if (out.savesText.length()) out.savesText += ", ";
        int v = kv.value().as<int>();
        out.savesText += String(kv.key().c_str()) + " " + (v >= 0 ? "+" : "") + String(v);
    }
    out.skillsText = "";
    for (JsonPairConst kv : doc["skills"].as<JsonObjectConst>()) {
        if (out.skillsText.length()) out.skillsText += ", ";
        int v = kv.value().as<int>();
        out.skillsText += String(kv.key().c_str()) + " " + (v >= 0 ? "+" : "") + String(v);
    }

    out.traits.clear(); out.actions.clear(); out.bonusActions.clear();
    out.reactions.clear(); out.legendary.clear(); out.tips.clear();
    readEntries(doc["traits"].as<JsonArrayConst>(), out.traits);
    readEntries(doc["actions"].as<JsonArrayConst>(), out.actions);
    readEntries(doc["bonus"].as<JsonArrayConst>(), out.bonusActions);
    readEntries(doc["reactions"].as<JsonArrayConst>(), out.reactions);
    readEntries(doc["legendary"].as<JsonArrayConst>(), out.legendary);
    for (JsonVariantConst v : doc["tips"].as<JsonArrayConst>()) {
        out.tips.push_back(String(v.as<const char *>()));
    }

    delete[] buf;
    return true;
}
