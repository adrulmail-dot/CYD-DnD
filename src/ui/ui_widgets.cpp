#include "ui_widgets.h"

// ---- shared row layout helpers ----------------------------------------

static lv_obj_t *makeRow(lv_obj_t *parent, const char *label, lv_obj_t **outLabel) {
    lv_obj_t *row = lv_obj_create(parent);
    lv_obj_remove_style_all(row);
    lv_obj_set_size(row, LV_PCT(100), 30);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *lbl = lv_label_create(row);
    lv_label_set_text(lbl, label);
    if (outLabel) *outLabel = lbl;
    return row;
}

// ---- int / long steppers ------------------------------------------------

struct IntStepperCtx { int *value; int minV, maxV, step; lv_obj_t *valueLabel; };
struct LongStepperCtx { long *value; long minV, maxV, step; lv_obj_t *valueLabel; };

static void intStepperRefresh(IntStepperCtx *ctx) {
    lv_label_set_text_fmt(ctx->valueLabel, "%d", *ctx->value);
}
static void longStepperRefresh(LongStepperCtx *ctx) {
    lv_label_set_text_fmt(ctx->valueLabel, "%ld", *ctx->value);
}

lv_obj_t *ui_add_int_stepper_row(lv_obj_t *parent, const char *label, int *value,
                                  int minV, int maxV, int step) {
    lv_obj_t *row = makeRow(parent, label, nullptr);

    lv_obj_t *controls = lv_obj_create(row);
    lv_obj_remove_style_all(controls);
    lv_obj_set_size(controls, 120, 28);
    lv_obj_set_flex_flow(controls, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(controls, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(controls, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *minus = lv_btn_create(controls);
    lv_obj_set_size(minus, 28, 26);
    lv_obj_t *minusLbl = lv_label_create(minus);
    lv_label_set_text(minusLbl, "-");
    lv_obj_center(minusLbl);

    lv_obj_t *valueLbl = lv_label_create(controls);
    lv_obj_set_width(valueLbl, 40);
    lv_obj_set_style_text_align(valueLbl, LV_TEXT_ALIGN_CENTER, 0);

    lv_obj_t *plus = lv_btn_create(controls);
    lv_obj_set_size(plus, 28, 26);
    lv_obj_t *plusLbl = lv_label_create(plus);
    lv_label_set_text(plusLbl, "+");
    lv_obj_center(plusLbl);

    auto *ctx = new IntStepperCtx{value, minV, maxV, step, valueLbl};
    intStepperRefresh(ctx);

    lv_obj_add_event_cb(minus, [](lv_event_t *e) {
        auto *c = static_cast<IntStepperCtx *>(lv_event_get_user_data(e));
        *c->value -= c->step;
        if (*c->value < c->minV) *c->value = c->minV;
        intStepperRefresh(c);
    }, LV_EVENT_CLICKED, ctx);
    lv_obj_add_event_cb(plus, [](lv_event_t *e) {
        auto *c = static_cast<IntStepperCtx *>(lv_event_get_user_data(e));
        *c->value += c->step;
        if (*c->value > c->maxV) *c->value = c->maxV;
        intStepperRefresh(c);
    }, LV_EVENT_CLICKED, ctx);
    lv_obj_add_event_cb(row, [](lv_event_t *e) {
        delete static_cast<IntStepperCtx *>(lv_event_get_user_data(e));
    }, LV_EVENT_DELETE, ctx);

    return row;
}

lv_obj_t *ui_add_long_stepper_row(lv_obj_t *parent, const char *label, long *value,
                                   long minV, long maxV, long step) {
    lv_obj_t *row = makeRow(parent, label, nullptr);

    lv_obj_t *controls = lv_obj_create(row);
    lv_obj_remove_style_all(controls);
    lv_obj_set_size(controls, 120, 28);
    lv_obj_set_flex_flow(controls, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(controls, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(controls, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *minus = lv_btn_create(controls);
    lv_obj_set_size(minus, 28, 26);
    lv_obj_t *minusLbl = lv_label_create(minus);
    lv_label_set_text(minusLbl, "-");
    lv_obj_center(minusLbl);

    lv_obj_t *valueLbl = lv_label_create(controls);
    lv_obj_set_width(valueLbl, 40);
    lv_obj_set_style_text_align(valueLbl, LV_TEXT_ALIGN_CENTER, 0);

    lv_obj_t *plus = lv_btn_create(controls);
    lv_obj_set_size(plus, 28, 26);
    lv_obj_t *plusLbl = lv_label_create(plus);
    lv_label_set_text(plusLbl, "+");
    lv_obj_center(plusLbl);

    auto *ctx = new LongStepperCtx{value, minV, maxV, step, valueLbl};
    longStepperRefresh(ctx);

    lv_obj_add_event_cb(minus, [](lv_event_t *e) {
        auto *c = static_cast<LongStepperCtx *>(lv_event_get_user_data(e));
        *c->value -= c->step;
        if (*c->value < c->minV) *c->value = c->minV;
        longStepperRefresh(c);
    }, LV_EVENT_CLICKED, ctx);
    lv_obj_add_event_cb(plus, [](lv_event_t *e) {
        auto *c = static_cast<LongStepperCtx *>(lv_event_get_user_data(e));
        *c->value += c->step;
        if (*c->value > c->maxV) *c->value = c->maxV;
        longStepperRefresh(c);
    }, LV_EVENT_CLICKED, ctx);
    lv_obj_add_event_cb(row, [](lv_event_t *e) {
        delete static_cast<LongStepperCtx *>(lv_event_get_user_data(e));
    }, LV_EVENT_DELETE, ctx);

    return row;
}

// ---- text popup editor ---------------------------------------------------

struct TextEditCtx {
    String *target;
    lv_obj_t *ta;
    lv_obj_t *modal;
    lv_obj_t *rowValueLabel; // updated after save, may be nullptr
    bool summaryOnUpdate;    // true: show "..."/"(пусто)" instead of raw text
};

static void openTextEditor(String *target, bool multiline, lv_obj_t *rowValueLabel, bool summaryOnUpdate = false) {
    lv_obj_t *modal = lv_obj_create(lv_layer_top());
    lv_obj_remove_style_all(modal);
    lv_obj_set_size(modal, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(modal, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(modal, LV_OPA_60, 0);
    lv_obj_clear_flag(modal, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *panel = lv_obj_create(modal);
    lv_obj_set_size(panel, LV_PCT(92), multiline ? 150 : 70);
    lv_obj_align(panel, LV_ALIGN_TOP_MID, 0, 8);

    lv_obj_t *ta = lv_textarea_create(panel);
    lv_obj_set_size(ta, LV_PCT(100), multiline ? 110 : 40);
    lv_textarea_set_one_line(ta, !multiline);
    lv_textarea_set_text(ta, target->c_str());

    lv_obj_t *kb = lv_keyboard_create(modal);
    lv_obj_set_size(kb, LV_PCT(100), 150);
    lv_obj_align(kb, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_keyboard_set_textarea(kb, ta);

    auto *ctx = new TextEditCtx{target, ta, modal, rowValueLabel, summaryOnUpdate};

    lv_obj_add_event_cb(kb, [](lv_event_t *e) {
        lv_event_code_t code = lv_event_get_code(e);
        auto *c = static_cast<TextEditCtx *>(lv_event_get_user_data(e));
        if (code == LV_EVENT_READY) {
            *c->target = String(lv_textarea_get_text(c->ta));
            if (c->rowValueLabel) {
                if (c->summaryOnUpdate) {
                    lv_label_set_text(c->rowValueLabel, c->target->length() ? "..." : "(пусто)");
                } else {
                    lv_label_set_text(c->rowValueLabel, c->target->c_str());
                }
            }
        }
        if (code == LV_EVENT_READY || code == LV_EVENT_CANCEL) {
            lv_obj_del(c->modal);
        }
    }, LV_EVENT_ALL, ctx);

    lv_obj_add_event_cb(modal, [](lv_event_t *e) {
        delete static_cast<TextEditCtx *>(lv_event_get_user_data(e));
    }, LV_EVENT_DELETE, ctx);
}

lv_obj_t *ui_add_text_edit_row(lv_obj_t *parent, const char *label, String *value) {
    lv_obj_t *row = makeRow(parent, label, nullptr);
    lv_obj_t *valueLbl = lv_label_create(row);
    lv_label_set_text(valueLbl, value->c_str());
    lv_obj_set_style_text_color(valueLbl, lv_palette_main(LV_PALETTE_BLUE), 0);

    struct ClickCtx { String *value; lv_obj_t *valueLbl; };
    auto *ctx = new ClickCtx{value, valueLbl};
    lv_obj_add_flag(row, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(row, [](lv_event_t *e) {
        auto *c = static_cast<ClickCtx *>(lv_event_get_user_data(e));
        openTextEditor(c->value, false, c->valueLbl);
    }, LV_EVENT_CLICKED, ctx);
    lv_obj_add_event_cb(row, [](lv_event_t *e) {
        delete static_cast<ClickCtx *>(lv_event_get_user_data(e));
    }, LV_EVENT_DELETE, ctx);
    return row;
}

lv_obj_t *ui_add_textarea_edit_row(lv_obj_t *parent, const char *label, String *value) {
    lv_obj_t *row = makeRow(parent, label, nullptr);
    lv_obj_t *valueLbl = lv_label_create(row);
    lv_label_set_text(valueLbl, value->length() ? "..." : "(пусто)");
    lv_obj_set_style_text_color(valueLbl, lv_palette_main(LV_PALETTE_BLUE), 0);

    struct ClickCtx { String *value; lv_obj_t *valueLbl; };
    auto *ctx = new ClickCtx{value, valueLbl};
    lv_obj_add_flag(row, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(row, [](lv_event_t *e) {
        auto *c = static_cast<ClickCtx *>(lv_event_get_user_data(e));
        openTextEditor(c->value, true, c->valueLbl, true);
    }, LV_EVENT_CLICKED, ctx);
    lv_obj_add_event_cb(row, [](lv_event_t *e) {
        delete static_cast<ClickCtx *>(lv_event_get_user_data(e));
    }, LV_EVENT_DELETE, ctx);
    return row;
}

lv_obj_t *ui_add_checkbox(lv_obj_t *parent, const char *label, bool *value) {
    lv_obj_t *cb = lv_checkbox_create(parent);
    lv_checkbox_set_text(cb, label);
    if (*value) lv_obj_add_state(cb, LV_STATE_CHECKED);
    lv_obj_add_event_cb(cb, [](lv_event_t *e) {
        auto *v = static_cast<bool *>(lv_event_get_user_data(e));
        lv_obj_t *target = lv_event_get_target(e);
        *v = lv_obj_has_state(target, LV_STATE_CHECKED);
    }, LV_EVENT_VALUE_CHANGED, value);
    return cb;
}
