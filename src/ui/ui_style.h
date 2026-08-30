#pragma once
#include <lvgl.h>
#include <Arduino.h>

// Screen ids for the bottom navigation bar / back-navigation.
enum ScreenId {
    SCREEN_HOME,
    SCREEN_BESTIARY_LIST,
    SCREEN_BESTIARY_DETAIL,
    SCREEN_CHARACTER,
    SCREEN_INVENTORY,
    SCREEN_SPELLS,
    SCREEN_SPELL_DETAIL,
};

void ui_init_styles();

// Shows one of the cached, singleton screens (built once, reused after).
void ui_show_screen(ScreenId id);

// Detail screens depend on a selection, so they are rebuilt fresh each time
// and the previous instance is deleted.
void ui_show_bestiary_detail(const String &slug);
void ui_show_spell_detail(const String &slug);

// Adds a slim bottom nav bar with Home/Bestiary/Character/Inventory/Spells
// buttons to `parent`, wired to ui_show_screen(). Call once per screen after
// building its content, sized to leave room above it (see NAV_BAR_HEIGHT).
void ui_add_nav_bar(lv_obj_t *parent);

#define NAV_BAR_HEIGHT 34

// Shared styles, initialized by ui_init_styles().
extern lv_style_t style_card;
extern lv_style_t style_title;
extern lv_style_t style_dim;
