#include "ui_style.h"
#include "../fonts/ru_fonts.h"
#include "screen_home.h"
#include "screen_bestiary.h"
#include "screen_character.h"
#include "screen_inventory.h"
#include "screen_spells.h"

lv_style_t style_card;
lv_style_t style_title;
lv_style_t style_dim;

static lv_obj_t *cached[SCREEN_SPELLS + 1] = {nullptr};
static lv_obj_t *detail_screen = nullptr; // current bestiary/spell detail, rebuilt each time

void ui_init_styles() {
    lv_style_init(&style_card);
    lv_style_set_bg_color(&style_card, lv_color_white());
    lv_style_set_radius(&style_card, 6);
    lv_style_set_border_width(&style_card, 1);
    lv_style_set_border_color(&style_card, lv_palette_main(LV_PALETTE_GREY));
    lv_style_set_pad_all(&style_card, 6);

    lv_style_init(&style_title);
    lv_style_set_text_font(&style_title, &ru_font_18);

    lv_style_init(&style_dim);
    lv_style_set_text_color(&style_dim, lv_palette_main(LV_PALETTE_GREY));
}

void ui_add_nav_bar(lv_obj_t *parent) {
    lv_obj_t *bar = lv_obj_create(parent);
    lv_obj_set_size(bar, LV_PCT(100), NAV_BAR_HEIGHT);
    lv_obj_align(bar, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_flex_flow(bar, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(bar, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(bar, 2, 0);
    lv_obj_clear_flag(bar, LV_OBJ_FLAG_SCROLLABLE);

    struct NavDef { const char *label; ScreenId id; };
    static const NavDef defs[] = {
        {"Дом", SCREEN_HOME},
        {"Бестиарий", SCREEN_BESTIARY_LIST},
        {"Герой", SCREEN_CHARACTER},
        {"Инвентарь", SCREEN_INVENTORY},
        {"Заклинания", SCREEN_SPELLS},
    };
    for (const auto &d : defs) {
        lv_obj_t *btn = lv_btn_create(bar);
        lv_obj_set_style_pad_hor(btn, 6, 0);
        lv_obj_add_event_cb(btn, [](lv_event_t *e) {
            ScreenId id = (ScreenId)(intptr_t)lv_event_get_user_data(e);
            ui_show_screen(id);
        }, LV_EVENT_CLICKED, (void *)(intptr_t)d.id);
        lv_obj_t *lbl = lv_label_create(btn);
        lv_label_set_text(lbl, d.label);
        lv_obj_set_style_text_font(lbl, &ru_font_12, 0);
    }
}

void ui_show_screen(ScreenId id) {
    if (!cached[id]) {
        switch (id) {
            case SCREEN_HOME: cached[id] = screen_home_create(); break;
            case SCREEN_BESTIARY_LIST: cached[id] = screen_bestiary_list_create(); break;
            case SCREEN_CHARACTER: cached[id] = screen_character_create(); break;
            case SCREEN_INVENTORY: cached[id] = screen_inventory_create(); break;
            case SCREEN_SPELLS: cached[id] = screen_spells_list_create(); break;
            default: return;
        }
    }
    lv_scr_load(cached[id]);
}

void ui_show_bestiary_detail(const String &slug) {
    lv_obj_t *fresh = screen_bestiary_detail_create(slug);
    lv_obj_t *old = detail_screen;
    detail_screen = fresh;
    lv_scr_load(fresh);
    if (old) lv_obj_del(old);
}

void ui_show_spell_detail(const String &slug) {
    lv_obj_t *fresh = screen_spell_detail_create(slug);
    lv_obj_t *old = detail_screen;
    detail_screen = fresh;
    lv_scr_load(fresh);
    if (old) lv_obj_del(old);
}
