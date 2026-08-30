#include "screen_character.h"
#include "ui_style.h"
#include "ui_widgets.h"
#include "../app_state.h"

static void addSaveButton(lv_obj_t *parent) {
    lv_obj_t *btn = lv_btn_create(parent);
    lv_obj_set_size(btn, LV_PCT(100), 32);
    lv_obj_set_style_bg_color(btn, lv_palette_main(LV_PALETTE_GREEN), 0);
    lv_obj_add_event_cb(btn, [](lv_event_t *) { App.persist(); }, LV_EVENT_CLICKED, nullptr);
    lv_obj_t *lbl = lv_label_create(btn);
    lv_label_set_text(lbl, LV_SYMBOL_SAVE " Сохранить персонажа");
    lv_obj_center(lbl);
}

static void buildMainTab(lv_obj_t *tab) {
    Character &c = App.character;
    lv_obj_set_flex_flow(tab, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(tab, 2, 0);

    ui_add_text_edit_row(tab, "Имя", &c.name);
    ui_add_text_edit_row(tab, "Раса", &c.race);
    ui_add_text_edit_row(tab, "Класс", &c.charClass);
    ui_add_text_edit_row(tab, "Предыстория", &c.background);
    ui_add_text_edit_row(tab, "Мировоззрение", &c.alignment);
    ui_add_int_stepper_row(tab, "Уровень", &c.level, 1, 20);

    lv_obj_t *sep1 = lv_label_create(tab);
    lv_label_set_text(sep1, "-- Характеристики --");
    lv_obj_add_style(sep1, &style_dim, 0);

    ui_add_int_stepper_row(tab, "Сила (STR)", &c.str, 1, 30);
    ui_add_int_stepper_row(tab, "Ловкость (DEX)", &c.dex, 1, 30);
    ui_add_int_stepper_row(tab, "Телосложение (CON)", &c.con, 1, 30);
    ui_add_int_stepper_row(tab, "Интеллект (INT)", &c.intel, 1, 30);
    ui_add_int_stepper_row(tab, "Мудрость (WIS)", &c.wis, 1, 30);
    ui_add_int_stepper_row(tab, "Харизма (CHA)", &c.cha, 1, 30);

    lv_obj_t *sep2 = lv_label_create(tab);
    lv_label_set_text(sep2, "-- Бой --");
    lv_obj_add_style(sep2, &style_dim, 0);

    ui_add_int_stepper_row(tab, "Максимум ХП", &c.maxHp, 1, 999);
    ui_add_int_stepper_row(tab, "Текущие ХП", &c.curHp, -50, 999);
    ui_add_int_stepper_row(tab, "Временные ХП", &c.tempHp, 0, 999);
    ui_add_int_stepper_row(tab, "Класс доспеха (AC)", &c.armorClass, 0, 40);
    ui_add_int_stepper_row(tab, "Скорость (фт.)", &c.speed, 0, 200, 5);
    ui_add_int_stepper_row(tab, "Бонус инициативы", &c.initiativeBonus, -10, 20);
    ui_add_int_stepper_row(tab, "Бонус мастерства", &c.proficiencyBonus, 0, 10);
    ui_add_int_stepper_row(tab, "Кости хитов (всего)", &c.hitDiceTotal, 1, 20);
    ui_add_int_stepper_row(tab, "Кости хитов (потрачено)", &c.hitDiceUsed, 0, 20);
    ui_add_text_edit_row(tab, "Тип кости хитов", &c.hitDiceType);

    addSaveButton(tab);
}

static void buildSkillsTab(lv_obj_t *tab) {
    Character &c = App.character;
    lv_obj_set_flex_flow(tab, LV_FLEX_FLOW_COLUMN);

    static const char *abilNames[6] = {"Сил.", "Лов.", "Тел.", "Инт.", "Мдр.", "Хар."};
    lv_obj_t *sep = lv_label_create(tab);
    lv_label_set_text(sep, "Владение спасбросками:");
    for (int i = 0; i < 6; i++) {
        ui_add_checkbox(tab, abilNames[i], &c.saveProf[i]);
    }

    lv_obj_t *sep2 = lv_label_create(tab);
    lv_label_set_text(sep2, "Владение навыками (галочка = владение, повторное нажатие после владения не даёт экспертизу отдельно):");
    lv_label_set_long_mode(sep2, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(sep2, LV_PCT(96));

    extern const char *SKILL_NAMES[Character::SKILL_COUNT];
    for (int i = 0; i < Character::SKILL_COUNT; i++) {
        lv_obj_t *row = lv_obj_create(tab);
        lv_obj_remove_style_all(row);
        lv_obj_set_size(row, LV_PCT(100), 30);
        lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(row, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);
        ui_add_checkbox(row, SKILL_NAMES[i], &c.skillProf[i]);
        ui_add_checkbox(row, "Эксп.", &c.skillExpertise[i]);
    }

    addSaveButton(tab);
}

static void buildNotesTab(lv_obj_t *tab) {
    Character &c = App.character;
    lv_obj_set_flex_flow(tab, LV_FLEX_FLOW_COLUMN);

    lv_obj_t *sep = lv_label_create(tab);
    lv_label_set_text(sep, "-- Деньги --");
    lv_obj_add_style(sep, &style_dim, 0);
    ui_add_long_stepper_row(tab, "ПМ (платина)", &c.pp, 0, 999999);
    ui_add_long_stepper_row(tab, "ЗМ (золото)", &c.gp, 0, 999999);
    ui_add_long_stepper_row(tab, "ЭМ (электрум)", &c.ep, 0, 999999);
    ui_add_long_stepper_row(tab, "СМ (серебро)", &c.sp, 0, 999999);
    ui_add_long_stepper_row(tab, "ММ (медь)", &c.cp, 0, 999999);

    lv_obj_t *sep2 = lv_label_create(tab);
    lv_label_set_text(sep2, "-- Заметки --");
    lv_obj_add_style(sep2, &style_dim, 0);
    ui_add_textarea_edit_row(tab, "Черты характера", &c.personalityTraits);
    ui_add_textarea_edit_row(tab, "Умения и черты", &c.features);
    ui_add_textarea_edit_row(tab, "Заметки", &c.notes);

    addSaveButton(tab);
}

lv_obj_t *screen_character_create() {
    lv_obj_t *scr = lv_obj_create(NULL);
    lv_obj_set_style_pad_all(scr, 0, 0);

    lv_obj_t *tv = lv_tabview_create(scr, LV_DIR_TOP, 30);
    lv_obj_set_size(tv, LV_PCT(100), 320 - NAV_BAR_HEIGHT);

    lv_obj_t *tabMain = lv_tabview_add_tab(tv, "Герой");
    lv_obj_t *tabSkills = lv_tabview_add_tab(tv, "Навыки");
    lv_obj_t *tabNotes = lv_tabview_add_tab(tv, "Прочее");

    buildMainTab(tabMain);
    buildSkillsTab(tabSkills);
    buildNotesTab(tabNotes);

    ui_add_nav_bar(scr);
    return scr;
}
