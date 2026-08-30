#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <TFT_eSPI.h>
#include <lvgl.h>

#include "pins.h"
#include "version.h"
#include "app_state.h"
#include "ui/ui_style.h"
#include "ui/screen_home.h"

static TFT_eSPI tft = TFT_eSPI();
static SPIClass sdSPI(HSPI);

// This exact board's ILI9341 panel renders garbled/transposed content at
// TFT_eSPI's default portrait rotation (0) - a known clone-hardware quirk
// where the glass is bonded 90 degrees off from the controller's assumed
// orientation. Rotation 1 (which TFT_eSPI treats as landscape, 320x240)
// gives a clean, non-garbled raw image on this unit; LVGL's software
// rotation (see dispDrv.rotated below) then turns that clean landscape
// buffer back into the portrait UI the app is built for, with
// touchRead() doing the matching coordinate conversion.
// PHYS_* = the raw panel's own coordinate space after setRotation(1).
static const uint16_t PHYS_W = 320;
static const uint16_t PHYS_H = 240;

static lv_disp_draw_buf_t drawBuf;
static lv_color_t buf1[PHYS_W * 30];
static lv_color_t buf2[PHYS_W * 30];

static void dispFlush(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_p) {
    uint32_t w = area->x2 - area->x1 + 1;
    uint32_t h = area->y2 - area->y1 + 1;

    tft.startWrite();
    tft.setAddrWindow(area->x1, area->y1, w, h);
    tft.pushColors((uint16_t *)&color_p->full, w * h, true);
    tft.endWrite();

    lv_disp_flush_ready(drv);
}

static void touchRead(lv_indev_drv_t *drv, lv_indev_data_t *data) {
    uint16_t px, py;
    bool touched = tft.getTouch(&px, &py);
    if (touched) {
        // tft.getTouch() reports coordinates in the raw PHYS_W x PHYS_H
        // panel space (see rotation comment above). LVGL's sw_rotate only
        // transforms what gets drawn, not touch input, so the inverse of
        // that same 90-degree rotation has to be applied here by hand -
        // derived from LVGL's own draw_buf_rotate_90() so it matches
        // dispDrv.rotated = LV_DISP_ROT_90 exactly.
        // If the UI turns out upside-down/mirrored once this is on real
        // hardware, switch dispDrv.rotated to LV_DISP_ROT_270 below and
        // use this formula here instead:
        //   data->point.x = py;
        //   data->point.y = (PHYS_W - 1) - px;
        data->state = LV_INDEV_STATE_PR;
        data->point.x = (PHYS_H - 1) - py;
        data->point.y = px;
    } else {
        data->state = LV_INDEV_STATE_REL;
    }
}

static void initDisplayAndTouch() {
    tft.init();
    tft.setRotation(1); // landscape, 320x240. If the image still looks
                         // rotated/mirrored on your unit, try 3 here instead
                         // (0 and 2 are the two portrait options).
    pinMode(CYD_TFT_BL, OUTPUT);
    digitalWrite(CYD_TFT_BL, HIGH);

    // Calibration data for the resistive XPT2046 touch panel. These are the
    // commonly published defaults for the CYD ESP32-2432S028; run the
    // TFT_eSPI touch calibration sketch once and paste your own values here
    // if touch coordinates feel off on your particular unit.
    uint16_t calData[5] = {200, 3700, 200, 3700, 7};
    tft.setTouch(calData);

    lv_init();
    lv_disp_draw_buf_init(&drawBuf, buf1, buf2, PHYS_W * 30);

    static lv_disp_drv_t dispDrv;
    lv_disp_drv_init(&dispDrv);
    dispDrv.hor_res = PHYS_W;   // raw panel resolution - LVGL derives the
    dispDrv.ver_res = PHYS_H;   // logical 240x320 portrait size from this
                                // plus `rotated` below (lv_disp_get_hor_res()
                                // / get_ver_res() swap them for ROT_90/270).
    dispDrv.flush_cb = dispFlush;
    dispDrv.draw_buf = &drawBuf;
    dispDrv.sw_rotate = 1;
    dispDrv.rotated = LV_DISP_ROT_90;
    lv_disp_drv_register(&dispDrv);

    static lv_indev_drv_t indevDrv;
    lv_indev_drv_init(&indevDrv);
    indevDrv.type = LV_INDEV_TYPE_POINTER;
    indevDrv.read_cb = touchRead;
    lv_indev_drv_register(&indevDrv);
}

static bool initSd() {
    // Strategy 1: SD card on its own SPI bus (HSPI) - the most commonly
    // published wiring for this board (SCK=18, MISO=19, MOSI=23, CS=5).
    sdSPI.begin(CYD_SD_SCK, CYD_SD_MISO, CYD_SD_MOSI, CYD_SD_CS);
    if (SD.begin(CYD_SD_CS, sdSPI)) {
        Serial.println("microSD mounted on dedicated HSPI bus (18/19/23, CS=5).");
        return true;
    }
    SD.end();

    // Strategy 2: some CYD board revisions instead wire the SD card onto
    // the *same* SPI bus as the TFT/touch (VSPI: 12/13/14), differing only
    // by chip-select (still CS=5). Retry there before giving up.
    if (SD.begin(CYD_SD_CS, TFT_eSPI::getSPIinstance())) {
        Serial.println("microSD mounted sharing the TFT's VSPI bus (12/13/14, CS=5).");
        return true;
    }

    Serial.println("microSD not found on either the dedicated HSPI bus or the "
                    "TFT's shared VSPI bus. Check: card is FAT32, fully seated, "
                    "and sd_card_data/ was copied to its root.");
    return false;
}

void setup() {
    Serial.begin(115200);
    Serial.println("CYD D&D Bestiary & Spell Tracker v" APP_VERSION);
    initDisplayAndTouch();

    if (!initSd()) {
        Serial.println("microSD not found - insert a card with sd_card_data/ copied to its root.");
        lv_obj_t *scr = lv_obj_create(NULL);
        lv_obj_t *lbl = lv_label_create(scr);
        lv_label_set_text(lbl, "Ошибка: не найдена microSD\nСкопируйте sd_card_data/\nна карту и перезагрузите.");
        lv_label_set_long_mode(lbl, LV_LABEL_LONG_WRAP);
        lv_obj_set_width(lbl, 220);
        lv_obj_center(lbl);
        lv_scr_load(scr);
        return;
    }

    App.begin();
    ui_init_styles();
    ui_show_screen(SCREEN_HOME);
}

void loop() {
    lv_timer_handler();
    delay(5);
}
