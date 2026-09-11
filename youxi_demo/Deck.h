#pragma once
#include <vector>
#include <functional>

// 牌堆:抽牌堆 / 手牌 / 弃牌堆,存的是卡牌下标
class Deck {
public:
    std::vector<int> draw;      // 抽牌堆
    std::vector<int> hand;      // 手牌
    std::vector<int> discard;   // 弃牌堆
    std::function<void()> onShuffle;   // 每次洗牌完成后回调(遗物:日晷/重织),战斗中由 Combat 挂载

    void Reset(const std::vector<int>& startCards);
    void ShuffleDraw();
    void Draw(int n);
    void DiscardHand();
};
