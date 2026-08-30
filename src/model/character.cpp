#include "character.h"

const char *SKILL_NAMES[Character::SKILL_COUNT] = {
    "Акробатика", "Анализ", "Атлетика", "Восприятие", "Выживание",
    "Выступление", "Запугивание", "История", "Ловкость рук",
    "Магия", "Медицина", "Обман", "Природа", "Проницательность",
    "Религия", "Скрытность", "Убеждение", "Уход за животными",
};

// index into {str,dex,con,int,wis,cha} = 0..5
const int SKILL_ABILITY[Character::SKILL_COUNT] = {
    1, /*Акробатика - Dex*/
    3, /*Анализ (Investigation) - Int*/
    0, /*Атлетика - Str*/
    4, /*Восприятие (Perception) - Wis*/
    4, /*Выживание - Wis*/
    5, /*Выступление (Performance) - Cha*/
    5, /*Запугивание - Cha*/
    3, /*История - Int*/
    1, /*Ловкость рук (Sleight of Hand) - Dex*/
    3, /*Магия (Arcana) - Int*/
    4, /*Медицина - Wis*/
    5, /*Обман (Deception) - Cha*/
    3, /*Природа - Int*/
    4, /*Проницательность (Insight) - Wis*/
    3, /*Религия - Int*/
    1, /*Скрытность (Stealth) - Dex*/
    5, /*Убеждение (Persuasion) - Cha*/
    4, /*Уход за животными (Animal Handling) - Wis*/
};
