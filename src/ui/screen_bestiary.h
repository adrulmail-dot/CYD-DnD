#pragma once
#include <lvgl.h>
#include <Arduino.h>

lv_obj_t *screen_bestiary_list_create();
lv_obj_t *screen_bestiary_detail_create(const String &slug);
