#pragma once
#include <vector>
#include <string>
#include "Player.h"

// 商品种类:买卡,删卡,药水,遗物
enum class ShopKind { CARD, REMOVE_CARD, POTION, RELIC };

// 一件待售商品
struct ShopItem {
    ShopKind kind = ShopKind::CARD;
    std::string name;
    std::string desc;
    int cardIdx = -1;   // kind==CARD 时:对应 g_cards 下标
    int itemIdx = -1;   // kind==POTION/RELIC 时:该商品在 g_itemCatalog 里的下标
    int price = 0;
};

class Shop {
public:
    Shop();                 // 新建一间店:构造时就随机好这间的库存
    void Init();            // (重新)生成库存;进一间新店前可手动调
    void Enter(Player& p);  // 逛店主循环(不再重新生成库存)
private:
    std::vector<ShopItem> stock;
    void AddRandomItems(bool isRelic, int count); // 随机上架 count 个某类道具
    bool RemoveOne(Player& p);
};