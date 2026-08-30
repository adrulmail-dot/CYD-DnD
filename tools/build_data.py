#!/usr/bin/env python3
"""
Convert open5e-api SRD JSON fixtures into a compact, on-device friendly
data set for the CYD D&D Bestiary & Spell Tracker firmware.

Source: open5e/open5e-api, data/v1/wotc-srd/*.json (5.1 SRD, Open Gaming
License content) plus the 12 CC BY-NC 4.0 open5e-illustrations monster
images bundled in that repo's static/ folder.

Output: ./sd_card_data/  -- copy this whole folder to the root of the
microSD card used by the device.

Usage:
    python3 tools/build_data.py /path/to/open5e-api/checkout
"""
import json
import os
import re
import sys
import shutil

try:
    from PIL import Image
except ImportError:
    Image = None

ABILS = ["strength", "dexterity", "constitution", "intelligence", "wisdom", "charisma"]
ABIL_SHORT = {"strength": "str", "dexterity": "dex", "constitution": "con",
              "intelligence": "int", "wisdom": "wis", "charisma": "cha"}
ABIL_RU = {"strength": "Силу", "dexterity": "Ловкость", "constitution": "Телосложение",
           "intelligence": "Интеллект", "wisdom": "Мудрость", "charisma": "Харизму"}


def load(repo, name):
    path = os.path.join(repo, "data", "v1", "wotc-srd", name)
    with open(path, encoding="utf-8") as f:
        return json.load(f)


def jloads(s, default=None):
    if not s or s == "null":
        return default
    try:
        return json.loads(s)
    except (json.JSONDecodeError, TypeError):
        return default


def ability_mod(score):
    return (score - 10) // 2 if score is not None else 0


def fmt_speed(speed_json):
    d = jloads(speed_json, {})
    if not isinstance(d, dict):
        return ""
    parts = []
    order = ["walk", "fly", "swim", "climb", "burrow"]
    for k in order:
        if k in d and d[k]:
            label = {"walk": "ходьба", "fly": "полёт", "swim": "плавание",
                     "climb": "лазание", "burrow": "рытьё"}[k]
            parts.append(f"{label} {d[k]} фт.")
    for k, v in d.items():
        if k not in order and k != "hover" and v:
            parts.append(f"{k} {v} фт.")
    if d.get("hover"):
        parts.append("(парит)")
    return ", ".join(parts) if parts else "0 фт."


