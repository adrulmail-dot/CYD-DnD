// TFT_eSPI configuration for the CYD ESP32-2432S028 (ILI9341, 320x240 SPI).
// Force-included into every translation unit via platformio.ini build_flags.
#pragma once

#include "pins.h"

#define ILI9341_DRIVER

#define TFT_MISO CYD_TFT_MISO
#define TFT_MOSI CYD_TFT_MOSI
#define TFT_SCLK CYD_TFT_SCLK
#define TFT_CS   CYD_TFT_CS
#define TFT_DC   CYD_TFT_DC
#define TFT_RST  CYD_TFT_RST
#define TFT_BL   CYD_TFT_BL
#define TFT_BACKLIGHT_ON HIGH

// XPT2046 touch shares the TFT SPI bus on this board.
#define TOUCH_CS CYD_TOUCH_CS

#define LOAD_GLCD
#define LOAD_FONT2
#define LOAD_FONT4
#define LOAD_FONT6
#define LOAD_FONT7
#define LOAD_FONT8
#define LOAD_GFXFF
#define SMOOTH_FONT

// 40MHz produced corrupted/garbled rendering specifically for complex,
// rapidly-changing pixel data (text) while large solid-color fills looked
// fine on this board (uniform data masks single-bit glitches; text data
// doesn't) - a classic signal-integrity symptom on cheap clone wiring.
// The font data and LVGL's own text rasterizer were separately verified
// correct on a PC build outside the ESP32, which points at the SPI link
// itself. Trying a safer clock here as the next hypothesis.
#define SPI_FREQUENCY       20000000
#define SPI_READ_FREQUENCY  16000000
#define SPI_TOUCH_FREQUENCY 2500000
