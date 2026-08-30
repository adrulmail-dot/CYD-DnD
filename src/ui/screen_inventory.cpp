#include "screen_inventory.h"
#include "ui_style.h"
#include "ui_widgets.h"
#include "../fonts/ru_fonts.h"
#include "../app_state.h"
#include <algorithm>
#include <cstdio>

static float parseWeightLb(const String &s) {
    float v = 0;
    sscanf(s.c_str(), "%f", &v);
    return v;
}

static lv_obj_t *s_listContainer = nullptr;
static lv_obj_t *s_summaryLabel = nullptr;

static void refreshInventory() {
    if (!s_listContainer) return;
    lv_obj_clean(s_listContainer);

    for (size_t i = 0; i < App.inventory.items.size(); i++) {
        InventoryItem &item = App.inventory.items[i];

        lv_obj_t *row = lv_obj_create(s_listContainer);
        lv_obj_set_size(row, LV_PCT(100), 44);
        lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(row, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

        lv_obj_t *info = lv_obj_create(row);
        lv_obj_remove_style_all(info);
        lv_obj_set_size(info, 120, 40);
        lv_obj_set_flex_flow(info, LV_FLEX_FLOW_COLUMN);
        lv_obj_clear_flag(info, LV_OBJ_FLAG_SCROLLABLE);

        lv_obj_t *nameLbl = lv_label_create(info);
        String nameText = item.name + (item.quantity > 1 ? " x" + String(item.quantity) : "");
        lv_label_set_text(nameLbl, nameText.c_str());

        lv_obj_t *weightLbl = lv_label_create(info);
        char wbuf[32];
        snprintf(wbuf, sizeof(wbuf), "%.1f фт.", item.weight * item.quantity);
        lv_label_set_text(weightLbl, wbuf);
        lv_obj_add_style(weightLbl, &style_dim, 0);
        lv_obj_set_style_text_font(weightLbl, &ru_font_12, 0);

        lv_obj_t *eqCb = lv_checkbox_create(row);
        lv_checkbox_set_text(eqCb, "Экип.");
        if (item.equipped) lv_obj_add_state(eqCb, LV_STATE_CHECKED);
        lv_obj_add_event_cb(eqCb, [](lv_event_t *e) {
            size_t idx = (size_t)(intptr_t)lv_event_get_user_data(e);
            App.inventory.toggleEquipped(idx);
        }, LV_EVENT_VALUE_CHANGED, (void *)(intptr_t)i);

        lv_obj_t *delBtn = lv_btn_create(row);
        lv_obj_set_size(delBtn, 28, 28);
        lv_obj_set_style_bg_color(delBtn, lv_color_hex(UI_COLOR_RED), 0);
        lv_obj_t *delLbl = lv_label_create(delBtn);
        lv_label_set_text(delLbl, LV_SYMBOL_TRASH);
        lv_obj_center(delLbl);
        lv_obj_add_event_cb(delBtn, [](lv_event_t *e) {
            size_t idx = (size_t)(intptr_t)lv_event_get_user_data(e);
            App.inventory.remove(idx);
            refreshInventory();
        }, LV_EVENT_CLICKED, (void *)(intptr_t)i);
    }

    char summary[64];
    float total = App.inventory.totalWeight();
    float cap = App.inventory.carryCapacity(App.character.str);
    snprintf(summary, sizeof(summary), "Вес: %.1f / %.1f фт. (Сила x15)", total, cap);
    lv_label_set_text(s_summaryLabel, summary);
    if (total > cap) {
        lv_obj_set_style_text_color(s_summaryLabel, lv_color_hex(UI_COLOR_RED), 0);
    } else {
        lv_obj_set_style_text_color(s_summaryLabel, lv_color_hex(UI_COLOR_GREEN), 0);
    }
}

// ---- "Add custom item" dialog -----------------------------------------

static void openAddCustomDialog() {
    lv_obj_t *modal = lv_obj_create(lv_layer_top());
    lv_obj_remove_style_all(modal);
    lv_obj_set_size(modal, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(modal, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(modal, LV_OPA_60, 0);

    lv_obj_t *panel = lv_obj_create(modal);
    lv_obj_set_size(panel, LV_PCT(90), 100);
    lv_obj_align(panel, LV_ALIGN_TOP_MID, 0, 4);
    lv_obj_set_flex_flow(panel, LV_FLEX_FLOW_COLUMN);

    lv_obj_t *nameTa = lv_textarea_create(panel);
    lv_textarea_set_one_line(nameTa, true);
    lv_textarea_set_placeholder_text(nameTa, "Название предмета");
    lv_obj_set_width(nameTa, LV_PCT(100));

    lv_obj_t *weightTa = lv_textarea_create(panel);
    lv_textarea_set_one_line(weightTa, true);
    lv_textarea_set_placeholder_text(weightTa, "Вес (фт.), напр. 2.5");
    lv_obj_set_width(weightTa, LV_PCT(100));
    lv_textarea_set_accepted_chars(weightTa, "0123456789.");

    lv_obj_t *addBtn = lv_btn_create(panel);
    lv_obj_t *addLbl = lv_label_create(addBtn);
    lv_label_set_text(addLbl, "Добавить");

    lv_obj_t *kb = lv_keyboard_create(modal);
    lv_obj_set_size(kb, LV_PCT(100), 120);
    lv_obj_align(kb, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_keyboard_set_textarea(kb, nameTa);

    lv_obj_add_event_cb(nameTa, [](lv_event_t *e) {
        lv_keyboard_set_textarea((lv_obj_t *)lv_event_get_user_data(e), lv_event_get_target(e));
    }, LV_EVENT_FOCUSED, kb);
    lv_obj_add_event_cb(weightTa, [](lv_event_t *e) {
        lv_keyboard_set_textarea((lv_obj_t *)lv_event_get_user_data(e), lv_event_get_target(e));
    }, LV_EVENT_FOCUSED, kb);

    struct Ctx { lv_obj_t *nameTa, *weightTa, *modal; };
    auto *ctx = new Ctx{nameTa, weightTa, modal};
    lv_obj_add_event_cb(addBtn, [](lv_event_t *e) {
        auto *c = static_cast<Ctx *>(lv_event_get_user_data(e));
        String name = lv_textarea_get_text(c->nameTa);
        if (name.length() == 0) return;
        InventoryItem item;
        item.name = name;
        item.kind = "gear";
        item.weight = parseWeightLb(lv_textarea_get_text(c->weightTa));
        item.quantity = 1;
        App.inventory.add(item);
        refreshInventory();
        lv_obj_del(c->modal);
    }, LV_EVENT_CLICKED, ctx);
    lv_obj_add_event_cb(modal, [](lv_event_t *e) {
        delete static_cast<Ctx *>(lv_event_get_user_data(e));
    }, LV_EVENT_DELETE, ctx);
}

// ---- "Add from catalog" dialog -----------------------------------------

static void closeModal(lv_obj_t *modal) { lv_obj_del(modal); }

static void openCatalogDialog() {
    lv_obj_t *modal = lv_obj_create(lv_layer_top());
    lv_obj_remove_style_all(modal);
    lv_obj_set_size(modal, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(modal, lv_color_hex(UI_COLOR_BG), 0);
    lv_obj_set_style_bg_opa(modal, LV_OPA_COVER, 0);

    lv_obj_t *closeBtn = lv_btn_create(modal);
    lv_obj_set_size(closeBtn, 28, 28);
    lv_obj_align(closeBtn, LV_ALIGN_TOP_RIGHT, -4, 4);
    lv_obj_add_event_cb(closeBtn, [](lv_event_t *e) {
        closeModal((lv_obj_t *)lv_event_get_user_data(e));
    }, LV_EVENT_CLICKED, modal);
    lv_obj_t *closeLbl = lv_label_create(closeBtn);
    lv_label_set_text(closeLbl, LV_SYMBOL_CLOSE);
    lv_obj_center(closeLbl);

    lv_obj_t *tv = lv_tabview_create(modal, LV_DIR_TOP, 30);
    lv_obj_set_size(tv, LV_PCT(100), LV_PCT(100));

    lv_obj_t *tabWeapons = lv_tabview_add_tab(tv, "Оружие");
    lv_obj_t *tabArmor = lv_tabview_add_tab(tv, "Броня");
    lv_obj_t *tabMagic = lv_tabview_add_tab(tv, "Магия");

    lv_obj_t *weaponList = lv_list_create(tabWeapons);
    lv_obj_set_size(weaponList, LV_PCT(100), LV_PCT(100));
    for (const auto &w : App.items.weapons()) {
        String label = w.name + "  (" + w.damage + " " + w.damageType + ")";
        lv_obj_t *btn = lv_list_add_btn(weaponList, nullptr, label.c_str());
        auto *nameCopy = new String(w.name);
        auto *weightCopy = new float(parseWeightLb(w.weight));
        auto *slugCopy = new String(w.slug);
        struct Ctx { String *name, *slug; float *weight; lv_obj_t *modal; };
        auto *ctx = new Ctx{nameCopy, slugCopy, weightCopy, modal};
        lv_obj_add_event_cb(btn, [](lv_event_t *e) {
            auto *c = static_cast<Ctx *>(lv_event_get_user_data(e));
            InventoryItem item;
            item.name = *c->name;
            item.catalogSlug = *c->slug;
            item.kind = "weapon";
            item.weight = *c->weight;
            item.quantity = 1;
            App.inventory.add(item);
            refreshInventory();
            closeModal(c->modal);
        }, LV_EVENT_CLICKED, ctx);
        lv_obj_add_event_cb(btn, [](lv_event_t *e) {
            auto *c = static_cast<Ctx *>(lv_event_get_user_data(e));
            delete c->name; delete c->slug; delete c->weight; delete c;
        }, LV_EVENT_DELETE, ctx);
    }

    lv_obj_t *armorList = lv_list_create(tabArmor);
    lv_obj_set_size(armorList, LV_PCT(100), LV_PCT(100));
    for (const auto &a : App.items.armor()) {
        String label = a.name + "  (AC " + String(a.baseAc) + ")";
        lv_obj_t *btn = lv_list_add_btn(armorList, nullptr, label.c_str());
        auto *nameCopy = new String(a.name);
        auto *weightCopy = new float(parseWeightLb(a.weight));
        auto *slugCopy = new String(a.slug);
        struct Ctx { String *name, *slug; float *weight; lv_obj_t *modal; };
        auto *ctx = new Ctx{nameCopy, slugCopy, weightCopy, modal};
        lv_obj_add_event_cb(btn, [](lv_event_t *e) {
            auto *c = static_cast<Ctx *>(lv_event_get_user_data(e));
            InventoryItem item;
            item.name = *c->name;
            item.catalogSlug = *c->slug;
            item.kind = "armor";
            item.weight = *c->weight;
            item.quantity = 1;
            App.inventory.add(item);
            refreshInventory();
            closeModal(c->modal);
        }, LV_EVENT_CLICKED, ctx);
        lv_obj_add_event_cb(btn, [](lv_event_t *e) {
            auto *c = static_cast<Ctx *>(lv_event_get_user_data(e));
            delete c->name; delete c->slug; delete c->weight; delete c;
        }, LV_EVENT_DELETE, ctx);
    }

    lv_obj_t *magicList = lv_list_create(tabMagic);
    lv_obj_set_size(magicList, LV_PCT(100), LV_PCT(100));
    size_t shown = 0;
    for (size_t i = 0; i < App.items.magicCount() && shown < 80; i++, shown++) {
        const MagicItemIndexEntry &m = App.items.magicAt(i);
        String label = m.name + " (" + m.rarity + ")";
        lv_obj_t *btn = lv_list_add_btn(magicList, nullptr, label.c_str());
        auto *nameCopy = new String(m.name);
        auto *slugCopy = new String(m.slug);
        struct Ctx { String *name, *slug; lv_obj_t *modal; };
        auto *ctx = new Ctx{nameCopy, slugCopy, modal};
        lv_obj_add_event_cb(btn, [](lv_event_t *e) {
            auto *c = static_cast<Ctx *>(lv_event_get_user_data(e));
            InventoryItem item;
            item.name = *c->name;
            item.catalogSlug = *c->slug;
            item.kind = "magic";
            item.weight = 0;
            item.quantity = 1;
            App.inventory.add(item);
            refreshInventory();
            closeModal(c->modal);
        }, LV_EVENT_CLICKED, ctx);
        lv_obj_add_event_cb(btn, [](lv_event_t *e) {
            auto *c = static_cast<Ctx *>(lv_event_get_user_data(e));
            delete c->name; delete c->slug; delete c;
        }, LV_EVENT_DELETE, ctx);
    }
}

lv_obj_t *screen_inventory_create() {
    lv_obj_t *scr = ui_new_screen();
    lv_obj_set_flex_flow(scr, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(scr, 4, 0);
    lv_obj_set_style_pad_row(scr, 4, 0);

    lv_obj_t *title = lv_label_create(scr);
    lv_label_set_text(title, "Инвентарь");
    lv_obj_add_style(title, &style_title, 0);

    s_summaryLabel = lv_label_create(scr);
    lv_label_set_text(s_summaryLabel, "");

    lv_obj_t *btnRow = lv_obj_create(scr);
    lv_obj_remove_style_all(btnRow);
    lv_obj_set_size(btnRow, LV_PCT(100), 32);
    lv_obj_set_flex_flow(btnRow, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(btnRow, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(btnRow, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *addCustomBtn = lv_btn_create(btnRow);
    lv_obj_t *addCustomLbl = lv_label_create(addCustomBtn);
    lv_label_set_text(addCustomLbl, "+ Своё");
    lv_obj_add_event_cb(addCustomBtn, [](lv_event_t *) { openAddCustomDialog(); }, LV_EVENT_CLICKED, nullptr);

    lv_obj_t *addCatalogBtn = lv_btn_create(btnRow);
    lv_obj_t *addCatalogLbl = lv_label_create(addCatalogBtn);
    lv_label_set_text(addCatalogLbl, "+ Из каталога SRD");
    lv_obj_add_event_cb(addCatalogBtn, [](lv_event_t *) { openCatalogDialog(); }, LV_EVENT_CLICKED, nullptr);

    s_listContainer = lv_obj_create(scr);
    lv_obj_set_width(s_listContainer, LV_PCT(100));
    lv_obj_set_flex_grow(s_listContainer, 1);
    lv_obj_set_flex_flow(s_listContainer, LV_FLEX_FLOW_COLUMN);

    ui_add_nav_bar(scr);
    refreshInventory();
    return scr;
}
