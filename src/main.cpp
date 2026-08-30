#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <TFT_eSPI.h>
#include <lvgl.h>

#include "pins.h"
#include "app_state.h"
#include "ui/ui_style.h"
#include "ui/screen_home.h"

static TFT_eSPI tft = TFT_eSPI();
static SPIClass sdSPI(HSPI);

static const uint16_t SCREEN_W = 240;
static const uint16_t SCREEN_H = 320;

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
    uint16_t x, y;
    bool touched = tft.getTouch(&x, &y);
    if (touched) {
        data->state = LV_INDEV_STATE_PR;
        data->point.x = x;
        data->point.y = y;
    } else {
        data->state = LV_INDEV_STATE_REL;
    }
}

static void initDisplayAndTouch() {
    tft.init();
    tft.setRotation(0); // portrait, 240x320 - matches the CYD's default orientation
    pinMode(CYD_TFT_BL, OUTPUT);
    digitalWrite(CYD_TFT_BL, HIGH);

    // Calibration data for the resistive XPT2046 touch panel. These are the
    // commonly published defaults for the CYD ESP32-2432S028; run the
    // TFT_eSPI touch calibration sketch once and paste your own values here
    // if touch coordinates feel off on your particular unit.
    uint16_t calData[5] = {200, 3700, 200, 3700, 7};
    tft.setTouch(calData);

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
    return SD.begin(CYD_SD_CS, sdSPI);
}

void setup() {
    Serial.begin(115200);
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
