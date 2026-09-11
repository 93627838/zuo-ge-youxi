#include "Data.h"

// 卡牌表
const std::vector<Card> g_cards = {//费用,类型,主效果,主效果数值,副效果,副效果数值,是否X费卡,稀有度
    {"打击",     "造成 6 点伤害",                      1, CardType::ATTACK, CardEffect::DEAL_DMG, 6},//no.0
    {"防御",     "获得 5 点格挡",                      1, CardType::SKILL,  CardEffect::GAIN_BLOCK, 5},//no.1
    {"重锤",     "造成 30 点伤害",                     2, CardType::ATTACK, CardEffect::DEAL_DMG, 30, CardEffect::NONE, 0, false, CardRarity::RARE},//no.2
    {"铁斩波",   "造成 5 点伤害,获得 3 点格挡",         1, CardType::ATTACK, CardEffect::DEAL_DMG, 5, CardEffect::GAIN_BLOCK, 3},//no.3
    {"战吼",     "抽 3 张牌",                          0, CardType::SKILL,  CardEffect::DRAW_CARD, 3, CardEffect::NONE, 0, false, CardRarity::UNCOMMON},//no.4
    {"燃烧",     "获得 2 点力量",                      1, CardType::POWER,  CardEffect::GAIN_STRENGTH, 2},//no.5
    {"耸肩无视",     "获得 8 点格挡,抽 1 张牌",                      1, CardType::SKILL,  CardEffect::GAIN_BLOCK, 8,CardEffect::DRAW_CARD, 1},//no.6
    {"肾上腺素", "获得 1 点能量,抽 2 张牌",             0, CardType::SKILL,  CardEffect::GAIN_ENERGY, 1, CardEffect::DRAW_CARD, 2},//no.7
    {"强身",     "获得 3 点格挡,获得 1 点力量",         1, CardType::SKILL,  CardEffect::GAIN_BLOCK, 3, CardEffect::GAIN_STRENGTH, 1},//no.8
    {"旋风斩",   "造成 X 次 12 点伤害",                     0, CardType::ATTACK, CardEffect::DEAL_DMG, 12, CardEffect::NONE, 0, true, CardRarity::RARE},//no.9
    {"痛击",   "造成 8 点伤害,给予 2 层易伤",      1, CardType::ATTACK, CardEffect::DEAL_DMG, 8, CardEffect::APPLY_VULNERABLE, 2, false, CardRarity::COMMON},//no.10
     {"上勾拳", "造成 13 点伤害,给予 1 层虚弱、1 层易伤", 2, CardType::ATTACK, CardEffect::DEAL_DMG, 13,CardEffect::APPLY_WEAK, 1, false, CardRarity::UNCOMMON, CardEffect::APPLY_VULNERABLE, 1},//no.11
     {"人理之础", "获得 2 点坚韧", 1, CardType::POWER, CardEffect::GAIN_TOUGHNESS, 2},//no.12
};

// 起始牌组(Player::NewRun 里的 startDeck)固定用这几个下标:
// 5 张打击 + 4 张防御 + 铁斩波 + 痛击 + 上勾拳
bool IsStarterCard(int card_idx) {
    switch (card_idx) {
    case 0:  return true;    // 打击
    case 1:  return true;    // 防御
    case 3:  return true;    // 铁斩波
    case 10: return true;    // 痛击
    case 11: return true;    // 上勾拳
    default: return false;
    }
}

