#pragma once
#include <lvgl.h>
#include <Arduino.h>

// Decodes a JPEG file from the microSD card into a heap RGB565 buffer
// wrapped as an lv_img_dsc_t, for use as an lv_img widget's source. Only
// ~12 of the 322 SRD monsters ship with a real illustration (see
// sd_card_data/img/); everything else falls back to a drawn placeholder
// (see bestiary_placeholder in screen_bestiary.cpp).
//
// Returns nullptr if the file is missing or decoding fails. The caller
// owns the returned descriptor; attach it to the lv_img's LV_EVENT_DELETE
// via img_loader_attach_cleanup() so the buffer is freed automatically
// when the image widget (and its parent screen) is destroyed.
lv_img_dsc_t *load_jpeg_from_sd(const String &path, uint16_t maxWidth = 220);
void img_loader_attach_cleanup(lv_obj_t *img_obj, lv_img_dsc_t *dsc);
