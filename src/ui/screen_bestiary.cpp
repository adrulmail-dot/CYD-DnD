#include "screen_bestiary.h"
#include "ui_style.h"
#include "img_loader.h"
#include "../fonts/ru_fonts.h"
#include "../app_state.h"
#include <algorithm>

static const size_t MAX_RESULTS_SHOWN = 60;

// ---- List screen -----------------------------------------------------

static void bestiary_item_delete_cb(lv_event_t *e) {
    auto *slug = static_cast<String *>(lv_event_get_user_data(e));
    delete slug;
}

static void bestiary_item_click_cb(lv_event_t *e) {
    auto *slug = static_cast<String *>(lv_event_get_user_data(e));
    ui_show_bestiary_detail(*slug);
}

static void refresh_bestiary_list(lv_obj_t *list, const String &query, const String &typeFilter) {
    lv_obj_clean(list);
    std::vector<size_t> matches = App.bestiary.search(query, typeFilter);

    char buf[48];
    if (matches.size() > MAX_RESULTS_SHOWN) {
        snprintf(buf, sizeof(buf), "Показаны первые %u из %u", (unsigned)MAX_RESULTS_SHOWN, (unsigned)matches.size());
    } else {
        snprintf(buf, sizeof(buf), "Найдено: %u", (unsigned)matches.size());
    }
    lv_list_add_text(list, buf);

    size_t shown = 0;
    for (size_t idx : matches) {
        if (shown++ >= MAX_RESULTS_SHOWN) break;
        const BestiaryIndexEntry &e = App.bestiary.at(idx);
        String label = e.name + "  (CR " + e.cr + ", " + e.type + ")";
        lv_obj_t *btn = lv_list_add_btn(list, e.img.length() ? LV_SYMBOL_IMAGE : nullptr, label.c_str());
        auto *slugCopy = new String(e.slug);
        lv_obj_add_event_cb(btn, bestiary_item_click_cb, LV_EVENT_CLICKED, slugCopy);
        lv_obj_add_event_cb(btn, bestiary_item_delete_cb, LV_EVENT_DELETE, slugCopy);
    }
}

