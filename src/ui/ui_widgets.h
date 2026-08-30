#pragma once
#include <lvgl.h>
#include <Arduino.h>

// Small reusable widgets shared by the Character/Inventory/Spells screens,
// built around plain data pointers (all app state lives in one global
// AppState instance with a stable address, so rows can bind directly to a
// field's address instead of going through getter/setter callbacks).

// A "label  [-] value [+]" row bound directly to an int field. Steps by
// `step` and clamps to [minV, maxV]. Returns the row container.
lv_obj_t *ui_add_int_stepper_row(lv_obj_t *parent, const char *label, int *value,
                                  int minV, int maxV, int step = 1);

// Same, but for a `long` field (used for currency, which can get large).
lv_obj_t *ui_add_long_stepper_row(lv_obj_t *parent, const char *label, long *value,
                                   long minV, long maxV, long step = 1);

// A "label  value>" row that opens a full-screen keyboard editor for a
// String field when tapped.
lv_obj_t *ui_add_text_edit_row(lv_obj_t *parent, const char *label, String *value);

// A "label  value>" row that opens a full-screen keyboard editor for a
// multi-line String field (notes, features, ...).
lv_obj_t *ui_add_textarea_edit_row(lv_obj_t *parent, const char *label, String *value);

// A single checkbox bound directly to a bool field.
lv_obj_t *ui_add_checkbox(lv_obj_t *parent, const char *label, bool *value);
