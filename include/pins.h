#pragma once
// Pin map for the "Cheap Yellow Display" ESP32-2432S028 (resistive touch
// revision). Widely used community pinout - shared by the single-micro-USB
// and dual-USB (micro-USB + USB-C) board revisions, since only the USB
// connector changes between them, not the GPIO wiring.

// --- TFT (ILI9341), SPI bus VSPI ---
#define CYD_TFT_MISO 12
#define CYD_TFT_MOSI 13
#define CYD_TFT_SCLK 14
#define CYD_TFT_CS   15
#define CYD_TFT_DC    2
#define CYD_TFT_RST  -1   // not connected, tied to EN
#define CYD_TFT_BL   21   // backlight, active HIGH

// --- Touch (XPT2046), shares VSPI bus with the TFT ---
#define CYD_TOUCH_CS  33
#define CYD_TOUCH_IRQ 36

// --- microSD card, separate SPI bus (HSPI) ---
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
