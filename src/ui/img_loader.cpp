#include "img_loader.h"
#include <SD.h>
#include <TJpg_Decoder.h>

// TJpg_Decoder only supports one active output callback/target at a time,
// so we stash the destination buffer in file-scope statics for the
// duration of a single decode call (load_jpeg_from_sd is not reentrant -
// only call it from the LVGL/UI thread, never from an ISR or other task).
static uint16_t *s_buf = nullptr;
static uint16_t s_w = 0, s_h = 0;

static bool tjpgOutputCb(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t *bitmap) {
    if (!s_buf) return false;
    for (uint16_t row = 0; row < h; row++) {
        int destY = y + row;
        if (destY < 0 || destY >= s_h) continue;
        uint16_t *dstRow = s_buf + (destY * s_w);
        uint16_t *srcRow = bitmap + (row * w);
        for (uint16_t col = 0; col < w; col++) {
            int destX = x + col;
            if (destX < 0 || destX >= s_w) continue;
            dstRow[destX] = srcRow[col];
        }
    }
    return true;
}

lv_img_dsc_t *load_jpeg_from_sd(const String &path, uint16_t maxWidth) {
    if (!SD.exists(path)) return nullptr;
    File f = SD.open(path, FILE_READ);
    if (!f) return nullptr;
    size_t size = f.size();
    uint8_t *jpgBuf = new (std::nothrow) uint8_t[size];
    if (!jpgBuf) {
        f.close();
        return nullptr;
    }
    f.read(jpgBuf, size);
    f.close();

    uint16_t origW = 0, origH = 0;
    TJpgDec.setSwapBytes(false); // keep LVGL-native byte order; the display
                                  // flush callback performs the final swap.
    if (TJpgDec.getJpgSize(&origW, &origH, jpgBuf, size) != 0 || origW == 0 || origH == 0) {
        delete[] jpgBuf;
        return nullptr;
    }

    uint8_t scale = 1;
    while ((origW / scale) > maxWidth && scale < 8) scale *= 2;
    TJpgDec.setJpgScale(scale);

    s_w = origW / scale;
    s_h = origH / scale;
    s_buf = new (std::nothrow) uint16_t[(size_t)s_w * s_h];
    if (!s_buf) {
        delete[] jpgBuf;
        s_w = s_h = 0;
        return nullptr;
    }

    TJpgDec.setCallback(tjpgOutputCb);
    bool ok = (TJpgDec.drawJpg(0, 0, jpgBuf, size) == 0);
    delete[] jpgBuf;

    if (!ok) {
        delete[] s_buf;
        s_buf = nullptr;
        return nullptr;
    }

    lv_img_dsc_t *dsc = new (std::nothrow) lv_img_dsc_t();
    if (!dsc) {
        delete[] s_buf;
        s_buf = nullptr;
        return nullptr;
    }
    dsc->header.always_zero = 0;
    dsc->header.w = s_w;
    dsc->header.h = s_h;
    dsc->header.cf = LV_IMG_CF_TRUE_COLOR;
    dsc->data_size = (uint32_t)s_w * s_h * 2;
    dsc->data = reinterpret_cast<const uint8_t *>(s_buf);

    s_buf = nullptr; // ownership now belongs to dsc->data
    s_w = s_h = 0;
    return dsc;
}

void img_loader_attach_cleanup(lv_obj_t *img_obj, lv_img_dsc_t *dsc) {
    lv_obj_add_event_cb(img_obj, [](lv_event_t *e) {
        auto *d = static_cast<lv_img_dsc_t *>(lv_event_get_user_data(e));
        delete[] reinterpret_cast<const uint16_t *>(d->data);
        delete d;
    }, LV_EVENT_DELETE, dsc);
}
