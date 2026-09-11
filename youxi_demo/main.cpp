// main.cpp
#include "FloorManager.h"
#include "Player.h"
#include "Combat.h"
#include "EncounterBuilder.h"
#include "Data.h"
#include "DeckView.h"
#include "shop.h"
#include "Item.h"

#include <iostream>
#include <limits>
#include <string>
#include <vector>
#include <cstdlib>
#include <cctype>
#include <ctime>

using namespace std;

// 从控制台读取一个限定范围内的整数
int readNumber(int min_value, int max_value) {
    int number = 0;
    while (true) {
        cout << "> ";
        if (cin >> number && number >= min_value && number <= max_value) {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return number;
        }
        if (cin.eof()) return min_value;   // 输入流结束(如管道用尽),别死循环
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "请输入 " << min_value << " 到 " << max_value << " 之间的数字。\n";
    }
}

// 故事模式第7层通关后的空王座结局
void showStoryEnding() {
    cout << "\n========================================\n";
    cout << "           第七层 · 空王座           \n";
    cout << "========================================\n";
    cout << "\n你击败了王座前的最后一道残影。\n";
    cout << "空王座上坐着的那个人，是那个如果从一开始就选择另一条路的你。\n\n";
    cout << "它问：你想成为怎样的神？\n\n";
    cout << "1. 封神——吞噬王座上的自己，升上天空，孤独地注视人间。\n";
    cout << "2. 碎塔——引爆体内所有神骸，解放塔中所有灵魂。\n";
    cout << "3. 永囚——拒绝选择，坐上王座，成为塔的新核心。\n";

    const int choice = readNumber(1, 3);
    if (choice == 1) {
        cout << "\n你吞下了空王座上的自己。塔在你身后崩塌，你升上天空。\n";
        cout << "世界恢复和平，但再无人能登塔。\n";
        cout << "——结局：封神——\n";
    }
    else if (choice == 2) {
        cout << "\n你将所有神骸集中到胸口，从塔心引爆。\n";
        cout << "你死了，但所有被困的灵魂获得了解放。\n";
        cout << "凡间回归正常，神性与魔法一同消亡。\n";
        cout << "——结局：碎塔——\n";
    }
    else {
        cout << "\n你在空王座上坐下，闭上眼睛。\n";
        cout << "从此你就是塔，塔就是你。\n";
        cout << "后来的登塔者都会在顶层遇见你的残影。\n";
        cout << "——结局：永囚——\n";
    }
    cout << "\n故事模式完成。\n";
}

// ========== 命令规整 ==========

// 去掉首尾空白并转小写，让 'MAP' / 'Map' / 'm' 等价
static string normalizeCommand(const string& s) {
    const size_t a = s.find_first_not_of(" \t\r\n");
    if (a == string::npos) return "";
    const size_t b = s.find_last_not_of(" \t\r\n");
    string r = s.substr(a, b - a + 1);
    for (char& c : r) c = static_cast<char>(tolower(static_cast<unsigned char>(c)));
    return r;
}

// ===== 战后奖励 =====

// 结算地图模块自带的房间奖励(金币/物品/风味文本)
// 以前这段被 clearCurrentRoom() 的返回值丢掉了,现在统一在这里结算
static void grantReward(Player& p, const RoomReward& reward) {
    cout << "\n[战利品]\n";
    if (reward.gold > 0) {
        p.coin += reward.gold;
        cout << "  +" << reward.gold << " 金币 (当前 " << p.coin << ")\n";
    }
    if (!reward.item_name.empty())
        cout << "  获得: " << reward.item_name << " (" << reward.item_effect << ")\n";
    if (!reward.flavor_text.empty())
        cout << "  " << reward.flavor_text << "\n";
}

// 药水:70% 概率掉 1 瓶随机药水(遗物不参与)
static void RewardPotion(Player& p) {
    if (rand() % 100 >= 70) return;                 // 30% 不掉
    vector<int> cand;
    for (size_t i = 0; i < g_itemCatalog.size(); ++i)
        if (!g_itemCatalog[i].isRelic) cand.push_back((int)i);
    if (cand.empty()) return;
    const int r = cand[rand() % cand.size()];
    p.potions.push_back(makeItem(g_itemCatalog[r].type));
    cout << "掉落药水【" << g_itemCatalog[r].name << "】,已收进药水袋。\n";
}

