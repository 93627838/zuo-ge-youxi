#include "shop.h"
#include "Data.h"   // g_cards
#include "DeckView.h"
#include "UI.h"
#include <utility>           // std::pair
#include <cstdlib>           // std::rand / std::atoi
#include <cctype>            // std::tolower

// 去首尾空白并转小写,让 'D' / 'd' / ' D ' 等价(CJK 原样保留)
static std::string NormalizeInput(const std::string& s) {
    const size_t a = s.find_first_not_of(" \t\r\n");
    if (a == std::string::npos) return "";
    const size_t b = s.find_last_not_of(" \t\r\n");
    std::string r = s.substr(a, b - a + 1);
    for (char& c : r) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return r;
}

static int CardPrice(const Card& c) {
    switch (c.rarity) {
    case CardRarity::UNCOMMON: return 75;
    case CardRarity::RARE:     return 125;
    default:                   return 50;   // COMMON
    }
}

static std::string SectionTitle(ShopKind k) {
    switch (k) {
    case ShopKind::CARD:        return "── 卡 牌 ────────";
    case ShopKind::POTION:      return "── 药 水 ────────";
    case ShopKind::RELIC:       return "── 遗 物 ────────";
    case ShopKind::REMOVE_CARD: return "── 删 牌 ────────";
    }
    return "";
}

void Shop::Init() {
    stock.clear();
    for (int k = 0; k < 4; ++k) {               // 4 张不重复的随机卡
        int idx = std::rand() % (int)g_cards.size();
        bool dup = false;
        for (const auto& it : stock)
            if (it.cardIdx == idx) { dup = true; break; }
        if (dup) { --k; continue; }
        ShopItem it;
        it.kind = ShopKind::CARD;
        it.cardIdx = idx;
        it.name = g_cards[idx].name;
        it.desc = g_cards[idx].desc;
        it.price = CardPrice(g_cards[idx]);
        stock.push_back(it);
    }
    AddRandomItems(false, 3);   // 药水区:随机上架 3 瓶药水
    AddRandomItems(true, 3);    // 遗物区:随机上架 3 个遗物
    ShopItem rm;                                // 删牌服务固定放最后
    rm.kind = ShopKind::REMOVE_CARD;
    rm.name = "清卡";
    rm.desc = "从牌组中删去 1 张牌";
    rm.price = 75;
    stock.push_back(rm);
}
Shop::Shop() { Init(); }

// 从目录里挑 count 个"不重复"的某类(药水/遗物),加入库存
void Shop::AddRandomItems(bool isRelic, int count) {
    std::vector<int> cand;                     // 候选:目录中同类型且还没上架的
    for (size_t i = 0; i < g_itemCatalog.size(); ++i) {
        const ItemDef& d = g_itemCatalog[i];
        if (d.isRelic != isRelic) continue;    // 只筛药水 / 或只筛遗物
        bool dup = false;
        for (const auto& it : stock)
            if (it.itemIdx == (int)i) { dup = true; break; }
        if (!dup) cand.push_back((int)i);
    }
    while (count > 0 && !cand.empty()) {       // 随机抽 count 个
        int r = std::rand() % (int)cand.size();
        int idx = cand[r];
        cand.erase(cand.begin() + r);
        const ItemDef& d = g_itemCatalog[idx];
        ShopItem it;
        it.kind = isRelic ? ShopKind::RELIC : ShopKind::POTION;
        it.itemIdx = idx;
        it.name = d.name;
        it.desc = d.desc;
        it.price = d.price;
        stock.push_back(it);
        --count;
    }
}

void Shop::Enter(Player& p) {
    while (true) {
        UI::Clear();
        UI::Print("========== 商 店 ==========");
        UI::Print("金币: " + std::to_string(p.coin));
        UI::Print("");
        ShopKind lastKind = ShopKind::CARD;     // 只在分区变化时打印小标题
        bool firstRow = true;
        for (size_t i = 0; i < stock.size(); ++i) {
            const ShopItem& it = stock[i];
            if (firstRow || it.kind != lastKind) {
                UI::Print(SectionTitle(it.kind));
                firstRow = false;
                lastKind = it.kind;
            }
            UI::Print(std::to_string(i + 1) + ". 【" + it.name + "】 "
                + it.desc + " — " + std::to_string(it.price) + " 金币");
        }
        UI::Print("0. 离开商店");
        UI::Print("输入编号购买 / 0 离开 / D 查看牌组:");

        // 用 GetLine 而不是 GetInt:这样 'D' 之类的字母才不会被 atoi 成 0 直接踢出商店
        const std::string cmd = NormalizeInput(UI::GetLine());
        if (cmd == "d" || cmd == "牌组") {
            DeckView::ShowDeckOnly(p);
            continue;
        }
        if (cmd == "0") break;
        const int sel = std::atoi(cmd.c_str());
        if (sel < 1 || sel >(int)stock.size()) { UI::Print("无效选择。"); UI::Pause(); continue; }

        ShopItem& it = stock[sel - 1];
        if (it.kind == ShopKind::CARD) {
            if (p.coin < it.price) { UI::Print("金币不足!"); UI::Pause(); continue; }
            p.coin -= it.price;
            p.startDeck.push_back(it.cardIdx);          // 买进永久牌组
            UI::Print("买下【" + it.name + "】,已加入牌组。");
            stock.erase(stock.begin() + sel - 1);       // 下架
        }
        else if (it.kind == ShopKind::REMOVE_CARD) {
            if (p.coin < it.price) { UI::Print("金币不足!"); UI::Pause(); continue; }
            if (!RemoveOne(p)) continue;                // 取消删卡就不扣钱
            p.coin -= it.price;
        }
        else if (it.kind == ShopKind::POTION || it.kind == ShopKind::RELIC) {
            if (p.coin < it.price) { UI::Print("金币不足!"); UI::Pause(); continue; }
            p.coin -= it.price;
            auto item = makeItem(g_itemCatalog[it.itemIdx].type);
            if (it.kind == ShopKind::RELIC)
                p.relics.push_back(std::move(item));   // 进遗物栏
            else
                p.potions.push_back(std::move(item));  // 进药水背包
            UI::Print("买下【" + it.name + "】,已收进你的背包。");
            stock.erase(stock.begin() + sel - 1);      // 下架
        }
        UI::Pause();
    }
}

bool Shop::RemoveOne(Player& p) {
    // 统计牌组里每种牌各几张,保持出现顺序
    std::vector<std::pair<int, int>> uniq;
    for (int c : p.startDeck) {
        bool found = false;
        for (auto& u : uniq)
            if (u.first == c) { ++u.second; found = true; break; }
        if (!found) uniq.push_back({ c, 1 });
    }
    if (uniq.empty()) { UI::Print("牌组为空,无法删卡。"); return false; }

    UI::Clear();
    UI::Print("牌组共 " + std::to_string(p.startDeck.size())
        + " 张,选一种删掉 1 张(0 取消):");
    for (size_t i = 0; i < uniq.size(); ++i) {
        const Card& c = g_cards[uniq[i].first];
        UI::Print(std::to_string(i + 1) + ". " + c.name + "  ×" + std::to_string(uniq[i].second));
    }
    int sel = UI::GetInt();
    if (sel < 1 || sel >(int)uniq.size()) return false;

    int target = uniq[sel - 1].first;
    for (size_t i = 0; i < p.startDeck.size(); ++i) {
        if (p.startDeck[i] == target) {
            p.startDeck.erase(p.startDeck.begin() + i);
            UI::Print("已删掉 1 张【" + g_cards[target].name + "】。");
            return true;
        }
    }
    return false;
}