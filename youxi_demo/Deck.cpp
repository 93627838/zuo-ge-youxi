#include "Deck.h"
#include <cstdlib>
#include <algorithm>

// 用起始牌组初始化,并洗牌
void Deck::Reset(const std::vector<int>& startCards) {
    draw = startCards;
    hand.clear();
    discard.clear();
    ShuffleDraw();
}

// Fisher-Yates 洗牌
void Deck::ShuffleDraw() {
    for (size_t i = draw.size(); i > 1; --i) {
        size_t j = std::rand() % i;
        std::swap(draw[i - 1], draw[j]);
    }
    if (onShuffle) onShuffle();   // 洗牌后通知遗物(日晷/重织)
}

// 抽 n 张牌;抽牌堆空了就把弃牌堆洗回去接着抽
void Deck::Draw(int n) {
    for (int k = 0; k < n; ++k) {
        if (draw.empty()) {
            if (discard.empty()) return;
            draw.swap(discard);
            ShuffleDraw();
        }
        hand.push_back(draw.back());
        draw.pop_back();
    }
}

// 回合结束时把全部手牌送入弃牌堆
void Deck::DiscardHand() {
    for (int c : hand) discard.push_back(c);
    hand.clear();
}
