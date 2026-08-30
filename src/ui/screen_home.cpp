#include "screen_home.h"
#include "ui_style.h"
#include "../app_state.h"
#include "../fonts/ru_fonts.h"
#include "version.h"

lv_obj_t *screen_home_create() {
    lv_obj_t *scr = ui_new_screen();

    lv_obj_t *title = lv_label_create(scr);
    lv_label_set_text(title, "D&D Бестиарий и заклинания");
    lv_obj_add_style(title, &style_title, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 8);

    lv_obj_t *subtitle = lv_label_create(scr);
    static char subtitleBuf[64];
    snprintf(subtitleBuf, sizeof(subtitleBuf), "%s, ур. %d", App.character.name.c_str(), App.character.level);
    lv_label_set_text(subtitle, subtitleBuf);
    lv_obj_add_style(subtitle, &style_dim, 0);
    lv_obj_align_to(subtitle, title, LV_ALIGN_OUT_BOTTOM_MID, 0, 4);

    lv_obj_t *grid = lv_obj_create(scr);
    lv_obj_remove_style_all(grid);
    lv_obj_set_size(grid, LV_PCT(100), 160);
    lv_obj_align(grid, LV_ALIGN_CENTER, 0, 4);
    lv_obj_set_flex_flow(grid, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_flex_align(grid, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(grid, LV_OBJ_FLAG_SCROLLABLE);

    struct Tile { const char *label; ScreenId id; lv_color_t color; };
    static const Tile tiles[] = {
        {"Бестиарий", SCREEN_BESTIARY_LIST, lv_color_hex(UI_COLOR_RED)},
        {"Персонаж", SCREEN_CHARACTER, lv_color_hex(UI_COLOR_BLUE)},
        {"Инвентарь", SCREEN_INVENTORY, lv_color_hex(UI_COLOR_AMBER)},
        {"Заклинания", SCREEN_SPELLS, lv_color_hex(UI_COLOR_PURPLE)},
    };
    for (const auto &t : tiles) {
        lv_obj_t *btn = lv_btn_create(grid);
        lv_obj_set_size(btn, 130, 68);
        ui_style_panel(btn, t.color);
        lv_obj_add_event_cb(btn, [](lv_event_t *e) {
            ScreenId id = (ScreenId)(intptr_t)lv_event_get_user_data(e);
            ui_show_screen(id);
        }, LV_EVENT_CLICKED, (void *)(intptr_t)t.id);
        lv_obj_t *lbl = lv_label_create(btn);
        lv_label_set_text(lbl, t.label);
        lv_obj_center(lbl);
        lv_obj_set_style_text_font(lbl, &ru_font_16, 0);
        lv_obj_set_style_text_color(lbl, t.color, 0);
    }

    lv_obj_t *statusRow = lv_obj_create(scr);
    lv_obj_remove_style_all(statusRow);
    lv_obj_set_size(statusRow, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(statusRow, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(statusRow, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(statusRow, 6, 0);
    lv_obj_clear_flag(statusRow, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_align(statusRow, LV_ALIGN_BOTTOM_MID, 0, -4);

    lv_obj_t *version = lv_label_create(statusRow);
    lv_label_set_text(version, "v" APP_VERSION);
    lv_obj_add_style(version, &style_dim, 0);
    lv_obj_set_style_text_font(version, &ru_font_12, 0);

    // Quick at-a-glance SD/data status: a card icon when the bestiary
    // actually loaded entries, a red cross when the card is missing or
    // the data files weren't found (see the diagnostic in main.cpp for
    // the detailed error screen version of this same check).
    bool sdOk = App.dataLoaded && App.bestiary.count() > 0;
    lv_obj_t *sdIcon = lv_label_create(statusRow);
    lv_label_set_text(sdIcon, sdOk ? LV_SYMBOL_SD_CARD : LV_SYMBOL_CLOSE);
    lv_obj_set_style_text_color(sdIcon, lv_color_hex(sdOk ? UI_COLOR_GREEN : UI_COLOR_RED), 0);
    lv_obj_set_style_text_font(sdIcon, &ru_font_12, 0);

    return scr;
}
