#include "item_catalog.h"
#include "sd_json.h"
#include <ArduinoJson.h>

bool ItemCatalog::begin() {
    weapons_.clear();
    armor_.clear();
    magicIndex_.clear();

    {
        DynamicJsonDocument doc(16384);
        char *buf = nullptr;
        if (sdjson::loadInto("/items/weapons.json", doc, buf)) {
            for (JsonObjectConst o : doc.as<JsonArrayConst>()) {
                WeaponDef w;
                w.slug = String(o["s"] | "");
                w.name = String(o["n"] | "");
                w.category = String(o["cat"] | "");
                w.cost = String(o["cost"] | "");
                w.damage = String(o["dmg"] | "");
                w.damageType = String(o["dmg_type"] | "");
                w.weight = String(o["weight"] | "");
                for (JsonVariantConst p : o["props"].as<JsonArrayConst>()) {
                    w.properties.push_back(String(p.as<const char *>()));
                }
                weapons_.push_back(w);
            }
            delete[] buf;
        }
    }

    {
        DynamicJsonDocument doc(8192);
        char *buf = nullptr;
        if (sdjson::loadInto("/items/armor.json", doc, buf)) {
            for (JsonObjectConst o : doc.as<JsonArrayConst>()) {
                ArmorDef a;
                a.slug = String(o["s"] | "");
                a.name = String(o["n"] | "");
                a.category = String(o["cat"] | "");
                a.cost = String(o["cost"] | "");
                a.weight = String(o["weight"] | "");
                a.baseAc = o["base_ac"] | 10;
                a.dexMod = o["dex_mod"] | false;
                a.dexMax = o["dex_max"].isNull() ? -1 : o["dex_max"].as<int>();
                a.strReq = o["str_req"] | 0;
                a.stealthDisadvantage = o["stealth_dis"] | false;
                armor_.push_back(a);
            }
            delete[] buf;
        }
    }

    return sdjson::forEachLine("/items/magic_index.jsonl", 160, [this](JsonObjectConst o) {
        MagicItemIndexEntry e;
        e.slug = String(o["s"].as<const char *>());
        e.name = String(o["n"].as<const char *>());
        e.rarity = String(o["rarity"] | "");
        magicIndex_.push_back(e);
    });
}

std::vector<size_t> ItemCatalog::magicSearch(const String &query) const {
    std::vector<size_t> out;
    String q = query;
    q.toLowerCase();
    for (size_t i = 0; i < magicIndex_.size(); i++) {
        if (q.length()) {
            String n = magicIndex_[i].name;
            n.toLowerCase();
            if (n.indexOf(q) < 0) continue;
        }
        out.push_back(i);
    }
    return out;
}

bool ItemCatalog::loadMagicDetail(const String &slug, MagicItemDetail &out) const {
    String path = "/items/magic/" + slug + ".json";
    DynamicJsonDocument doc(4096);
    char *buf = nullptr;
    if (!sdjson::loadInto(path, doc, buf)) return false;
    out.name = String(doc["n"] | "");
    out.type = String(doc["type"] | "");
    out.rarity = String(doc["rarity"] | "");
    out.attune = String(doc["attune"] | "");
    out.desc = String(doc["desc"] | "");
    delete[] buf;
    return true;
}
