#include "Player.h"

// 开新的一局:整局共享的遗物/药水/牌组/金币只在这里初始化一次
void Player::NewRun() {
    maxHp = 70;
    hp = 70;
    block = 0;
    strength = 0;
    nextAtkMult = 0;
    chr = 0.114514;
    coin = 0;
    // 起始牌组:下标对应 Data.cpp 里的 g_cards 表
    startDeck = {0, 0, 0, 0, 0, 1, 1, 1, 1, 3, 10, 11};

    // ===== 测试用:先自带一遗物两药水方便看效果;之后改为从奖励/商店获得 =====
    relics.clear();
    relics.push_back(std::make_unique<BloodVial>());   // 开局自带小血瓶:每场战斗开始回 2 血
    potions.clear();
    potions.push_back(std::make_unique<ForesightPotion>());
    potions.push_back(std::make_unique<DrawPotion>());

    Reset();   // 铺好第一战的牌堆
}

// 每场战斗开始前调用:只重置"对局内"的东西;生命/遗物/药水/牌组/金币跨场保留
void Player::Reset() {
    block = 0;
    strength = 0;
    toughness = 0;
    nextAtkMult = 0;
    weak = 0;
    vulnerable = 0;
    apothGauge = 0;
    apotheosis = false;
    deck.Reset(startDeck);   // 按当前 startDeck(含商店买进的牌)重新洗牌
}

