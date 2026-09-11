#include "Game.h"
#include "Data.h"
#include "Combat.h"
#include "UI.h"
#include <string>
#include <vector>
#include <cstdlib>          // std::rand
#include <utility>          // std::move
#include "shop.h"

// ===== 战后奖励 =====
static void RewardGold(Player& p) {
    int g = 35 + std::rand() % 11;            // 35~45
    p.coin += g;
    UI::Print("掉落 " + std::to_string(g) + " 金币 (当前 " + std::to_string(p.coin) + ")。");
}

// 3 张随机卡里选 1 张带走,0 = 跳过本次卡牌奖励
static void RewardCards(Player& p) {
    int picks[3];
    for (int k = 0; k < 3; ++k) {             // 抽 3 张不重复的
        int idx = std::rand() % (int)g_cards.size();
        bool dup = false;
        for (int i = 0; i < k; ++i)
            if (picks[i] == idx) { dup = true; break; }
        if (dup) { --k; continue; }
        picks[k] = idx;
    }
    UI::Print("========== 卡牌奖励 ==========");
    UI::Print("三张卡牌选一张带走 (0 跳过):");
    for (int i = 0; i < 3; ++i) {
        const Card& c = g_cards[picks[i]];
        UI::Print(std::to_string(i + 1) + ". 【" + c.name + "】 " + c.desc);
    }
    UI::Print("请选择:");
    int sel = UI::GetInt();
    if (sel >= 1 && sel <= 3) {
        p.startDeck.push_back(picks[sel - 1]);
        UI::Print("获得卡牌【" + g_cards[picks[sel - 1]].name + "】,已加入牌组。");
    }
    else {
        UI::Print("你跳过了这次卡牌奖励。");
    }
    UI::Pause();
}

// 70% 概率掉 1 瓶随机药水(遗物不参与)
static void RewardPotion(Player& p) {
    if (std::rand() % 100 >= 70) return;      // 30% 不掉
    std::vector<int> cand;                    // 目录里所有药水
    for (size_t i = 0; i < g_itemCatalog.size(); ++i)
        if (!g_itemCatalog[i].isRelic) cand.push_back((int)i);
    int r = cand[std::rand() % cand.size()];
    p.potions.push_back(makeItem(g_itemCatalog[r].type));
    UI::Print("掉落药水【" + g_itemCatalog[r].name + "】,已收进药水袋。");
}

void Game::Run() {
    while (true) {
        MainMenu();
    }
}

void Game::MainMenu() {
    UI::Clear();
    UI::Print("=======================================");
    UI::Print("            测试版");
    UI::Print("=======================================");
    UI::Print("1. 开始战斗");
    UI::Print("2. 退出");
    UI::Print("");
    UI::Print("请选择:");

    int sel = UI::GetInt();
    if (sel == 1) NewBattle();
}

// 把一次"出场组合"实例化成一群敌人(同名出现多次 = 同种多只,各自独立血量)
static std::vector<Enemy> MakeGroup(const Encounter& ec) {
    std::vector<Enemy> group;
    for (int idx : ec.enemy) group.push_back(g_enemies[idx]);
    return group;
}

void Game::NewBattle() {
    while (true) {   // ===== 一局测试循环:选对手 → 打 → 奖励 → 商店 → 再来 =====

        // ---------- 选对手(顺带显示攒下的家底) ----------
        UI::Clear();
        UI::Print("选择你的对手:");
        UI::Print("(当前 HP " + std::to_string(player.hp) + "/" + std::to_string(player.maxHp)
            + "  金币 " + std::to_string(player.coin)
            + "  卡组 " + std::to_string(player.startDeck.size()) + " 张"
            + "  药水 " + std::to_string(player.potions.size())
            + "  遗物 " + std::to_string(player.relics.size()) + ")");
        UI::Print("");
        for (size_t i = 0; i < g_encounters.size(); ++i) {
            const Encounter& ec = g_encounters[i];
            if (ec.enemy.size() == 1) {            // 单体怪:只显示它自己
                const Enemy& en = g_enemies[ec.enemy[0]];
                UI::Print(std::to_string(i + 1) + ". " + ec.name
                    + " (HP " + std::to_string(en.maxHp) + ")");
            }
            else {                                 // 群怪:列出成员构成 + 总血量
                std::string members;
                int total = 0;
                for (size_t k = 0; k < ec.enemy.size(); ++k) {
                    if (k > 0) members += " + ";
                    members += g_enemies[ec.enemy[k]].name;
                    total += g_enemies[ec.enemy[k]].maxHp;
                }
                UI::Print(std::to_string(i + 1) + ". " + ec.name + " [" + members
                    + "] (总 HP " + std::to_string(total) + ")");
            }
        }
        UI::Print("0. 回主菜单(当前一局进度保留)");
        UI::Print("");
        UI::Print("请选择:");

        int sel = UI::GetInt();
        if (sel == 0) return;                                   // 主动结束循环,回主菜单(进度还在)
        if (sel < 1 || sel > static_cast<int>(g_encounters.size())) {
            UI::Print("无效选择。"); UI::Pause(); continue;
        }

        if (freshRun) { player.NewRun(); freshRun = false; }    // 开新一局(只第一次)
        else { player.Reset(); }                                // 续当前局:牌/药水/遗物/金币都保留

        std::string groupName = g_encounters[sel - 1].name;
        std::vector<Enemy> enemies = MakeGroup(g_encounters[sel - 1]);
        Combat combat(player, enemies);
        bool win = combat.Run();

        UI::Clear();
        if (!win) {
            UI::Print("你被 " + groupName + " 击败了... 本局结束,回主菜单重新开始吧。");
            UI::Pause();
            freshRun = true;            // 输掉 -> 下次重开一局
            return;
        }

        RewardGold(player);
        RewardPotion(player);           // 70% 掉,没掉就只显示金币
        UI::Pause();                    // 金币+药水同屏
        RewardCards(player);            // 卡牌奖励单独一屏(3选1/跳过)

        // ===== 测试入口:奖励后进商店,出店直接进下一场 =====
        UI::Print("你走进了一间商店...(买卡/清卡都在这里,选 0 离开就进下一场战斗)");
        Shop shop;                 // 每间店 new 一个新对象 = 每次库存都不同
        shop.Enter(player);
        // ---- 回到循环顶部:下一场 ----
    }
}