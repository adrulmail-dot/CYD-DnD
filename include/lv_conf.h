/* Minimal LVGL 8.3 configuration for the CYD ESP32-2432S028 (no PSRAM). */
#ifndef LV_CONF_H
#define LV_CONF_H

#include <stdint.h>

#define LV_COLOR_DEPTH 16
#define LV_COLOR_16_SWAP 0

#define LV_MEM_CUSTOM 0
#define LV_MEM_SIZE (48U * 1024U)

#define LV_DISP_DEF_REFR_PERIOD 30
#define LV_INDEV_DEF_READ_PERIOD 30

#define LV_TICK_CUSTOM 1
#define LV_TICK_CUSTOM_INCLUDE "Arduino.h"
#define LV_TICK_CUSTOM_SYS_TIME_EXPR (millis())

#define LV_USE_PERF_MONITOR 0
#define LV_USE_MEM_MONITOR 0

/* Stock Montserrat fonts only cover Latin glyphs - the app's UI text is
 * Russian, so it uses custom-built fonts (Montserrat + Cyrillic + the LVGL
 * symbol icons) declared globally here instead. See src/fonts/ru_fonts.h
 * and tools/build_fonts.sh. */
#define LV_FONT_MONTSERRAT_12 0
#define LV_FONT_MONTSERRAT_14 0
#define LV_FONT_MONTSERRAT_16 0
#define LV_FONT_MONTSERRAT_18 0
#define LV_FONT_CUSTOM_DECLARE LV_FONT_DECLARE(ru_font_12) LV_FONT_DECLARE(ru_font_14) LV_FONT_DECLARE(ru_font_16) LV_FONT_DECLARE(ru_font_18) LV_FONT_DECLARE(ru_pixel_14)
#define LV_FONT_DEFAULT &ru_font_14

#define LV_USE_LOG 0

#define LV_USE_ARC        1
#define LV_USE_BAR        1
#define LV_USE_BTN        1
#define LV_USE_BTNMATRIX  1
#define LV_USE_CANVAS     1
#define LV_USE_CHECKBOX   1
#define LV_USE_DROPDOWN   1
#define LV_USE_IMG        1
#define LV_USE_LABEL      1
#define LV_USE_LINE       1
#define LV_USE_ROLLER     1
#define LV_USE_SLIDER     1
#define LV_USE_SWITCH     1
#define LV_USE_TEXTAREA   1
#define LV_USE_TABLE      1

#define LV_USE_KEYBOARD   1
#define LV_USE_LIST       1
#define LV_USE_MSGBOX     1
#define LV_USE_TABVIEW    1
#define LV_USE_WIN        0
#define LV_USE_SPINBOX    1

#define LV_USE_THEME_DEFAULT 1
#define LV_THEME_DEFAULT_DARK 0
#define LV_THEME_DEFAULT_GROW 1
#define LV_THEME_DEFAULT_TRANSITION_TIME 80

#define LV_USE_FLEX 1
#define LV_USE_GRID 1

#define LV_SPRINTF_CUSTOM 0
#define LV_USE_ASSERT_NULL 1
#define LV_USE_ASSERT_MALLOC 1

#endif /*LV_CONF_H*/
