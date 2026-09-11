#pragma once
#include <string>

// 卡牌类型
enum class CardType { ATTACK, SKILL ,POWER};

// 卡牌效果种类
enum class CardEffect {
    NONE,
    DEAL_DMG,       // 造成伤害
    GAIN_BLOCK,     // 获得格挡
    GAIN_STRENGTH,  // 获得力量
    GAIN_TOUGHNESS,//获得坚韧
    GAIN_ENERGY,    // 获得能量
    DRAW_CARD,       // 抽牌
    APPLY_WEAK,        // 使敌人虚弱(其攻击 -25%)
    APPLY_VULNERABLE   // 使敌人易伤(你对其伤害 +50%)
};

// 卡牌稀有度
enum class CardRarity { COMMON, UNCOMMON, RARE };

// 一张卡牌:主效果(effect+x) + 可选副效果(effect2+x2)
struct Card {
    std::string name;
    std::string desc;
    int cost = 0;
    CardType type = CardType::ATTACK;
    CardEffect effect = CardEffect::NONE;
    int x = 0;
    CardEffect effect2 = CardEffect::NONE;
    int x2 = 0;
    bool isX = false;      // X 费卡标记:花费全部能量,伤害按能量计算
    CardRarity rarity = CardRarity::COMMON;   // 默认普通
    CardEffect effect3 = CardEffect::NONE;   // 第 3 效果(目前给"伤害+双负面"卡用)
    int x3 = 0;
};
