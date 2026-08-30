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

void ui_apply_dark_theme(lv_disp_t *disp) {
    lv_theme_t *th = lv_theme_default_init(disp, lv_color_hex(UI_COLOR_AMBER), lv_color_hex(UI_COLOR_BLUE),
                                            true, &ru_font_14);
    lv_disp_set_theme(disp, th);
}

lv_obj_t *ui_new_screen() {
    lv_obj_t *scr = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(scr, lv_color_hex(UI_COLOR_BG), 0);
    return scr;
}

void ui_init_styles() {
    lv_style_init(&style_card);
    lv_style_set_bg_color(&style_card, lv_color_hex(UI_COLOR_PANEL));
    lv_style_set_radius(&style_card, 8);
    lv_style_set_border_width(&style_card, 2);
    lv_style_set_border_color(&style_card, lv_color_hex(UI_COLOR_BORDER));
    lv_style_set_pad_all(&style_card, 6);

    lv_style_init(&style_title);
    lv_style_set_text_font(&style_title, &ru_font_18);
    lv_style_set_text_color(&style_title, lv_color_hex(UI_COLOR_AMBER));

    lv_style_init(&style_dim);
    lv_style_set_text_color(&style_dim, lv_color_hex(UI_COLOR_TEXT_DIM));
}

void ui_style_panel(lv_obj_t *obj, lv_color_t accent) {
    lv_obj_set_style_bg_color(obj, lv_color_hex(UI_COLOR_PANEL), 0);
    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(obj, 8, 0);
    lv_obj_set_style_border_width(obj, 2, 0);
    lv_obj_set_style_border_color(obj, accent, 0);
    lv_obj_set_style_shadow_width(obj, 8, 0);
    lv_obj_set_style_shadow_ofs_x(obj, 2, 0);
    lv_obj_set_style_shadow_ofs_y(obj, 2, 0);
    lv_obj_set_style_shadow_color(obj, lv_color_black(), 0);
    lv_obj_set_style_shadow_opa(obj, LV_OPA_40, 0);
}

lv_obj_t *ui_make_badge(lv_obj_t *parent, const char *text, lv_color_t accent) {
    lv_obj_t *badge = lv_obj_create(parent);
    lv_obj_remove_style_all(badge);
    lv_obj_set_size(badge, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_color(badge, accent, 0);
    lv_obj_set_style_bg_opa(badge, LV_OPA_30, 0);
    lv_obj_set_style_border_width(badge, 1, 0);
    lv_obj_set_style_border_color(badge, accent, 0);
    lv_obj_set_style_radius(badge, 10, 0);
    lv_obj_set_style_pad_hor(badge, 8, 0);
    lv_obj_set_style_pad_ver(badge, 3, 0);
    lv_obj_clear_flag(badge, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *lbl = lv_label_create(badge);
    lv_label_set_text(lbl, text);
    lv_obj_set_style_text_font(lbl, &ru_font_12, 0);
    lv_obj_set_style_text_color(lbl, accent, 0);
    return badge;
}

lv_obj_t *ui_make_stat_tile(lv_obj_t *parent, const char *label, const char *value, lv_color_t accent) {
    lv_obj_t *tile = lv_obj_create(parent);
    lv_obj_remove_style_all(tile);
    lv_obj_set_size(tile, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    ui_style_panel(tile, lv_color_hex(UI_COLOR_BORDER));
    lv_obj_set_style_pad_all(tile, 6, 0);
    lv_obj_set_flex_flow(tile, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(tile, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(tile, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *lblLabel = lv_label_create(tile);
    lv_label_set_text(lblLabel, label);
    lv_obj_set_style_text_font(lblLabel, &ru_font_12, 0);
    lv_obj_add_style(lblLabel, &style_dim, 0);

    lv_obj_t *lblValue = lv_label_create(tile);
    lv_label_set_text(lblValue, value);
    lv_obj_set_style_text_font(lblValue, &ru_pixel_14, 0);
    lv_obj_set_style_text_color(lblValue, accent, 0);

    return tile;
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
