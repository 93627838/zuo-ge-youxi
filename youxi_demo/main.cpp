// main.cpp
#include "FloorManager.h"
#include "Player.h"
#include "Combat.h"
#include "Data.h"
#include "shop.h"
#include "Item.h"

#include <iostream>
#include <limits>
#include <string>
#include <vector>
#include <cstdlib>
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

// ========== 地图房间 → 战斗系统 的适配层 ==========

// 按楼层 + 房间类型，挑一个 g_encounters 的下标
static int EncounterForRoom(int floor, RoomType type) {
    // 无尽模式第 8 层起，循环复用第 1~6 层的主题
    // (与 FloorSource.cpp 的 theme_floor 公式保持一致)
    if (floor > 7) floor = ((floor - 8) % 6) + 1;

    if (type == RoomType::Combat) {
        switch (floor) {
        case 1: return 0;
        case 2: return 8;
        case 3: return 11;
        case 4: return 15;
        case 5: return 19;
        case 6: return 23;
        case 7: return 0;    // 第7层没有普通战斗房，兜底
        }
    }
    if (type == RoomType::Elite) {
        switch (floor) {
        case 1: return 4;
        case 2: return 10;
        case 3: return 13;
        case 4: return 17;
        case 5: return 21;
        case 6: return 25;
        case 7: return 4;    // 第7层没有精英房，兜底
        }
    }
    if (type == RoomType::Boss) {
        switch (floor) {
        case 1: return 4;
        case 2: return 10;
        case 3: return 14;
        case 4: return 18;
        case 5: return 22;
        case 6: return 26;
        case 7: return 27;   // 命运残线 = 空王座前的最后残影
        }
    }
    return -1;
}

// 把一次 Encounter 展开成群怪（下标可重复 = 同种怪上多只）
static vector<Enemy> MakeGroup(const Encounter& ec) {
    vector<Enemy> group;
    for (int idx : ec.enemy) {
        group.push_back(g_enemies[idx]);
    }
    return group;
}

// ===== 战后奖励 =====

// 金币:35~45
static void RewardGold(Player& p) {
    const int g = 35 + rand() % 11;
    p.coin += g;
    cout << "\n掉落 " << g << " 金币 (当前 " << p.coin << ")。\n";
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

// 卡牌:必出,从全部卡里随机抽 3 张不重复,选 1 张带走(0 跳过)
static void RewardCards(Player& p) {
    int picks[3];
    for (int k = 0; k < 3; ++k) {                   // 抽 3 张不重复的
        const int idx = rand() % (int)g_cards.size();
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

// 战斗房间：开打 → 胜利发金币 → 标记完成
static bool RunCombatRoom(Player& player, FloorManager& manager,
    const Room& room, int floor) {
    const int encounterIndex = EncounterForRoom(floor, room.getType());
    if (encounterIndex < 0 ||
        encounterIndex >= static_cast<int>(g_encounters.size())) {
        cout << "\n这个房间还没有配置敌人。\n";
        return false;
    }

    player.Reset();

    const Encounter& encounter = g_encounters[encounterIndex];
    vector<Enemy> enemies = MakeGroup(encounter);

    Combat combat(player, enemies);
    if (!combat.Run()) {
        cout << "\n你在房间【" << room.getName() << "】中战败了。\n";
        return false;
    }

    cout << "\n战斗胜利！\n";

    RewardGold(player);      // 金币(必掉)
    RewardPotion(player);    // 药水(70% 概率)
    RewardCards(player);     // 卡牌(必出,三选一/跳过)

    manager.clearCurrentRoom();
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

    bool quit = false;
    while (!quit) {
        const Room& cur = manager.getCurrentRoom();
        cout << "\n========== [第 " << manager.getCurrentFloor() << " 层] "
            << cur.getColoredName() << " ==========\n";
        cout << "\n" << cur.getDesc() << "\n";
        cout << "[类型: " << cur.getTypeNameWithColor() << "]\n";
        cout << "[HP " << player.hp << "/" << player.maxHp
            << "  金币 " << player.coin << "]\n";

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
            cout << "\n可选路径:\n";
            for (size_t i = 0; i < options.size(); ++i) {
                const Room& next = manager.getRoomById(options[i]);
                cout << "  " << (i + 1) << ". " << next.getColoredName()
                    << " (" << next.getTypeNameWithColor() << ")\n";
            }
            cout << "\n请输入数字选择，或输入 'map'、'up'、'quit': ";

            string input;
            cin >> input;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            if (input == "quit" || input == "q") {
                quit = true;
                break;
            }
            if (input == "map" || input == "m") {
                cout << "\033[2J\033[2;1H";
                manager.drawMap();
                continue;
            }
            if (input == "up") {
                if (manager.tryGoUp()) {
                    cout << "\033[2J\033[H";
                    cout << "你踏入了下一层...\n";
                    continue;
                }
                cout << "\033[2J\033[H";
                cout << "必须击败BOSS才能上楼！\n";
                continue;
            }

            try {
                const int choice = stoi(input);
                if (choice >= 1 && choice <= static_cast<int>(options.size())) {
                    manager.moveTo(options[choice - 1]);

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
                        RewardGold(player);      // 金币
                        RewardRelic(player);     // 遗物
                        manager.clearCurrentRoom();
                        break;
                    }

                    case RoomType::Event: {
                        cout << "\n你触发了事件。\n";
                        cout << "1. 接受交易（支付 30 金币，获得一件随机遗物）\n";
                        cout << "2. 就地休息（恢复最大生命的 30%）\n";
                        cout << "3. 转身离开\n";
                        cout << "> ";
                        string in;
                        cin >> in;
                        cin.ignore(numeric_limits<streamsize>::max(), '\n');
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
                        manager.clearCurrentRoom();
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
            cin >> input;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            if (input == "quit" || input == "q") {
                quit = true;
            }
            else if (input == "up") {
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