def gen_tips(f):
    """Original, algorithmically derived tactical hints - not copied from
    any book. Based only on structured stat fields."""
    tips = []

    vuln = (f.get("damage_vulnerabilities") or "").strip()
    if vuln:
        tips.append(f"Уязвимость к урону: {vuln} — этот тип урона наносит двойной эффект, применяйте в первую очередь.")

    imm = (f.get("damage_immunities") or "").strip()
    if imm:
        tips.append(f"Иммунитет к урону: {imm} — не тратьте на это ресурсы, ищите другой тип урона.")

    res = (f.get("damage_resistances") or "").strip()
    if res:
        tips.append(f"Сопротивление урону: {res} — вполовину менее эффективно, предпочтите другой тип.")

    cimm = (f.get("condition_immunities") or "").strip()
    if cimm:
        tips.append(f"Иммунитет к состояниям: {cimm} — заклинания/эффекты с такими состояниями не подействуют.")

    # weakest saving throw
    mods = {}
    for a in ABILS:
        explicit = f.get(f"{ABIL_SHORT[a]}_save")
        score = f.get(a)
        mods[a] = explicit if explicit is not None else ability_mod(score)
    if mods:
        weakest_val = min(mods.values())
        weakest = [a for a, v in mods.items() if v == weakest_val]
        if weakest_val <= 1:
            names = " / ".join(ABIL_RU[a] for a in weakest)
            sign = "+" if weakest_val >= 0 else ""
            tips.append(f"Слабый спасбросок: {names} ({sign}{weakest_val}) — эффекты и заклинания на эту характеристику с высокой вероятностью пройдут.")

    strongest_val = max(mods.values()) if mods else 0
    strongest = [a for a, v in mods.items() if v == strongest_val]
    if strongest_val >= 7:
        names = " / ".join(ABIL_RU[a] for a in strongest)
        tips.append(f"Очень сильный спасбросок: {names} (+{strongest_val}) — не полагайтесь на эффекты по этой характеристике.")

    all_text_blobs = []
    for key in ("special_abilities_json", "actions_json", "legendary_actions_json", "reactions_json", "bonus_actions_json"):
        arr = jloads(f.get(key), [])
        if isinstance(arr, list):
            for item in arr:
                if isinstance(item, dict):
                    all_text_blobs.append((item.get("name") or "", item.get("desc") or ""))
    joined_lower = " ".join(f"{n} {d}" for n, d in all_text_blobs).lower()

    if "legendary resistance" in joined_lower:
        m = re.search(r"legendary resistance \((\d+)/day\)", joined_lower)
        n = m.group(1) if m else "несколько"
        tips.append(f"Легендарное сопротивление ({n}/день) — первые провалы спасбросков будут автоматически превращены в успех. Приберегите ключевые контролирующие заклинания на потом, либо сначала исчерпайте эти использования слабыми эффектами.")

    if "pack tactics" in joined_lower:
        tips.append("Тактика стаи — получает преимущество на атаку, если союзник рядом с целью. Не давайте себя окружать, разделяйте противников.")

    if "regenerat" in joined_lower:
        for n, d in all_text_blobs:
            if "regenerat" in n.lower() or "regenerat" in d.lower():
                dmg_match = re.search(r"unless (?:it|the \w+) takes (\w+) damage", d.lower())
                if dmg_match:
                    tips.append(f"Регенерация — восстанавливает ХП каждый раунд, если не получит урон типа «{dmg_match.group(1)}». Нанесите такой урон, чтобы остановить лечение.")
                else:
                    tips.append("Регенерация — восстанавливает ХП каждый раунд при определённых условиях (см. способность). Прочитайте условие подавления, иначе бой затянется.")
                break

    if "spellcasting" in joined_lower or jloads(f.get("spells_json")):
        tips.append("Заклинатель — по возможности используйте Немоту (Silence), прерывание заклинаний (Counterspell) или атаку в упор, чтобы сорвать компоненты.")

    if "multiattack" in joined_lower:
        tips.append("Мультиатака — наносит несколько ударов за ход. Эффекты обездвиживания/оглушения особенно ценны против него.")

    speed = jloads(f.get("speed_json"), {})
    if isinstance(speed, dict) and speed.get("fly"):
        tips.append("Имеет скорость полёта — учитывайте дальнобойные атаки и заклинания, у существа может не быть наземных ограничений.")

    size = (f.get("size") or "").lower()
    if size in ("huge", "gargantuan"):
        tips.append("Крупный размер — хорошая цель для заклинаний по области и провоцирующих атак, но часто высокий урон за удар.")

    try:
        cr = float(f.get("cr") or 0)
    except (TypeError, ValueError):
        cr = 0
    hp = f.get("hit_points") or 0
    if cr >= 1 and hp > 100 and hp / cr > 30:
        tips.append("Много здоровья относительно опасности — рассчитывайте на затяжной бой, экономьте ресурсы для поздних раундов.")

    if not tips:
        tips.append("Особых слабостей не выявлено — полагайтесь на стандартную тактику и следите за спасбросками в статблоке.")

    return tips


def slugify(name):
    s = name.lower()
    s = re.sub(r"[^a-z0-9]+", "-", s)
    return s.strip("-")


