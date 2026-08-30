#!/usr/bin/env bash
# Regenerates src/fonts/ru_font_*.c - Montserrat Medium + Cyrillic (0x400-0x45F)
# + the LVGL built-in symbol icons (LV_SYMBOL_*), since LVGL's stock
# lv_font_montserrat_* fonts only ship Latin glyphs and render Russian text
# as empty boxes.
#
# Requires: npm install -g lv_font_conv, and a local checkout of
# https://github.com/lvgl/lvgl (for its bundled Montserrat-Medium.ttf and
# FontAwesome5-Solid+Brands+Regular.woff source fonts).
#
# Usage: bash tools/build_fonts.sh /path/to/lvgl/checkout
set -e
LVGL_DIR="${1:?Usage: build_fonts.sh /path/to/lvgl/checkout}"
OUT_DIR="$(dirname "$0")/../src/fonts"

# Exact codepoint list LVGL itself uses for its built-in symbol icons
# (see lvgl/scripts/built_in_font/built_in_font_gen.py).
SYMS="61441,61448,61451,61452,61452,61453,61457,61459,61461,61465,61468,61473,61478,61479,61480,61502,61507,61512,61515,61516,61517,61521,61522,61523,61524,61543,61544,61550,61552,61553,61556,61559,61560,61561,61563,61587,61589,61636,61637,61639,61641,61664,61671,61674,61683,61724,61732,61787,61931,62016,62017,62018,62019,62020,62087,62099,62212,62189,62810,63426,63650"

for SIZE in 12 14 16 18; do
    lv_font_conv --no-compress --no-prefilter --bpp 4 --size "$SIZE" \
        --font "$LVGL_DIR/scripts/built_in_font/Montserrat-Medium.ttf" \
        -r 0x20-0x7F,0xB0,0x2022,0x400-0x45F \
        --font "$LVGL_DIR/scripts/built_in_font/FontAwesome5-Solid+Brands+Regular.woff" \
        -r "$SYMS" \
        --format lvgl -o "$OUT_DIR/ru_font_$SIZE.c" --force-fast-kern-format
    echo "Generated $OUT_DIR/ru_font_$SIZE.c"
done

# Retro "Press Start 2P" pixel font (14px only) for short, bounded UI text -
# stat-tile numbers and CR/level badges in the retro-RPG theme. NOT safe for
# labels, nav bar text or body copy: measured ~1.6x wider per Cyrillic glyph
# than ru_font_* at the same size, so long/variable-length strings overflow.
# (Note: the popular "Pixelify Sans" font was tried first and rejected - it is
# missing the Cyrillic capital letters О and П entirely, which would silently
# render as tofu boxes in real Russian words.)
PRESS_START_TTF="${2:?Usage: build_fonts.sh /path/to/lvgl/checkout /path/to/PressStart2P-Regular.ttf}"
lv_font_conv --no-compress --no-prefilter --bpp 4 --size 14 \
    --font "$PRESS_START_TTF" \
    -r 0x20-0x7F,0xB0,0x2022,0x400-0x45F \
    --font "$LVGL_DIR/scripts/built_in_font/FontAwesome5-Solid+Brands+Regular.woff" \
    -r "$SYMS" \
    --format lvgl -o "$OUT_DIR/ru_pixel_14.c" --force-fast-kern-format
echo "Generated $OUT_DIR/ru_pixel_14.c"
