// TFT_eSPI configuration for the CYD ESP32-2432S028 (ILI9341, 320x240 SPI).
// Force-included into every translation unit via platformio.ini build_flags.
//
// Cross-checked against pr3y/Bruce's proven config for this exact board
// (boards/CYD-2432S028/CYD-2432S028.ini, [env:CYD-2432S028] / [env:CYD-2USB])
// after the original settings here (ILI9341_DRIVER, default VSPI, touch
// sharing the TFT bus) produced garbled text rendering and non-functional
// touch on the real device. Two concrete differences from Bruce's config:
//  - ILI9341_2_DRIVER: an alternate ILI9341 init sequence (different
//    power/VCOM/gamma register values) that this particular clone panel
//    needs - see https://github.com/Bodmer/TFT_eSPI/issues/1172.
//  - USE_HSPI_PORT: puts the TFT on the HSPI peripheral instead of the
//    default VSPI (the microSD card uses VSPI instead now - see main.cpp).
// Touch is NOT on this bus at all on this board (see pins.h) so there is
// no TOUCH_CS here; it's read via the separate driver in src/touch/.
#pragma once

#include "pins.h"

#define ILI9341_2_DRIVER
#define USE_HSPI_PORT

#define TFT_MISO CYD_TFT_MISO
#define TFT_MOSI CYD_TFT_MOSI
#define TFT_SCLK CYD_TFT_SCLK
#define TFT_CS   CYD_TFT_CS
#define TFT_DC   CYD_TFT_DC
#define TFT_RST  CYD_TFT_RST
#define TFT_BL   CYD_TFT_BL
#define TFT_BACKLIGHT_ON HIGH

#define LOAD_GLCD
#define LOAD_FONT2
#define LOAD_FONT4
#define LOAD_FONT6
#define LOAD_FONT7
#define LOAD_FONT8
#define LOAD_GFXFF
#define SMOOTH_FONT

// Matches Bruce's proven values for this board - the earlier corruption
// wasn't actually an SPI-clock issue (lowering it to 20MHz alone didn't
// help), so back to the standard 40MHz now that the real cause (wrong
// driver/SPI port) is fixed.
#define SPI_FREQUENCY       40000000
#define SPI_READ_FREQUENCY  16000000