def build_monsters(repo, out_dir):
    data = load(repo, "Monster.json")
    bestiary_dir = os.path.join(out_dir, "bestiary")
    os.makedirs(bestiary_dir, exist_ok=True)
    index = []
    for entry in data:
        f = entry["fields"]
        slug = entry["pk"] or slugify(f["name"])
        img = f.get("img_main") or ""
        img_slug = os.path.splitext(os.path.basename(img))[0] if img else ""

        def actions_compact(key):
            arr = jloads(f.get(key), [])
            out = []
            if isinstance(arr, list):
                for item in arr:
                    if isinstance(item, dict):
                        out.append({"n": item.get("name", ""), "d": item.get("desc", "")})
            return out

        saves = {}
        for a in ABILS:
            v = f.get(f"{ABIL_SHORT[a]}_save")
            if v is not None:
                saves[ABIL_SHORT[a]] = v

        record = {
            "n": f["name"],
            "sz": f.get("size", ""),
            "ty": f.get("type", ""),
            "sub": f.get("subtype") or "",
            "al": f.get("alignment", ""),
            "ac": f.get("armor_class"),
            "acd": f.get("armor_desc") or "",
            "hp": f.get("hit_points"),
            "hd": f.get("hit_dice", ""),
            "sp": fmt_speed(f.get("speed_json")),
            "str": f.get("strength"), "dex": f.get("dexterity"), "con": f.get("constitution"),
            "int": f.get("intelligence"), "wis": f.get("wisdom"), "cha": f.get("charisma"),
            "saves": saves,
            "skills": jloads(f.get("skills_json"), {}),
            "vuln": f.get("damage_vulnerabilities") or "",
            "res": f.get("damage_resistances") or "",
            "imm": f.get("damage_immunities") or "",
            "cimm": f.get("condition_immunities") or "",
            "senses": f.get("senses") or "",
            "langs": f.get("languages") or "",
            "cr": f.get("challenge_rating", ""),
            "img": img_slug,
            "traits": actions_compact("special_abilities_json"),
            "actions": actions_compact("actions_json"),
            "bonus": actions_compact("bonus_actions_json"),
            "reactions": actions_compact("reactions_json"),
            "legendary": actions_compact("legendary_actions_json"),
            "legendary_desc": f.get("legendary_desc") or "",
            "tips": gen_tips(f),
        }
        with open(os.path.join(bestiary_dir, f"{slug}.json"), "w", encoding="utf-8") as out:
            json.dump(record, out, ensure_ascii=False, separators=(",", ":"))

        index.append({
            "s": slug, "n": f["name"], "ty": f.get("type", ""),
            "cr": f.get("challenge_rating", ""), "sz": f.get("size", ""),
            "img": img_slug,
        })

    index.sort(key=lambda r: r["n"])
    with open(os.path.join(out_dir, "bestiary_index.jsonl"), "w", encoding="utf-8") as out:
        for r in index:
            out.write(json.dumps(r, ensure_ascii=False, separators=(",", ":")) + "\n")
    print(f"Bestiary: {len(index)} monsters")
    return index


def build_spells(repo, out_dir):
    data = load(repo, "Spell.json")
    spells_dir = os.path.join(out_dir, "spells")
    os.makedirs(spells_dir, exist_ok=True)
    index = []
    for entry in data:
        f = entry["fields"]
        slug = entry["pk"] or slugify(f["name"])
        record = {
            "n": f["name"],
            "lvl": f.get("spell_level", 0),
            "school": f.get("school", ""),
            "cast": f.get("casting_time", ""),
            "range": f.get("range", ""),
            "comp": {
                "v": bool(f.get("requires_verbal_components")),
                "s": bool(f.get("requires_somatic_components")),
                "m": bool(f.get("requires_material_components")),
                "mat": f.get("material") or "",
            },
            "dur": f.get("duration", ""),
            "conc": bool(f.get("requires_concentration")),
            "ritual": bool(f.get("can_be_cast_as_ritual")),
            "classes": f.get("dnd_class", ""),
            "desc": f.get("desc", ""),
            "higher": f.get("higher_level") or "",
        }
        with open(os.path.join(spells_dir, f"{slug}.json"), "w", encoding="utf-8") as out:
            json.dump(record, out, ensure_ascii=False, separators=(",", ":"))
        index.append({"s": slug, "n": f["name"], "lvl": f.get("spell_level", 0),
                       "school": f.get("school", ""), "classes": f.get("dnd_class", "")})

    index.sort(key=lambda r: (r["lvl"], r["n"]))
    with open(os.path.join(out_dir, "spells_index.jsonl"), "w", encoding="utf-8") as out:
        for r in index:
            out.write(json.dumps(r, ensure_ascii=False, separators=(",", ":")) + "\n")
    print(f"Spells: {len(index)}")


