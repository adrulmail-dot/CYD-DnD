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
