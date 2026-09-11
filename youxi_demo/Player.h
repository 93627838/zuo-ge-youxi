#pragma once
#include <vector>
#include <memory>
#include "Deck.h"
#include "Item.h"

// 玩家(第一版只有玩家单角色)
struct Player {
    int maxHp = 70;
    int hp = 70;
    int block = 0;
    int coin = 0;
	double chr = 0.114514;          //暴击率,0~1,每次攻击有 chr 概率触发暴击(伤害 ×2)
    int strength = 0;               // 力量,叠加到攻击伤害上
    int toughness = 0;              // 坚韧:打出加格挡牌时,格挡值 +toughness
    int nextAtkMult = 0;            // 超巨化药水:接下来 N 张攻击牌伤害 ×3
    int weak = 0;                   // 虚弱:你的攻击 -25%
    int vulnerable = 0;             // 易伤:你受到的伤害 +50%
    int apothGauge = 0;             // 登神进度 0~100
    bool apotheosis = false;        // 是否处于登神形态
    std::vector<int> startDeck;     // 起始牌组(卡牌下标)
    Deck deck;                      // 对局中的牌堆
    std::vector<std::unique_ptr<Item>> relics;    // 已拥有的遗物(被动,常驻)
    std::vector<std::unique_ptr<Item>> potions;   // 药水背包(主动或自动,用掉就少)

    void NewRun();   // 开一局:初始化金币/遗物/药水/起始牌组(一局只调一次)
    void Reset();    // 每场战斗开始前:只重置对局内数值,不清空攒下的遗物/药水/牌组/金币
};