const std::vector<Enemy> g_enemies = {
    // ===== 第一层:灰烬平原 =====
    {"枯皮游荡者", 50, 50, 0, 0, 0, 0, {
        {IntentType::ATTACK, 6},
        {IntentType::ATTACK, 8},
        {IntentType::BLOCK, 6},
        {IntentType::MULTI_ATTACK, 4, 2},
    }, 0},
    {"饥饿幻影", 70, 70, 0, 0, 0, 0, {
        {IntentType::ATTACK, 7},
        {IntentType::APPLY_WEAK, 2},
        {IntentType::ATTACK, 8},
        {IntentType::MULTI_ATTACK, 5, 2},
    }, 0},
    {"锈剑哨兵", 125, 125, 0, 0, 0, 0, {
        {IntentType::ATTACK, 6},
        {IntentType::BUFF_STRENGTH, 2},
        {IntentType::BLOCK, 8},
        {IntentType::MULTI_ATTACK, 5, 2},
        {IntentType::ATTACK, 8},
    }, 0},
    {"无面战团", 110, 110, 0, 0, 0, 0, {
        {IntentType::MULTI_ATTACK, 4, 3},
        {IntentType::ATTACK, 8},
        {IntentType::BLOCK, 8},
        {IntentType::MULTI_ATTACK, 6, 3},
    }, 0},
    {"烬骨巨像", 140, 140, 0, 0, 0, 0, {
        {IntentType::APPLY_VULNERABLE, 2},
        {IntentType::ATTACK, 12},
        {IntentType::BUFF_STRENGTH, 3},
        {IntentType::ATTACK, 18},
        {IntentType::BLOCK, 12},
        {IntentType::MULTI_ATTACK, 1, 15},
    }, 0},
    //群怪
    {"灰烬小鬼", 28, 28, 0, 0, 0, 0, {
          {IntentType::ATTACK, 5},
          {IntentType::ATTACK, 6},
          {IntentType::MULTI_ATTACK, 3, 2},
      }, 0},
      {"火种幽灵", 36, 36, 0, 0, 0, 0, {
          {IntentType::ATTACK, 6},
          {IntentType::BLOCK, 4},
          {IntentType::MULTI_ATTACK, 4, 2},
      }, 0},
      {"裂骨走卒", 46, 46, 0, 0, 0, 0, {
          {IntentType::ATTACK, 8},
          {IntentType::BLOCK, 6},
          {IntentType::BUFF_STRENGTH, 1},
          {IntentType::ATTACK, 7},
      }, 0},
      // ===== 第二层:倒悬海 =====
      {"溺魂聚合体", 85, 85, 0, 0, 0, 0, {
          {IntentType::ATTACK, 10}, {IntentType::BLOCK, 10},
          {IntentType::ATTACK, 12}, {IntentType::APPLY_WEAK, 2},
      }, 0},
      {"倒悬猎鲨", 95, 95, 0, 0, 0, 0, {
          {IntentType::BLOCK, 8}, {IntentType::ATTACK, 11},
          {IntentType::MULTI_ATTACK, 6, 2}, {IntentType::BUFF_STRENGTH, 2},
      }, 0},
      {"深渊之主", 115, 115, 0, 0, 0, 0, {
          {IntentType::ATTACK, 10}, {IntentType::APPLY_VULNERABLE, 2},
          {IntentType::MULTI_ATTACK, 5, 3}, {IntentType::BLOCK, 12},
      }, 0},
      // ===== 第三层:黄金迷宫 =====
        // 普通
        {"镀金骷髅", 105, 105, 0, 0, 0, 0, {
            {IntentType::BLOCK, 10}, {IntentType::ATTACK, 12},
            {IntentType::BUFF_STRENGTH, 2}, {IntentType::MULTI_ATTACK, 5, 2},
        }, 0},
        {"迷途回音", 90, 90, 0, 0, 0, 0, {
            {IntentType::ATTACK, 11}, {IntentType::APPLY_WEAK, 2},
            {IntentType::BLOCK, 12}, {IntentType::ATTACK, 15},
        }, 0},
        // 精英
        {"贪婪凝视者", 160, 160, 0, 0, 0, 0, {
            {IntentType::ATTACK, 14}, {IntentType::BUFF_STRENGTH, 3},
            {IntentType::MULTI_ATTACK, 6, 3}, {IntentType::BLOCK, 16},
        }, 0},
        {"镜像守卫", 180, 180, 0, 0, 0, 0, {
            {IntentType::BLOCK, 18}, {IntentType::ATTACK, 16},
            {IntentType::MULTI_ATTACK, 7, 2}, {IntentType::APPLY_VULNERABLE, 2},
        }, 0},

        // ===== 第四层:低语森林 =====
        // 普通
        {"发丝缠绕者", 125, 125, 0, 0, 0, 0, {
            {IntentType::APPLY_WEAK, 2}, {IntentType::ATTACK, 14},
            {IntentType::BLOCK, 14}, {IntentType::MULTI_ATTACK, 6, 2},
        }, 0},
        {"遗忘咒语书", 115, 115, 0, 0, 0, 0, {
            {IntentType::APPLY_VULNERABLE, 2}, {IntentType::ATTACK, 13},
            {IntentType::BLOCK, 16}, {IntentType::BUFF_STRENGTH, 3},
        }, 0},
        // 精英
        {"静默猎手", 205, 205, 0, 0, 0, 0, {
            {IntentType::ATTACK, 18}, {IntentType::APPLY_WEAK, 2},
            {IntentType::MULTI_ATTACK, 8, 3}, {IntentType::BLOCK, 20},
        }, 0},
        {"真名窃贼", 220, 220, 0, 0, 0, 0, {
            {IntentType::BUFF_STRENGTH, 4}, {IntentType::ATTACK, 20},
            {IntentType::APPLY_VULNERABLE, 2}, {IntentType::BLOCK, 22},
        }, 0},

        // ===== 第五层:熔炉胸腔 =====
        // 普通
        {"铁水史莱姆", 145, 145, 0, 0, 0, 0, {
            {IntentType::ATTACK, 16}, {IntentType::BLOCK, 16},
            {IntentType::MULTI_ATTACK, 7, 2}, {IntentType::BUFF_STRENGTH, 3},
        }, 0},
        {"熔炉守卫", 170, 170, 0, 0, 0, 0, {
            {IntentType::BLOCK, 22}, {IntentType::ATTACK, 19},
            {IntentType::ATTACK, 24}, {IntentType::BUFF_STRENGTH, 2},
        }, 0},
        // 精英
        {"契约烙印者", 245, 245, 0, 0, 0, 0, {
            {IntentType::APPLY_VULNERABLE, 2}, {IntentType::ATTACK, 22},
            {IntentType::BUFF_STRENGTH, 4}, {IntentType::MULTI_ATTACK, 9, 3},
            {IntentType::BLOCK, 24},
        }, 0},
        {"锻锤之灵", 270, 270, 0, 0, 0, 0, {
            {IntentType::BLOCK, 26}, {IntentType::ATTACK, 26},
            {IntentType::BUFF_STRENGTH, 5}, {IntentType::ATTACK, 34},
        }, 0},

        // ===== 第六层:镜面颅腔 =====
        // 普通
        {"怯懦之影", 165, 165, 0, 0, 0, 0, {
            {IntentType::APPLY_WEAK, 2}, {IntentType::BLOCK, 20},
            {IntentType::ATTACK, 20}, {IntentType::MULTI_ATTACK, 8, 2},
        }, 0},
        {"暴怒之我", 190, 190, 0, 0, 0, 0, {
            {IntentType::BUFF_STRENGTH, 4}, {IntentType::ATTACK, 22},
            {IntentType::MULTI_ATTACK, 9, 3}, {IntentType::ATTACK, 30},
        }, 0},
        // 精英
        {"贪婪镜像", 290, 290, 0, 0, 0, 0, {
            {IntentType::BLOCK, 30}, {IntentType::ATTACK, 26},
            {IntentType::APPLY_VULNERABLE, 2}, {IntentType::MULTI_ATTACK, 10, 3},
            {IntentType::BUFF_STRENGTH, 5},
        }, 0},
        {"命运残线", 320, 320, 0, 0, 0, 0, {
            {IntentType::ATTACK, 30}, {IntentType::APPLY_WEAK, 2},
            {IntentType::BLOCK, 32}, {IntentType::BUFF_STRENGTH, 6},
            {IntentType::MULTI_ATTACK, 12, 3},
        }, 0},
};