lv_obj_t *screen_bestiary_list_create() {
    lv_obj_t *scr = lv_obj_create(NULL);
    lv_obj_set_flex_flow(scr, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(scr, 4, 0);
    lv_obj_set_style_pad_row(scr, 4, 0);

    lv_obj_t *title = lv_label_create(scr);
    lv_label_set_text(title, "Бестиарий");
    lv_obj_add_style(title, &style_title, 0);

    lv_obj_t *searchRow = lv_obj_create(scr);
    lv_obj_remove_style_all(searchRow);
    lv_obj_set_size(searchRow, LV_PCT(100), 34);
    lv_obj_set_flex_flow(searchRow, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(searchRow, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(searchRow, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *search = lv_textarea_create(searchRow);
    lv_textarea_set_one_line(search, true);
    lv_textarea_set_placeholder_text(search, "Поиск по имени...");
    lv_obj_set_width(search, 150);

    lv_obj_t *typeDropdown = lv_dropdown_create(searchRow);
    lv_obj_set_width(typeDropdown, 80);
    String options = "Все";
    std::vector<String> seenTypes;
    for (size_t i = 0; i < App.bestiary.count(); i++) {
        const String &t = App.bestiary.at(i).type;
        if (t.length() && std::find(seenTypes.begin(), seenTypes.end(), t) == seenTypes.end()) {
            seenTypes.push_back(t);
        }
    }
    std::sort(seenTypes.begin(), seenTypes.end());
    for (const String &t : seenTypes) { options += "\n"; options += t; }
    lv_dropdown_set_options(typeDropdown, options.c_str());

    lv_obj_t *list = lv_list_create(scr);
    lv_obj_set_width(list, LV_PCT(100));
    lv_obj_set_flex_grow(list, 1);

    lv_obj_t *kb = lv_keyboard_create(scr);
    lv_obj_set_size(kb, LV_PCT(100), 120);
    lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
    lv_keyboard_set_textarea(kb, search);

    ui_add_nav_bar(scr);

    struct Ctx { lv_obj_t *list; lv_obj_t *search; lv_obj_t *dropdown; lv_obj_t *kb; };
    static Ctx ctx; // one bestiary list screen exists at a time
    ctx = {list, search, typeDropdown, kb};

    auto doRefresh = [](lv_event_t *) {
        char sel[32];
        lv_dropdown_get_selected_str(ctx.dropdown, sel, sizeof(sel));
        String typeFilter = (strcmp(sel, "Все") == 0) ? "" : String(sel);
        refresh_bestiary_list(ctx.list, lv_textarea_get_text(ctx.search), typeFilter);
    };

    lv_obj_add_event_cb(search, doRefresh, LV_EVENT_VALUE_CHANGED, nullptr);
    lv_obj_add_event_cb(typeDropdown, doRefresh, LV_EVENT_VALUE_CHANGED, nullptr);
    lv_obj_add_event_cb(search, [](lv_event_t *e) {
        lv_obj_clear_flag(ctx.kb, LV_OBJ_FLAG_HIDDEN);
    }, LV_EVENT_FOCUSED, nullptr);
    lv_obj_add_event_cb(kb, [](lv_event_t *e) {
        lv_event_code_t code = lv_event_get_code(e);
        if (code == LV_EVENT_READY || code == LV_EVENT_CANCEL) {
            lv_obj_add_flag(ctx.kb, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_state(ctx.search, LV_STATE_FOCUSED);
        }
    }, LV_EVENT_ALL, nullptr);

    refresh_bestiary_list(list, "", "");
    return scr;
}

// ---- Detail screen -----------------------------------------------------

static String formatAbility(const char *name, int score) {
    int mod = (score - 10 >= 0) ? (score - 10) / 2 : -((10 - score + 1) / 2);
    char buf[24];
    snprintf(buf, sizeof(buf), "%s %d (%s%d)", name, score, mod >= 0 ? "+" : "", mod);
    return String(buf);
}

lv_obj_t *screen_bestiary_detail_create(const String &slug) {
    lv_obj_t *scr = lv_obj_create(NULL);
    lv_obj_set_style_pad_all(scr, 0, 0);

    StatBlock sb;
    bool ok = App.bestiary.loadDetail(slug, sb);

    // Header: back button, image/placeholder, name + CR.
    lv_obj_t *header = lv_obj_create(scr);
    lv_obj_remove_style_all(header);
    lv_obj_set_size(header, LV_PCT(100), 56);
    lv_obj_set_style_pad_all(header, 4, 0);
    lv_obj_clear_flag(header, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *back = lv_btn_create(header);
    lv_obj_set_size(back, 26, 26);
    lv_obj_align(back, LV_ALIGN_LEFT_MID, 0, 0);
    lv_obj_add_event_cb(back, [](lv_event_t *) { ui_show_screen(SCREEN_BESTIARY_LIST); }, LV_EVENT_CLICKED, nullptr);
    lv_obj_t *backLbl = lv_label_create(back);
    lv_label_set_text(backLbl, LV_SYMBOL_LEFT);
    lv_obj_center(backLbl);

    lv_obj_t *thumb = lv_obj_create(header);
    lv_obj_set_size(thumb, 44, 44);
    lv_obj_align(thumb, LV_ALIGN_LEFT_MID, 32, 0);
    lv_obj_set_style_radius(thumb, 6, 0);
    lv_obj_clear_flag(thumb, LV_OBJ_FLAG_SCROLLABLE);

    lv_img_dsc_t *dsc = (ok && sb.img.length()) ? load_jpeg_from_sd("/img/" + sb.img + ".jpg", 44) : nullptr;
    if (dsc) {
        lv_obj_t *img = lv_img_create(thumb);
        lv_img_set_src(img, dsc);
        lv_img_set_zoom(img, (float)44 / dsc->header.w * 256);
        lv_obj_center(img);
        img_loader_attach_cleanup(img, dsc);
    } else {
        lv_obj_set_style_bg_color(thumb, lv_palette_main(LV_PALETTE_INDIGO), 0);
        lv_obj_t *lbl = lv_label_create(thumb);
        lv_label_set_text(lbl, ok && sb.name.length() ? String(sb.name[0]).c_str() : "?");
        lv_obj_set_style_text_color(lbl, lv_color_white(), 0);
        lv_obj_center(lbl);
    }

    lv_obj_t *nameLbl = lv_label_create(header);
    lv_label_set_text(nameLbl, ok ? sb.name.c_str() : "Не найдено");
    lv_obj_set_style_text_font(nameLbl, &ru_font_16, 0);
    lv_obj_align(nameLbl, LV_ALIGN_LEFT_MID, 84, -10);

    lv_obj_t *metaLbl = lv_label_create(header);
    String meta = ok ? (sb.size + " " + sb.type + (sb.subtype.length() ? " (" + sb.subtype + ")" : "") + " · CR " + sb.cr) : "";
    lv_label_set_text(metaLbl, meta.c_str());
    lv_obj_add_style(metaLbl, &style_dim, 0);
    lv_obj_set_style_text_font(metaLbl, &ru_font_12, 0);
    lv_obj_align(metaLbl, LV_ALIGN_LEFT_MID, 84, 10);

    // Tabs
    lv_obj_t *tv = lv_tabview_create(scr, LV_DIR_TOP, 28);
    lv_obj_set_size(tv, LV_PCT(100), 320 - 56 - NAV_BAR_HEIGHT);
    lv_obj_align_to(tv, header, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);

    lv_obj_t *tabStats = lv_tabview_add_tab(tv, "Статы");
    lv_obj_t *tabActions = lv_tabview_add_tab(tv, "Действия");
    lv_obj_t *tabTips = lv_tabview_add_tab(tv, "Тактика");

    if (ok) {
        String stats;
        stats += "КД: " + String(sb.ac) + (sb.armorDesc.length() ? " (" + sb.armorDesc + ")" : "") + "\n";
        stats += "ХП: " + String(sb.hp) + " (" + sb.hitDice + ")\n";
        stats += "Скорость: " + sb.speed + "\n";
        stats += "Мировоззрение: " + sb.alignment + "\n\n";
        stats += formatAbility("STR", sb.str) + "   " + formatAbility("DEX", sb.dex) + "   " + formatAbility("CON", sb.con) + "\n";
        stats += formatAbility("INT", sb.intel) + "   " + formatAbility("WIS", sb.wis) + "   " + formatAbility("CHA", sb.cha) + "\n\n";
        if (sb.savesText.length()) stats += "Спасброски: " + sb.savesText + "\n";
        if (sb.skillsText.length()) stats += "Навыки: " + sb.skillsText + "\n";
        if (sb.senses.length()) stats += "Чувства: " + sb.senses + "\n";
        if (sb.languages.length()) stats += "Языки: " + sb.languages + "\n";
        if (sb.vulnerabilities.length()) stats += "\nУязвимости: " + sb.vulnerabilities + "\n";
        if (sb.resistances.length()) stats += "Сопротивления: " + sb.resistances + "\n";
        if (sb.immunities.length()) stats += "Иммунитет к урону: " + sb.immunities + "\n";
        if (sb.conditionImmunities.length()) stats += "Иммунитет к состояниям: " + sb.conditionImmunities + "\n";

        lv_obj_t *statsLbl = lv_label_create(tabStats);
        lv_label_set_long_mode(statsLbl, LV_LABEL_LONG_WRAP);
        lv_obj_set_width(statsLbl, LV_PCT(96));
        lv_label_set_text(statsLbl, stats.c_str());

        String actionsText;
        auto appendSection = [&](const char *title, const std::vector<StatBlock::Entry> &entries) {
            if (entries.empty()) return;
            actionsText += String("== ") + title + " ==\n";
            for (const auto &e : entries) {
                actionsText += e.name + ": " + e.desc + "\n\n";
            }
        };
        appendSection("Особенности", sb.traits);
        appendSection("Действия", sb.actions);
        appendSection("Бонусные действия", sb.bonusActions);
        appendSection("Реакции", sb.reactions);
        if (!sb.legendary.empty()) {
            actionsText += "== Легендарные действия ==\n";
            if (sb.legendaryDesc.length()) actionsText += sb.legendaryDesc + "\n\n";
            for (const auto &e : sb.legendary) actionsText += e.name + ": " + e.desc + "\n\n";
        }
        if (!actionsText.length()) actionsText = "Нет особых действий.";

        lv_obj_t *actionsLbl = lv_label_create(tabActions);
        lv_label_set_long_mode(actionsLbl, LV_LABEL_LONG_WRAP);
        lv_obj_set_width(actionsLbl, LV_PCT(96));
        lv_label_set_text(actionsLbl, actionsText.c_str());

        String tipsText;
        for (const auto &t : sb.tips) tipsText += String(LV_SYMBOL_RIGHT) + " " + t + "\n\n";
        lv_obj_t *tipsLbl = lv_label_create(tabTips);
        lv_label_set_long_mode(tipsLbl, LV_LABEL_LONG_WRAP);
        lv_obj_set_width(tipsLbl, LV_PCT(96));
        lv_label_set_text(tipsLbl, tipsText.c_str());
    }

    ui_add_nav_bar(scr);
    return scr;
}