// 遗物:从遗物目录里随机抽 1 个(跳过已拥有的)
static void RewardRelic(Player& p) {
    vector<int> cand;
    for (size_t i = 0; i < g_itemCatalog.size(); ++i) {
        if (!g_itemCatalog[i].isRelic) continue;
        bool owned = false;
        for (const auto& rel : p.relics)
            if (rel->name == g_itemCatalog[i].name) { owned = true; break; }
        if (!owned) cand.push_back((int)i);
    }
    if (cand.empty()) {                     // 遗物已全拿齐
        cout << "宝箱里没有新的遗物了。\n";
        return;
    }
    const int r = cand[rand() % cand.size()];
    p.relics.push_back(makeItem(g_itemCatalog[r].type));
    cout << "获得遗物【" << g_itemCatalog[r].name << "】!\n";
}

// 露滴圣杯瓶:每层 BOSS 必掉(可叠加,每多一个每回合 +1 能量)
static void RewardHolyVial(Player& p) {
    p.relics.push_back(makeItem(ItemType::HOLY_VIAL));
    cout << "获得遗物【露滴圣杯瓶】!(每回合开始 +1 能量,效果可叠加)\n";
}

// 卡牌:必出,从全部卡里随机抽 3 张不重复,选 1 张带走(0 跳过)
// 起始牌组自带的牌(打击/防御/…)不上奖励池,免得奖励里全是开局就有的牌
static void RewardCards(Player& p) {
    vector<int> pool;
    for (size_t i = 0; i < g_cards.size(); ++i)
        if (!IsStarterCard((int)i)) pool.push_back((int)i);

    int picks[3];
    for (int k = 0; k < 3; ++k) {                   // 抽 3 张不重复的
        const int idx = pool[rand() % pool.size()];
        bool dup = false;
        for (int i = 0; i < k; ++i)
            if (picks[i] == idx) { dup = true; break; }
        if (dup) { --k; continue; }
        picks[k] = idx;
    }

    cout << "\n========== 卡牌奖励 ==========\n";
    cout << "三张卡牌选一张带走 (0 跳过):\n";
    for (int i = 0; i < 3; ++i) {
        const Card& c = g_cards[picks[i]];
        cout << "  " << (i + 1) << ". 【" << c.name << "】 " << c.desc << "\n";
    }
    cout << "请选择: ";

    string line;
    cin >> line;
    int sel = 0;
    try { sel = stoi(line); } catch (...) { sel = 0; }

    if (sel >= 1 && sel <= 3) {
        p.startDeck.push_back(picks[sel - 1]);
        cout << "获得卡牌【" << g_cards[picks[sel - 1]].name << "】,已加入牌组。\n";
    }
    else {
        cout << "你跳过了这次卡牌奖励。\n";
    }
}

// 战斗房间：开打 → 胜利结算奖励 → 标记完成
static bool RunCombatRoom(Player& player, FloorManager& manager,
    const Room& room, int floor) {
    // 楼层 + 房间类型 → 这一场的敌人(数值强化统一由 EncounterBuilder 负责)
    const EncounterBuilder::Setup setup = EncounterBuilder::build(floor, room.getType());

    cout << "\n遭遇:【" << setup.group_name << "】\n";
    cout << "按 Enter 进入战斗...";
    string dummy;
    getline(cin, dummy);

    player.Reset();

    Combat combat(player, setup.enemies);
    if (!combat.Run()) {
        cout << "\n你在房间【" << room.getName() << "】中战败了。\n";
        return false;
    }

    cout << "\n战斗胜利！\n";

    // 首领战后恢复:把血量补到最大生命的九成(本来就更高就不动)
    if (room.isBoss()) {
        const int target = player.maxHp * 90 / 100;
        if (player.hp < target) {
            const int heal = target - player.hp;
            player.hp = target;
            cout << "击败首领,你重整旗鼓,回复 " << heal << " 点生命 (HP "
                << player.hp << "/" << player.maxHp << ")。\n";
        }
    }

    RewardPotion(player);    // 药水(70% 概率)
    if (room.isBoss()) RewardHolyVial(player);   // BOSS 必掉露滴圣杯瓶
    RewardCards(player);     // 卡牌(必出,三选一/跳过)

    // 金币 / 经验 / 风味文本来自地图模块的 RoomReward
    auto reward = manager.clearCurrentRoom();
    if (reward.has_value()) grantReward(player, *reward);
    return true;
}

