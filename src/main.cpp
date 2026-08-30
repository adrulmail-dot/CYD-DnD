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
#include "touch/CYD28_TouchscreenR.h"

static TFT_eSPI tft = TFT_eSPI();
static SPIClass sdSPI(VSPI); // TFT now owns HSPI (see User_Setup.h), so the
                              // SD card gets the other hardware SPI unit.

// Matches pr3y/Bruce's proven config for this exact board (see
// User_Setup.h and pins.h for the full story): rotation 3 (landscape,
// 320x240) is the orientation this hardware actually renders and reports
// touch coordinates for cleanly. A portrait UI can be layered back on top
// later via LVGL's software rotation now that the underlying raw pipeline
// is confirmed correct - shipping landscape first to validate that.
static const uint16_t SCREEN_W = 320;
static const uint16_t SCREEN_H = 240;
static const uint8_t DISPLAY_ROTATION = 3;

static CYD28_TouchR touch(SCREEN_W, SCREEN_H);

static lv_disp_draw_buf_t drawBuf;
static lv_color_t buf1[SCREEN_W * 30];
static lv_color_t buf2[SCREEN_W * 30];

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
    if (touch.touched()) {
        CYD28_TS_Point p = touch.getPointScaled();
        data->state = LV_INDEV_STATE_PR;
        data->point.x = p.x;
        data->point.y = p.y;
    } else {
        data->state = LV_INDEV_STATE_REL;
    }
}

static void initDisplayAndTouch() {
    tft.init();
    tft.setRotation(DISPLAY_ROTATION);
    pinMode(CYD_TFT_BL, OUTPUT);
    digitalWrite(CYD_TFT_BL, HIGH);

    touch.begin();
    touch.setRotation(DISPLAY_ROTATION);

    lv_init();
    lv_disp_draw_buf_init(&drawBuf, buf1, buf2, SCREEN_W * 30);

    static lv_disp_drv_t dispDrv;
    lv_disp_drv_init(&dispDrv);
    dispDrv.hor_res = SCREEN_W;
    dispDrv.ver_res = SCREEN_H;
    dispDrv.flush_cb = dispFlush;
    dispDrv.draw_buf = &drawBuf;
    lv_disp_drv_register(&dispDrv);

    static lv_indev_drv_t indevDrv;
    lv_indev_drv_init(&indevDrv);
    indevDrv.type = LV_INDEV_TYPE_POINTER;
    indevDrv.read_cb = touchRead;
    lv_indev_drv_register(&indevDrv);
}

static bool initSd() {
    sdSPI.begin(CYD_SD_SCK, CYD_SD_MISO, CYD_SD_MOSI, CYD_SD_CS);
    if (SD.begin(CYD_SD_CS, sdSPI)) {
        Serial.println("microSD mounted on VSPI (18/19/23, CS=5).");
        return true;
    }
    Serial.println("microSD not found. Check: card is FAT32, fully seated, "
                    "and sd_card_data/ was copied to its root.");
    return false;
}

void setup() {
    Serial.begin(115200);
    Serial.println("CYD D&D Bestiary & Spell Tracker v" APP_VERSION);
    initDisplayAndTouch();

    if (!initSd()) {
        lv_obj_t *scr = lv_obj_create(NULL);
        lv_obj_t *lbl = lv_label_create(scr);
        lv_label_set_text(lbl, "Ошибка: не найдена microSD\nСкопируйте sd_card_data/\nна карту и перезагрузите.");
        lv_label_set_long_mode(lbl, LV_LABEL_LONG_WRAP);
        lv_obj_set_width(lbl, 280);
        lv_obj_center(lbl);
        lv_scr_load(scr);
        return;
    }

    App.begin();

    // The SD card can mount fine as a filesystem while the actual data
    // files are missing (most commonly: sd_card_data/ was copied onto the
    // card as a subfolder instead of its contents going to the card's
    // root) - that used to fail silently into an empty-looking app. Catch
    // it here with an actionable message instead.
    if (App.bestiary.count() == 0) {
        char buf[220];
        snprintf(buf, sizeof(buf),
                 "Ошибка: данные не найдены на SD\n"
                 "(бестиарий: %u, заклинания: %u).\n\n"
                 "Проверьте, что файлы из sd_card_data/\n"
                 "лежат прямо в КОРНЕ карты, а не во\n"
                 "вложенной папке (bestiary_index.jsonl\n"
                 "должен быть на самом верхнем уровне).",
                 (unsigned)App.bestiary.count(), (unsigned)App.spells.count());
        lv_obj_t *scr = lv_obj_create(NULL);
        lv_obj_t *lbl = lv_label_create(scr);
        lv_label_set_text(lbl, buf);
        lv_label_set_long_mode(lbl, LV_LABEL_LONG_WRAP);
        lv_obj_set_width(lbl, 300);
        lv_obj_center(lbl);
        lv_scr_load(scr);
        return;
    }

    ui_init_styles();
    ui_show_screen(SCREEN_HOME);
}

void loop() {
    lv_timer_handler();
    delay(5);
}
