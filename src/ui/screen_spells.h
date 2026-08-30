#pragma once
#include <lvgl.h>
#include <Arduino.h>

lv_obj_t *screen_spells_list_create();
lv_obj_t *screen_spell_detail_create(const String &slug);
