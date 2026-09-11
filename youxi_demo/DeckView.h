#pragma once
#include <string>
#include <vector>
#include "Card.h"
#include "Player.h"

// ============================================================
//  DeckView —— 牌组查看的共享渲染层
//
//  战斗 / 商店 / 事件三处都要看牌组,所以刻意不依赖 Combat:
//  战斗外拿不到敌人状态,只能看牌组构成;战斗内想看"这一张打出去
//  到底多少伤害"时,由 Combat::ShowDeckView() 把目标易伤传进来。
// ============================================================
namespace DeckView {

    // 「攻击 / 技能 / 能力」
    std::string CardTypeName(CardType t);

    // 按卡名分组计数打印：  打击×5  防御×4  铁斩波×1
    void PrintGroupedCounts(const std::vector<int>& cards, const std::string& title);

    // 一张卡的伤害/格挡预览行,复刻 Combat::ApplyEffect 的整数公式:
    //   【打击】(1费, 攻击) 造成 6×1.5×1.5 点伤害 (13)
    // enemyVulnerable = 目标敌人当前易伤层数(战斗外传 0)
    std::string FormatCardPreview(const Card& c, const Player& p, int enemyVulnerable);

    // 战斗外入口:打印 startDeck 的分组计数,然后等回车
    void ShowDeckOnly(const Player& p);

}