// ========== 主流程 ==========

int main() {
    Color::init();
    srand(static_cast<unsigned>(time(nullptr)));

    cout << "========================================\n";
    cout << "       逆神者之塔 - 肉鸽爬塔           \n";
    cout << "========================================\n";

    cout << "\n请选择游戏模式:\n";
    cout << "1. 故事模式（固定剧情，第1~7层，通关后选择结局）\n";
    cout << "2. 无尽模式（前7层固定，第8层起随机循环主题）\n";
    const int mode_choice = readNumber(1, 2);

    unsigned int seed = 0;
    cout << "\n请输入地图种子 (0 表示随机): ";
    string seedLine;
    cin >> seedLine;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    try {
        const long long v = stoll(seedLine);
        seed = (v < 0) ? 0u : static_cast<unsigned int>(v);
    }
    catch (...) {
        seed = 0;
        cout << "(输入无效，已改用随机种子)\n";
    }

    FloorManager manager;
    manager.setGameMode(mode_choice == 1 ? GameMode::Story : GameMode::Endless);
    try {
        manager.loadFloor(1, seed);
    }
    catch (const exception& e) {
        cerr << "\n加载楼层失败: " << e.what() << "\n";
        return 1;
    }

    Player player;
    player.NewRun();

    cout << "\n地图种子: " << seed << "\n";
    cout << "当前模式: "
        << (manager.getGameMode() == GameMode::Story ? "故事模式" : "无尽模式")
        << "\n";
    cout << "地图加载完成！\n\n请按Enter键继续...\n";
    cin.get();
    cout << "\033[2J\033[2;1H";

    // 房间信息块。只在"进入房间"时印一次,紧接着才是这个房间的内容
    // (商店/宝箱/遭遇),所以 moveTo / tryGoUp 之后要立刻调用。
    auto printRoomHeader = [&](const Room& r) {
        cout << "\n========== [第 " << manager.getCurrentFloor() << " 层] "
            << r.getColoredName() << " ==========\n";
        cout << "\n" << r.getDesc() << "\n";
        cout << "[类型: " << r.getTypeNameWithColor() << "]\n";
        cout << "[HP " << player.hp << "/" << player.maxHp
            << "  金币 " << player.coin << "]\n";
    };

    bool firstRoom = true;   // 起始房间也要印一次(它不是通过 moveTo 进的)
    bool quit = false;
    while (!quit) {
        const Room& cur = manager.getCurrentRoom();
        const bool headerJustPrinted = firstRoom;
        if (firstRoom) {
            printRoomHeader(cur);
            firstRoom = false;
        }

        if (cur.isBoss() && manager.isBossDefeated()) {
            if (manager.isStoryEnd()) {
                showStoryEnding();
                quit = true;
                break;
            }
            cout << "\n[提示] BOSS已击败！输入 'up' 前往下一层。\n";
        }

        const auto options = manager.getCurrentOptions();
        if (options.empty() && !cur.isBoss()) {
            cout << "\n此路已尽，但未遇到BOSS？请检查地图内容。\n";
            break;
        }

        if (!options.empty()) {
            // 房间信息块只在进入时印过一次,这里补一行当前状态,
            // 否则打完一场仗看不到自己还剩多少血。
            // 起始房间那轮刚印过信息块,就别再重复印一遍了。
            if (!headerJustPrinted)
                cout << "\n[HP " << player.hp << "/" << player.maxHp
                << "  金币 " << player.coin << "]\n";
            cout << "\n可选路径:\n";
            for (size_t i = 0; i < options.size(); ++i) {
                const Room& next = manager.getRoomById(options[i]);
                cout << "  " << (i + 1) << ". " << next.getColoredName()
                    << " (" << next.getTypeNameWithColor() << ")\n";
            }
            cout << "\n请输入数字选择，或输入 'map'、'up'、'd'(牌组)、'quit': ";

            string input;
            if (!(cin >> input)) { quit = true; break; }   // 输入结束(管道用尽/EOF)则退出
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            const string cmd = normalizeCommand(input);
            if (cmd == "quit" || cmd == "q") {
                quit = true;
                break;
            }
            if (cmd == "map" || cmd == "m") {
                cout << "\033[2J\033[2;1H";
                manager.drawMap();
                continue;
            }
            if (cmd == "up" || cmd == "u") {
                if (manager.tryGoUp()) {
                    cout << "\033[2J\033[H";
                    cout << "你踏入了下一层...\n";
                    printRoomHeader(manager.getCurrentRoom());
                    continue;
                }
                cout << "\033[2J\033[H";
                cout << "必须击败BOSS才能上楼！\n";
                continue;
            }
            if (cmd == "d" || cmd == "牌组") {
                DeckView::ShowDeckOnly(player);
                continue;
            }

            try {
                const int choice = stoi(cmd);
                if (choice >= 1 && choice <= static_cast<int>(options.size())) {
                    manager.moveTo(options[choice - 1]);

                    // 先印新房间的信息,再印这个房间的内容(商店/宝箱/遭遇)
                    printRoomHeader(manager.getCurrentRoom());

                    // ★ 关键：进入房间后，按房间类型分流
                    const Room& entered = manager.getCurrentRoom();
                    bool success = true;

                    switch (entered.getType()) {
                    case RoomType::Combat:
                    case RoomType::Elite:
                    case RoomType::Boss:
                        success = RunCombatRoom(player, manager, entered,
                            manager.getCurrentFloor());
                        break;

                    case RoomType::Shop: {
                        Shop shop;
                        shop.Enter(player);
                        manager.clearCurrentRoom();
                        break;
                    }

                    case RoomType::Treasure: {
                        cout << "\n你打开了宝箱。\n";
                        RewardRelic(player);     // 遗物
                        auto reward = manager.clearCurrentRoom();
                        if (reward.has_value()) grantReward(player, *reward);
                        break;
                    }

                    case RoomType::Event: {
                        cout << "\n你触发了事件。\n";
                        // 包一层循环:输入 d 看完牌组后回到这里,而不是把这次事件消耗掉
                        while (true) {
                            cout << "1. 接受交易（支付 30 金币，获得一件随机遗物）\n";
                            cout << "2. 就地休息（恢复最大生命的 30%）\n";
                            cout << "3. 转身离开\n";
                            cout << "(输入 d 查看牌组)\n";
                            cout << "> ";
                            string in;
                            cin >> in;
                            cin.ignore(numeric_limits<streamsize>::max(), '\n');
                            const string ec = normalizeCommand(in);
                            if (ec == "d" || ec == "牌组") {
                                DeckView::ShowDeckOnly(player);
                                continue;
                            }
                            if (in == "1") {
                                if (player.coin >= 30) {
                                    player.coin -= 30;
                                    RewardRelic(player);
                                }
                                else {
                                    cout << "你的金币不够，只好作罢。\n";
                                }
                            }
                            else if (in == "2") {
                                const int heal = player.maxHp * 30 / 100;
                                player.hp += heal;
                                if (player.hp > player.maxHp) player.hp = player.maxHp;
                                cout << "你靠着墙坐下，喘了口气，恢复了 " << heal
                                    << " 点生命 (当前 HP " << player.hp << "/"
                                    << player.maxHp << ")。\n";
                            }
                            else {
                                cout << "你离开了。\n";
                            }
                            break;
                        }
                        auto reward = manager.clearCurrentRoom();
                        if (reward.has_value()) grantReward(player, *reward);
                        break;
                    }

                    case RoomType::Start:
                        break;
                    }

                    if (!success) {
                        cout << "\n本局结束。\n";
                        quit = true;
                        break;
                    }
                    continue;
                }
                cout << "\033[2J\033[H";
                cout << "无效选择。\n";
            }
            catch (...) {
                cout << "\033[2J\033[H";
                cout << "无效输入，请输入数字、'map'、'up' 或 'quit'。\n";
            }
            continue;
        }

        if (cur.isBoss() && manager.isBossDefeated()) {
            cout << "输入 'up' 上楼，或 'quit' 退出: ";
            string input;
            if (!(cin >> input)) { quit = true; break; }   // 输入结束则退出
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            const string cmd = normalizeCommand(input);
            if (cmd == "quit" || cmd == "q") {
                quit = true;
            }
            else if (cmd == "up" || cmd == "u") {
                if (manager.tryGoUp()) {
                    cout << "\033[2J\033[H";
                    cout << "你踏入了下一层...\n";
                    continue;
                }
                cout << "\033[2J\033[H";
                cout << "无法继续上楼。\n";
            }
            else {
                cout << "\033[2J\033[H";
                cout << "无效输入。\n";
            }
        }

        cout << "\033[2J\033[2;1H";
    }
    return 0;
}