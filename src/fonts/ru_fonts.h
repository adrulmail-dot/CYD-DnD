#pragma once
#include <lvgl.h>

// Custom fonts (Montserrat Medium + Cyrillic 0x400-0x45F + the LVGL built-in
// symbol icons) generated with lv_font_conv, because LVGL's stock
// lv_font_montserrat_* fonts only cover Latin - Russian UI text renders as
// tofu boxes with them. See tools/build_fonts.sh to regenerate.
LV_FONT_DECLARE(ru_font_12)
LV_FONT_DECLARE(ru_font_14)
LV_FONT_DECLARE(ru_font_16)
LV_FONT_DECLARE(ru_font_18)

// Retro "Press Start 2P" pixel font, same Cyrillic+symbol coverage. Reserved
// for short, bounded strings only (stat numbers, CR/level badges) - measured
// ~1.6x wider per glyph than ru_font_* at the same size, so it overflows
// labels, nav bar text and anything else of variable/unbounded length.
LV_FONT_DECLARE(ru_pixel_14)
