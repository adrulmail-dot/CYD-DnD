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

// Retro dark-RPG palette (matches the companion web simulator's look: dark
// navy background, amber/gold primary accent, per-section category colors).
#define UI_COLOR_BG        0x0F172A // page background (slate-950)
#define UI_COLOR_PANEL     0x1E293B // card/panel background (slate-800)
#define UI_COLOR_BORDER    0x334155 // neutral panel border (slate-700)
#define UI_COLOR_AMBER     0xF59E0B // primary accent: CTAs, XP, bestiary
#define UI_COLOR_RED       0xEF4444 // danger / bestiary category
#define UI_COLOR_BLUE      0x38BDF8 // character category
#define UI_COLOR_PURPLE    0xC084FC // spells/magic category
#define UI_COLOR_GREEN     0x22C55E // HP / success
#define UI_COLOR_TEXT_DIM  0x94A3B8 // secondary text (slate-400)

// Applies the dark theme (colors above + our Cyrillic default font) to the
// given display. Call once, right after lv_disp_drv_register(), before any
// screen is created.
void ui_apply_dark_theme(lv_disp_t *disp);

// Creates a new screen root object with our exact UI_COLOR_BG background.
// LVGL's own dark theme uses a slightly different default screen color
// (0x15171A), so every screen sets this explicitly rather than relying on
// the theme default, to match the reference navy exactly.
lv_obj_t *ui_new_screen();

void ui_init_styles();

// Gives `obj` the shared dark "panel/card" look: panel background, a
// category-colored border, small radius and a soft drop shadow for depth.
void ui_style_panel(lv_obj_t *obj, lv_color_t accent);

// Creates a small rounded pill label (e.g. "CR 13", "ABERRATION") tinted
// with `accent`, using the normal (non-pixel) font so arbitrary-length
// tags never overflow.
lv_obj_t *ui_make_badge(lv_obj_t *parent, const char *text, lv_color_t accent);

// Creates a small vertical stat tile: a dim uppercase label on top and a
// large value in the retro pixel font below, tinted with `accent`. Use only
// for short, bounded values (numbers, "AC 18", "30 ft.") - see ru_pixel_14's
// width caveat in fonts/ru_fonts.h.
lv_obj_t *ui_make_stat_tile(lv_obj_t *parent, const char *label, const char *value, lv_color_t accent);

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
