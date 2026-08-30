#include "screen_spells.h"
#include "ui_style.h"
#include "ui_widgets.h"
#include "../fonts/ru_fonts.h"
#include "../app_state.h"
#include "../data/sd_json.h"
#include <cstdlib>

// ---- Known spells / slot tracker tab -----------------------------------

static lv_obj_t *s_slotsContainer = nullptr;
static lv_obj_t *s_knownContainer = nullptr;

static void refreshSlots() {
    if (!s_slotsContainer) return;
    lv_obj_clean(s_slotsContainer);
    Spellbook &sb = App.spellbook;
    for (int lvl = 1; lvl <= 9; lvl++) {
        lv_obj_t *row = lv_obj_create(s_slotsContainer);
        lv_obj_set_size(row, LV_PCT(100), 30);
        lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(row, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

        lv_obj_t *lbl = lv_label_create(row);
        char buf[24];
        snprintf(buf, sizeof(buf), "Ур.%d: %d/%d", lvl, sb.slotsUsed[lvl], sb.slotsTotal[lvl]);
        lv_label_set_text(lbl, buf);

        lv_obj_t *useBtn = lv_btn_create(row);
        lv_obj_set_size(useBtn, 50, 26);
        lv_obj_t *useLbl = lv_label_create(useBtn);
        lv_label_set_text(useLbl, "Каст");
        lv_obj_add_event_cb(useBtn, [](lv_event_t *e) {
            int lvl = (int)(intptr_t)lv_event_get_user_data(e);
            App.spellbook.useSlot(lvl);
            refreshSlots();
        }, LV_EVENT_CLICKED, (void *)(intptr_t)lvl);

        lv_obj_t *restoreBtn = lv_btn_create(row);
        lv_obj_set_size(restoreBtn, 26, 26);
        lv_obj_t *restoreLbl = lv_label_create(restoreBtn);
        lv_label_set_text(restoreLbl, "+");
        lv_obj_add_event_cb(restoreBtn, [](lv_event_t *e) {
            int lvl = (int)(intptr_t)lv_event_get_user_data(e);
            App.spellbook.restoreSlot(lvl);
            refreshSlots();
        }, LV_EVENT_CLICKED, (void *)(intptr_t)lvl);

        lv_obj_t *totalMinus = lv_btn_create(row);
        lv_obj_set_size(totalMinus, 26, 26);
        lv_obj_t *tmLbl = lv_label_create(totalMinus);
        lv_label_set_text(tmLbl, LV_SYMBOL_MINUS);
        lv_obj_add_event_cb(totalMinus, [](lv_event_t *e) {
            int lvl = (int)(intptr_t)lv_event_get_user_data(e);
            if (App.spellbook.slotsTotal[lvl] > 0) App.spellbook.slotsTotal[lvl]--;
            refreshSlots();
        }, LV_EVENT_CLICKED, (void *)(intptr_t)lvl);

        lv_obj_t *totalPlus = lv_btn_create(row);
        lv_obj_set_size(totalPlus, 26, 26);
        lv_obj_t *tpLbl = lv_label_create(totalPlus);
        lv_label_set_text(tpLbl, LV_SYMBOL_PLUS);
        lv_obj_add_event_cb(totalPlus, [](lv_event_t *e) {
            int lvl = (int)(intptr_t)lv_event_get_user_data(e);
            App.spellbook.slotsTotal[lvl]++;
            refreshSlots();
        }, LV_EVENT_CLICKED, (void *)(intptr_t)lvl);
    }
}

static void refreshKnown() {
    if (!s_knownContainer) return;
    lv_obj_clean(s_knownContainer);
    for (size_t i = 0; i < App.spellbook.known.size(); i++) {
        KnownSpell &s = App.spellbook.known[i];
        lv_obj_t *row = lv_obj_create(s_knownContainer);
        lv_obj_set_size(row, LV_PCT(100), 34);
        lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(row, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

        lv_obj_t *nameLbl = lv_label_create(row);
        String txt = s.name + " (ур." + String(s.level) + ")";
        lv_label_set_text(nameLbl, txt.c_str());
        lv_obj_add_flag(nameLbl, LV_OBJ_FLAG_CLICKABLE);
        auto *slugCopy = new String(s.slug);
        lv_obj_add_event_cb(nameLbl, [](lv_event_t *e) {
            auto *slug = static_cast<String *>(lv_event_get_user_data(e));
            ui_show_spell_detail(*slug);
        }, LV_EVENT_CLICKED, slugCopy);
        lv_obj_add_event_cb(nameLbl, [](lv_event_t *e) {
            delete static_cast<String *>(lv_event_get_user_data(e));
        }, LV_EVENT_DELETE, slugCopy);

        lv_obj_t *prepCb = lv_checkbox_create(row);
        lv_checkbox_set_text(prepCb, "Подг.");
        if (s.prepared) lv_obj_add_state(prepCb, LV_STATE_CHECKED);
        lv_obj_add_event_cb(prepCb, [](lv_event_t *e) {
            size_t idx = (size_t)(intptr_t)lv_event_get_user_data(e);
            App.spellbook.known[idx].prepared = !App.spellbook.known[idx].prepared;
        }, LV_EVENT_CLICKED, (void *)(intptr_t)i);

        lv_obj_t *delBtn = lv_btn_create(row);
        lv_obj_set_size(delBtn, 26, 26);
        lv_obj_set_style_bg_color(delBtn, lv_color_hex(UI_COLOR_RED), 0);
        lv_obj_t *delLbl = lv_label_create(delBtn);
        lv_label_set_text(delLbl, LV_SYMBOL_TRASH);
        lv_obj_center(delLbl);
        lv_obj_add_event_cb(delBtn, [](lv_event_t *e) {
            size_t idx = (size_t)(intptr_t)lv_event_get_user_data(e);
            App.spellbook.removeSpell(idx);
            refreshKnown();
        }, LV_EVENT_CLICKED, (void *)(intptr_t)i);
    }
}

static void buildMineTab(lv_obj_t *tab) {
    lv_obj_set_flex_flow(tab, LV_FLEX_FLOW_COLUMN);

    lv_obj_t *restBtn = lv_btn_create(tab);
    lv_obj_set_size(restBtn, LV_PCT(100), 30);
    lv_obj_set_style_bg_color(restBtn, lv_color_hex(UI_COLOR_PURPLE), 0);
    lv_obj_t *restLbl = lv_label_create(restBtn);
    lv_label_set_text(restLbl, "Долгий отдых (сброс ячеек)");
    lv_obj_center(restLbl);
    lv_obj_add_event_cb(restBtn, [](lv_event_t *) {
        App.spellbook.longRest();
        refreshSlots();
    }, LV_EVENT_CLICKED, nullptr);

    lv_obj_t *slotsTitle = lv_label_create(tab);
    lv_label_set_text(slotsTitle, "-- Ячейки заклинаний --");
    lv_obj_add_style(slotsTitle, &style_dim, 0);

    s_slotsContainer = lv_obj_create(tab);
    lv_obj_set_width(s_slotsContainer, LV_PCT(100));
    lv_obj_set_height(s_slotsContainer, 220);
    lv_obj_set_flex_flow(s_slotsContainer, LV_FLEX_FLOW_COLUMN);

    lv_obj_t *knownTitle = lv_label_create(tab);
    lv_label_set_text(knownTitle, "-- Известные заклинания --");
    lv_obj_add_style(knownTitle, &style_dim, 0);

    s_knownContainer = lv_obj_create(tab);
    lv_obj_set_width(s_knownContainer, LV_PCT(100));
    lv_obj_set_flex_flow(s_knownContainer, LV_FLEX_FLOW_COLUMN);

    lv_obj_t *saveBtn = lv_btn_create(tab);
    lv_obj_set_size(saveBtn, LV_PCT(100), 32);
    lv_obj_set_style_bg_color(saveBtn, lv_color_hex(UI_COLOR_GREEN), 0);
    lv_obj_add_event_cb(saveBtn, [](lv_event_t *) { App.persist(); }, LV_EVENT_CLICKED, nullptr);
    lv_obj_t *saveLbl = lv_label_create(saveBtn);
    lv_label_set_text(saveLbl, LV_SYMBOL_SAVE " Сохранить");
    lv_obj_center(saveLbl);

    refreshSlots();
    refreshKnown();
}

// ---- Catalog tab --------------------------------------------------------

static const size_t MAX_SPELL_RESULTS = 60;

static void buildCatalogTab(lv_obj_t *tab) {
    lv_obj_set_flex_flow(tab, LV_FLEX_FLOW_COLUMN);

    lv_obj_t *searchRow = lv_obj_create(tab);
    lv_obj_remove_style_all(searchRow);
    lv_obj_set_size(searchRow, LV_PCT(100), 34);
    lv_obj_set_flex_flow(searchRow, LV_FLEX_FLOW_ROW);
    lv_obj_clear_flag(searchRow, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *search = lv_textarea_create(searchRow);
    lv_textarea_set_one_line(search, true);
    lv_textarea_set_placeholder_text(search, "Поиск заклинания...");
    lv_obj_set_width(search, 150);

    lv_obj_t *levelDropdown = lv_dropdown_create(searchRow);
    lv_obj_set_width(levelDropdown, 80);
    lv_dropdown_set_options(levelDropdown, "Любой\n0\n1\n2\n3\n4\n5\n6\n7\n8\n9");

    lv_obj_t *list = lv_list_create(tab);
    lv_obj_set_width(list, LV_PCT(100));
    lv_obj_set_flex_grow(list, 1);

    lv_obj_t *kb = lv_keyboard_create(tab);
    lv_obj_set_size(kb, LV_PCT(100), 110);
    lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
    lv_keyboard_set_textarea(kb, search);

    struct Ctx { lv_obj_t *list, *search, *dropdown, *kb; };
    static Ctx ctx;
    ctx = {list, search, levelDropdown, kb};

    auto refresh = [](lv_event_t *) {
        lv_obj_clean(ctx.list);
        char sel[16];
        lv_dropdown_get_selected_str(ctx.dropdown, sel, sizeof(sel));
        int level = (strcmp(sel, "Любой") == 0) ? -1 : atoi(sel);
        std::vector<size_t> matches = App.spells.search(lv_textarea_get_text(ctx.search), level);

        char buf[48];
        if (matches.size() > MAX_SPELL_RESULTS) {
            snprintf(buf, sizeof(buf), "Показаны первые %u из %u", (unsigned)MAX_SPELL_RESULTS, (unsigned)matches.size());
        } else {
            snprintf(buf, sizeof(buf), "Найдено: %u", (unsigned)matches.size());
        }
        lv_list_add_text(ctx.list, buf);

        size_t shown = 0;
        for (size_t idx : matches) {
            if (shown++ >= MAX_SPELL_RESULTS) break;
            const SpellIndexEntry &s = App.spells.at(idx);
            String label = s.name + " (ур." + String(s.level) + ", " + s.school + ")";
            lv_obj_t *btn = lv_list_add_btn(ctx.list, nullptr, label.c_str());
            auto *slugCopy = new String(s.slug);
            lv_obj_add_event_cb(btn, [](lv_event_t *e) {
                auto *slug = static_cast<String *>(lv_event_get_user_data(e));
                ui_show_spell_detail(*slug);
            }, LV_EVENT_CLICKED, slugCopy);
            lv_obj_add_event_cb(btn, [](lv_event_t *e) {
                delete static_cast<String *>(lv_event_get_user_data(e));
            }, LV_EVENT_DELETE, slugCopy);
        }
    };

    lv_obj_add_event_cb(search, refresh, LV_EVENT_VALUE_CHANGED, nullptr);
    lv_obj_add_event_cb(levelDropdown, refresh, LV_EVENT_VALUE_CHANGED, nullptr);
    lv_obj_add_event_cb(search, [](lv_event_t *) { lv_obj_clear_flag(ctx.kb, LV_OBJ_FLAG_HIDDEN); }, LV_EVENT_FOCUSED, nullptr);
    lv_obj_add_event_cb(kb, [](lv_event_t *e) {
        lv_event_code_t code = lv_event_get_code(e);
        if (code == LV_EVENT_READY || code == LV_EVENT_CANCEL) {
            lv_obj_add_flag(ctx.kb, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_state(ctx.search, LV_STATE_FOCUSED);
        }
    }, LV_EVENT_ALL, nullptr);

    refresh(nullptr);
}

lv_obj_t *screen_spells_list_create() {
    lv_obj_t *scr = ui_new_screen();
    lv_obj_set_style_pad_all(scr, 0, 0);

    lv_obj_t *tv = lv_tabview_create(scr, LV_DIR_TOP, 30);
    lv_obj_set_size(tv, LV_PCT(100), 240 - NAV_BAR_HEIGHT);

    lv_obj_t *tabMine = lv_tabview_add_tab(tv, "Мои");
    lv_obj_t *tabCatalog = lv_tabview_add_tab(tv, "Каталог SRD");

    buildMineTab(tabMine);
    buildCatalogTab(tabCatalog);

    ui_add_nav_bar(scr);
    return scr;
}

// ---- Spell detail screen ------------------------------------------------

lv_obj_t *screen_spell_detail_create(const String &slug) {
    lv_obj_t *scr = ui_new_screen();
    lv_obj_set_flex_flow(scr, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(scr, 4, 0);

    SpellDetail sp;
    sdjson::lastError() = "";
    bool ok = App.spells.loadDetail(slug, sp);
    String loadError = sdjson::lastError();

    lv_obj_t *header = lv_obj_create(scr);
    lv_obj_remove_style_all(header);
    lv_obj_set_size(header, LV_PCT(100), 30);
    lv_obj_set_flex_flow(header, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(header, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(header, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *back = lv_btn_create(header);
    lv_obj_set_size(back, 26, 26);
    lv_obj_add_event_cb(back, [](lv_event_t *) { ui_show_screen(SCREEN_SPELLS); }, LV_EVENT_CLICKED, nullptr);
    lv_obj_t *backLbl = lv_label_create(back);
    lv_label_set_text(backLbl, LV_SYMBOL_LEFT);
    lv_obj_center(backLbl);

    lv_obj_t *title = lv_label_create(header);
    lv_label_set_text(title, ok ? sp.name.c_str() : "Не найдено");
    lv_obj_set_style_text_font(title, &ru_font_16, 0);

    lv_obj_t *addBtn = lv_btn_create(header);
    lv_obj_t *addLbl = lv_label_create(addBtn);
    lv_label_set_text(addLbl, LV_SYMBOL_PLUS);
    if (ok) {
        struct AddCtx { String slug; String name; int level; };
        auto *addCtx = new AddCtx{slug, sp.name, sp.level};
        lv_obj_add_event_cb(addBtn, [](lv_event_t *e) {
            auto *a = static_cast<AddCtx *>(lv_event_get_user_data(e));
            for (const auto &k : App.spellbook.known) {
                if (k.slug == a->slug) return; // already known
            }
            KnownSpell ks;
            ks.slug = a->slug;
            ks.name = a->name;
            ks.level = a->level;
            App.spellbook.addSpell(ks);
        }, LV_EVENT_CLICKED, addCtx);
        lv_obj_add_event_cb(addBtn, [](lv_event_t *e) {
            delete static_cast<AddCtx *>(lv_event_get_user_data(e));
        }, LV_EVENT_DELETE, addCtx);
    }

    if (ok) {
        lv_obj_t *badgeRow = lv_obj_create(scr);
        lv_obj_remove_style_all(badgeRow);
        lv_obj_set_size(badgeRow, LV_PCT(100), LV_SIZE_CONTENT);
        lv_obj_set_flex_flow(badgeRow, LV_FLEX_FLOW_ROW_WRAP);
        lv_obj_set_style_pad_column(badgeRow, 4, 0);
        lv_obj_clear_flag(badgeRow, LV_OBJ_FLAG_SCROLLABLE);
        char lvlBuf[16];
        if (sp.level == 0) {
            snprintf(lvlBuf, sizeof(lvlBuf), "Заговор");
        } else {
            snprintf(lvlBuf, sizeof(lvlBuf), "Уровень %d", sp.level);
        }
        ui_make_badge(badgeRow, lvlBuf, lv_color_hex(UI_COLOR_PURPLE));
        if (sp.school.length()) ui_make_badge(badgeRow, sp.school.c_str(), lv_color_hex(UI_COLOR_BLUE));
        if (sp.concentration) ui_make_badge(badgeRow, "Концентрация", lv_color_hex(UI_COLOR_AMBER));
        if (sp.ritual) ui_make_badge(badgeRow, "Ритуал", lv_color_hex(UI_COLOR_GREEN));

        lv_obj_t *meta = lv_label_create(scr);
        String metaText = "Время накладывания: " + sp.castingTime + "\n";
        metaText += "Дистанция: " + sp.range + "\n";
        metaText += "Длительность: " + sp.duration + (sp.concentration ? " (концентрация)" : "") + "\n";
        String comp = "";
        if (sp.verbal) comp += "В ";
        if (sp.somatic) comp += "С ";
        if (sp.material) comp += "М";
        metaText += "Компоненты: " + comp;
        if (sp.material && sp.materialDesc.length()) metaText += " (" + sp.materialDesc + ")";
        metaText += "\n";
        if (sp.ritual) metaText += "Можно кастовать ритуалом\n";
        metaText += "Классы: " + sp.classes;

        lv_label_set_long_mode(meta, LV_LABEL_LONG_WRAP);
        lv_obj_set_width(meta, LV_PCT(96));
        lv_obj_add_style(meta, &style_dim, 0);
        lv_label_set_text(meta, metaText.c_str());

        lv_obj_t *descLbl = lv_label_create(scr);
        lv_label_set_long_mode(descLbl, LV_LABEL_LONG_WRAP);
        lv_obj_set_width(descLbl, LV_PCT(96));
        String descText = sp.desc;
        if (sp.higherLevel.length()) descText += "\n\nНа больших уровнях: " + sp.higherLevel;
        lv_label_set_text(descLbl, descText.c_str());
    } else {
        lv_obj_t *errLbl = lv_label_create(scr);
        lv_label_set_long_mode(errLbl, LV_LABEL_LONG_WRAP);
        lv_obj_set_width(errLbl, LV_PCT(96));
        String msg = "Путь: /spells/" + slug + ".json\n\n";
        msg += loadError.length() ? loadError : "Причина неизвестна (пустая ошибка).";
        lv_label_set_text(errLbl, msg.c_str());
    }

    ui_add_nav_bar(scr);
    return scr;
}
