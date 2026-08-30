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

// A raw-TFT_eSPI diagnostic (colored quadrants cycling through all 4
// rotation values, no LVGL involved) on the actual board confirmed that
// rotation 0 (native portrait, 240x320) fills the screen correctly with
// everything in the right place; rotations 1 and 3 (which use ILI9341's
// MV/row-column-exchange bit for landscape) come out compressed/sheared
// and don't fully repaint between frames on this unit. So: plain native
// portrait, no LVGL software rotation, no touch coordinate remapping.
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

#if 0 // touch temporarily disabled, see initDisplayAndTouch()
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
#endif

static void initDisplayAndTouch() {
    tft.init();
    tft.setRotation(0); // portrait, 240x320 - confirmed correct on this
                         // board by the raw-TFT rotation diagnostic.
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

    // TEMPORARILY DISABLED: touch never registers a press on this board
    // (getTouch() always returns false) and disabling it is being tested
    // as a fix for the text-rendering corruption too - tft.getTouch()
    // does up to 5 SPI probes every ~30ms, each with an internal
    // wait-for-pressure-to-stabilize loop; if the touch chip isn't
    // actually wired/responding the way XPT2046 is expected to, those
    // probes could be leaving the shared SPI bus in a bad state before
    // the next display write. Re-enable once touch hardware is confirmed
    // working (see README).
    // static lv_indev_drv_t indevDrv;
    // lv_indev_drv_init(&indevDrv);
    // indevDrv.type = LV_INDEV_TYPE_POINTER;
    // indevDrv.read_cb = touchRead;
    // lv_indev_drv_register(&indevDrv);
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