const std::vector<Encounter> g_encounters = {
    // ===== 第一层:灰烬平原(单体怪) =====
    {"枯皮游荡者", {0}},
    {"饥饿幻影", {1}},
    {"锈剑哨兵", {2}},
    {"无面战团", {3}},
    {"烬骨巨像", {4}},
    // ===== 第一层:低级杂兵组队的群怪 =====
    {"烬灰尸群", {5, 5, 5}},          // 3 只灰烬小鬼
    {"幽魂双影", {6, 5}},             // 火种幽灵 + 灰烬小鬼
    {"破甲巡逻队", {7, 7}},           // 2 只裂骨走卒
    {"断骨小队", {7, 5, 6}},          // 裂骨走卒 + 灰烬小鬼 + 火种幽灵
    // ===== 第二层:倒悬海(单体怪) =====
    {"溺魂聚合体", {8}},
    {"倒悬猎鲨", {9}},
    {"深渊之主", {10}},
    // ===== 第三层:黄金迷宫 =====
    {"镀金骷髅", {11}},
    {"迷途回音", {12}},
    {"贪婪凝视者", {13}},
    {"镜像守卫", {14}},
    // ===== 第四层:低语森林 =====
    {"发丝缠绕者", {15}},
    {"遗忘咒语书", {16}},
    {"静默猎手", {17}},
    {"真名窃贼", {18}},
    // ===== 第五层:熔炉胸腔 =====
    {"铁水史莱姆", {19}},
    {"熔炉守卫", {20}},
    {"契约烙印者", {21}},
    {"锻锤之灵", {22}},
    // ===== 第六层:镜面颅腔 =====
    {"怯懦之影", {23}},
    {"暴怒之我", {24}},
    {"贪婪镜像", {25}},
    {"命运残线", {26}},
};
