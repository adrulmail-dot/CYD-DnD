#pragma once
// Pin map for the "Cheap Yellow Display" ESP32-2432S028 (resistive touch
// revision, incl. the dual-USB "CYD-2USB" variant). Cross-checked against
// pr3y/Bruce's board config for this exact model (boards/CYD-2432S028) after
// this project's original assumptions (TFT on default VSPI, touch sharing
// that bus) turned out to be wrong on real hardware.

// --- TFT (ILI9341), forced onto the HSPI peripheral - see include/User_Setup.h ---
#define CYD_TFT_MISO 12
#define CYD_TFT_MOSI 13
#define CYD_TFT_SCLK 14
#define CYD_TFT_CS   15
#define CYD_TFT_DC    2
#define CYD_TFT_RST  -1   // not connected, tied to EN
#define CYD_TFT_BL   21   // backlight, active HIGH

// --- Touch (XPT2046) ---
// NOT on the TFT's SPI bus on this board: it has its own dedicated pins,
// read via bit-banged (software) SPI. See src/touch/CYD28_TouchscreenR.*
// (ported from pr3y/Bruce, itself from Paul Stoffregen's XPT2046_Touchscreen
// with CYD28-specific pins/calibration by Piotr Zapart).
#define CYD_TOUCH_CS   33
#define CYD_TOUCH_IRQ  36
#define CYD_TOUCH_MOSI 32
#define CYD_TOUCH_MISO 39
#define CYD_TOUCH_CLK  25

// --- microSD card, separate SPI bus (VSPI, since the TFT now owns HSPI) ---
#define CYD_SD_CS   5
#define CYD_SD_MOSI 23
#define CYD_SD_MISO 19
#define CYD_SD_SCK  18

// --- RGB status LED (common cathode, active LOW) ---
#define CYD_LED_R 4
#define CYD_LED_G 16
#define CYD_LED_B 17

// --- Misc ---
#define CYD_LDR_PIN 34   // light dependent resistor, ADC input
#define CYD_SPEAKER 26   // piezo speaker / buzzer
