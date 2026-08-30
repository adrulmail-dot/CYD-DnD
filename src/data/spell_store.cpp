#include "spell_store.h"
#include "sd_json.h"
#include <ArduinoJson.h>

static const char *kIndexPath = "/spells_index.jsonl";
static const char *kDetailDir = "/spells/";

bool SpellStore::begin() {
    index_.clear();
    index_.reserve(330);
    return sdjson::forEachLine(kIndexPath, 200, [this](JsonObjectConst o) {
        SpellIndexEntry e;
        e.slug = String(o["s"].as<const char *>());
        e.name = String(o["n"].as<const char *>());
        e.level = o["lvl"] | 0;
        e.school = String(o["school"] | "");
        e.classes = String(o["classes"] | "");
        index_.push_back(e);
    });
}

std::vector<size_t> SpellStore::search(const String &query, int level) const {
    std::vector<size_t> out;
    String q = query;
    q.toLowerCase();
    for (size_t i = 0; i < index_.size(); i++) {
        if (level >= 0 && index_[i].level != level) continue;
        if (q.length()) {
            String n = index_[i].name;
            n.toLowerCase();
            if (n.indexOf(q) < 0) continue;
        }
        out.push_back(i);
    }
    return out;
}

bool SpellStore::loadDetail(const String &slug, SpellDetail &out) const {
    String path = String(kDetailDir) + slug + ".json";
    DynamicJsonDocument doc(8192);
    char *buf = nullptr;
    if (!sdjson::loadInto(path, doc, buf)) return false;

    out.name = String(doc["n"] | "");
    out.level = doc["lvl"] | 0;
    out.school = String(doc["school"] | "");
    out.castingTime = String(doc["cast"] | "");
    out.range = String(doc["range"] | "");
    out.duration = String(doc["dur"] | "");
    JsonObjectConst comp = doc["comp"];
    out.verbal = comp["v"] | false;
    out.somatic = comp["s"] | false;
    out.material = comp["m"] | false;
    out.materialDesc = String(comp["mat"] | "");
    out.concentration = doc["conc"] | false;
    out.ritual = doc["ritual"] | false;
    out.classes = String(doc["classes"] | "");
    out.desc = String(doc["desc"] | "");
    out.higherLevel = String(doc["higher"] | "");

    delete[] buf;
    return true;
}