def build_items(repo, out_dir):
    items_dir = os.path.join(out_dir, "items")
    os.makedirs(items_dir, exist_ok=True)

    weapons = load(repo, "Weapon.json")
    out = []
    for entry in weapons:
        f = entry["fields"]
        out.append({
            "s": entry["pk"], "n": f["name"], "kind": "weapon",
            "cat": f.get("category", ""), "cost": f.get("cost", ""),
            "dmg": f.get("damage_dice", ""), "dmg_type": f.get("damage_type", ""),
            "weight": f.get("weight", ""), "props": jloads(f.get("properties_json"), []),
        })
    with open(os.path.join(items_dir, "weapons.json"), "w", encoding="utf-8") as fp:
        json.dump(out, fp, ensure_ascii=False, separators=(",", ":"))

    armor = load(repo, "Armor.json")
    out = []
    for entry in armor:
        f = entry["fields"]
        out.append({
            "s": entry["pk"], "n": f["name"], "kind": "armor",
            "cat": f.get("category", ""), "cost": f.get("cost", ""),
            "base_ac": f.get("base_ac"), "dex_mod": bool(f.get("plus_dex_mod")),
            "dex_max": f.get("plus_max"), "str_req": f.get("strength_requirement"),
            "stealth_dis": bool(f.get("stealth_disadvantage")), "weight": f.get("weight", ""),
        })
    with open(os.path.join(items_dir, "armor.json"), "w", encoding="utf-8") as fp:
        json.dump(out, fp, ensure_ascii=False, separators=(",", ":"))

    magic = load(repo, "MagicItem.json")
    magic_dir = os.path.join(items_dir, "magic")
    os.makedirs(magic_dir, exist_ok=True)
    index = []
    for entry in magic:
        f = entry["fields"]
        slug = entry["pk"] or slugify(f["name"])
        record = {
            "n": f["name"], "type": f.get("type", ""), "rarity": f.get("rarity", ""),
            "attune": f.get("requires_attunement") or "", "desc": f.get("desc", ""),
        }
        with open(os.path.join(magic_dir, f"{slug}.json"), "w", encoding="utf-8") as fp:
            json.dump(record, fp, ensure_ascii=False, separators=(",", ":"))
        index.append({"s": slug, "n": f["name"], "rarity": f.get("rarity", "")})
    index.sort(key=lambda r: r["n"])
    with open(os.path.join(items_dir, "magic_index.jsonl"), "w", encoding="utf-8") as fp:
        for r in index:
            fp.write(json.dumps(r, ensure_ascii=False, separators=(",", ":")) + "\n")

    print(f"Items: {len(weapons)} weapons, {len(armor)} armor, {len(magic)} magic items")


def build_images(repo, out_dir):
    if Image is None:
        print("Pillow not installed, skipping image resize")
        return
    src_dir = os.path.join(repo, "static", "img", "object_illustrations", "open5e-illustrations", "monsters")
    if not os.path.isdir(src_dir):
        print("No illustrations found, skipping")
        return
    dst_dir = os.path.join(out_dir, "img")
    os.makedirs(dst_dir, exist_ok=True)
    count = 0
    for fname in os.listdir(src_dir):
        if not fname.lower().endswith(".png"):
            continue
        slug = os.path.splitext(fname)[0]
        img = Image.open(os.path.join(src_dir, fname)).convert("RGB")
        target_w = 220
        ratio = target_w / img.width
        target_h = int(img.height * ratio)
        img = img.resize((target_w, target_h), Image.LANCZOS)
        img.save(os.path.join(dst_dir, f"{slug}.jpg"), "JPEG", quality=78, optimize=True)
        count += 1
    print(f"Images: converted {count} illustrations to {dst_dir}")

    license_note = (
        "Illustrations in this folder originate from the open5e project\n"
        "(https://github.com/open5e/open5e-api, static/img/object_illustrations)\n"
        "and are licensed Creative Commons Attribution-NonCommercial 4.0\n"
        "International (CC BY-NC 4.0). Attribution: open5e (open5e.com).\n"
        "Non-commercial use only. See https://creativecommons.org/licenses/by-nc/4.0/\n"
    )
    with open(os.path.join(dst_dir, "LICENSE.txt"), "w", encoding="utf-8") as f:
        f.write(license_note)


def main():
    if len(sys.argv) < 2:
        print("Usage: build_data.py /path/to/open5e-api/checkout")
        sys.exit(1)
    repo = sys.argv[1]
    out_dir = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "sd_card_data")
    if os.path.exists(out_dir):
        shutil.rmtree(out_dir)
    os.makedirs(out_dir, exist_ok=True)

    build_monsters(repo, out_dir)
    build_spells(repo, out_dir)
    build_items(repo, out_dir)
    build_images(repo, out_dir)
    print("Done. Output in", out_dir)


if __name__ == "__main__":
    main()
