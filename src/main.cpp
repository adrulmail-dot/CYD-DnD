// TEMPORARY rotation diagnostic build. This replaces the real app for one
// flash cycle so we can see, on the actual hardware, which of TFT_eSPI's
// four rotation values gives a clean, correctly-oriented, portrait image on
// this specific board. Cycles through rotation 0/1/2/3 every 4 seconds,
// each time filling the screen with four colored quadrants + a big
// "ROT n / WxH" label (ASCII only, GLCD font, so this doesn't depend on the
// Cyrillic font work happening in parallel). No LVGL, no SD, no touch -
// deliberately minimal so a garbled/blank result can only mean the
// rotation value itself, nothing else in the app stack.
//
// Report back which rotation number showed:
//  - readable, non-garbled text
//  - right-side up when the board is held with USB at the top
//  - top-left quadrant actually in the top-left, etc.
// Once known, this file will be restored to the real app with that
// rotation hardcoded (see git history / src/main.cpp before this commit).
#include <Arduino.h>
#include <TFT_eSPI.h>
#include "pins.h"

static TFT_eSPI tft = TFT_eSPI();

static void drawRotationTest(uint8_t rot) {
    tft.setRotation(rot);
    int w = tft.width();
    int h = tft.height();
    int hw = w / 2, hh = h / 2;

    tft.fillScreen(TFT_BLACK);
    tft.fillRect(0, 0, hw, hh, TFT_RED);          // top-left
    tft.fillRect(hw, 0, w - hw, hh, TFT_GREEN);   // top-right
    tft.fillRect(0, hh, hw, h - hh, TFT_BLUE);    // bottom-left
    tft.fillRect(hw, hh, w - hw, h - hh, TFT_YELLOW); // bottom-right

    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(3);
    tft.setCursor(4, 4);
    tft.printf("ROT %d", rot);
    tft.setTextSize(2);
    tft.setCursor(4, 32);
    tft.printf("%dx%d", w, h);
    tft.setTextSize(1);
    tft.setCursor(4, h - 18);
    tft.print("TL=RED TR=GREEN");
    tft.setCursor(4, h - 9);
    tft.print("BL=BLUE BR=YELLOW");
}

void setup() {
    Serial.begin(115200);
    delay(200);
    Serial.println("=== ROTATION DIAGNOSTIC BUILD ===");
    Serial.println("Cycling rotation 0,1,2,3 every 4s. Report which one is");
    Serial.println("clean/correct (text readable, right-side up, USB at top).");

    tft.init();
    pinMode(CYD_TFT_BL, OUTPUT);
    digitalWrite(CYD_TFT_BL, HIGH);
}

void loop() {
    static uint8_t rot = 0;
    drawRotationTest(rot);
    Serial.print("Showing ROT ");
    Serial.print(rot);
    Serial.print(" (");
    Serial.print(tft.width());
    Serial.print("x");
    Serial.print(tft.height());
    Serial.println(")");
    rot = (rot + 1) % 4;
    delay(4000);
}
