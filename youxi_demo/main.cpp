// main.cpp —— 逆神者之塔:爬塔主循环(已接入卡牌战斗系统)
//
// 合并说明:
//   在原有"地图 / 楼层 / 导航"框架之上,接入了战斗系统的战斗引擎。
//   玩家的一条命贯穿整座塔:HP / 金币 / 牌组 / 遗物 / 药水跨楼层保留,
//   战斗房(战斗 / 精英 / 首领)必须真正打赢才能通过,商店房进入商店。
//   地图显示沿用队友优化:去路展开房间描述 + 命令支持大写 + 看地图后暂停。
#include "FloorManager.h"
#include "Combat.h"
#include "Player.h"
#include "EncounterBuilder.h"
#include "shop.h"
#include <iostream>
#include <limits>
#include <string>
#include <cstdlib>
#include <ctime>
#include <cctype>
#include <algorithm>

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

    string input;
    cin >> input;
    if (input == "1") {
        cout << "\n你吞下了空王座上的自己。塔在你身后崩塌，你升上天空。\n";
        cout << "世界恢复和平，但再无人能登塔。\n";
        cout << "——结局：封神——\n";
    }
    else if (input == "2") {
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

// 按统一前缀逐行打印文本，保持菜单排版
void printIndented(const string& text, const string& prefix) {
    size_t start = 0;
    while (start <= text.size()) {
        const size_t end = text.find('\n', start);
        const string line = (end == string::npos)
            ? text.substr(start)
            : text.substr(start, end - start);
        if (line.empty())
            cout << "\n";
        else
            cout << prefix << line << "\n";
        if (end == string::npos)
            break;
        start = end + 1;
    }
}

// 命令规整:去掉首尾空白并转小写,让 'MAP' / 'Map' / 'm' 等价
string normalizeCommand(const string& s) {
    const size_t a = s.find_first_not_of(" \t\r\n");
    if (a == string::npos) return "";
    const size_t b = s.find_last_not_of(" \t\r\n");
    string r = s.substr(a, b - a + 1);
    for (char& c : r) c = static_cast<char>(tolower(static_cast<unsigned char>(c)));
    return r;
}

// ==================== 战斗接入 ====================

// 每场战斗前:清掉"对局内"的临时状态,
// 但 HP / 金币 / 牌组 / 遗物 / 药水 一律保留 —— 这才是爬塔该有的语义。
void resetBattleState(Player& p) {
    p.block = 0;
    p.strength = 0;
    p.toughness = 0;
    p.nextAtkMult = 0;
    p.weak = 0;
    p.vulnerable = 0;
    p.apothGauge = 0;
    p.apotheosis = false;
}

// 打一场战斗。返回 true = 玩家获胜,false = 玩家阵亡。
bool runBattle(Player& player, int floor, RoomType type) {
    EncounterBuilder::Setup setup = EncounterBuilder::build(floor, type);

    cout << "\n========================================\n";
    cout << "  遭遇战:" << setup.group_name << "\n";
    cout << "========================================\n";
    cout << "按 Enter 进入战斗...";
    string dummy;
    getline(cin, dummy);

    resetBattleState(player);
    Combat combat(player, setup.enemies);
    return combat.Run();
}

// 结算一场胜利的奖励(金币直接计入玩家口袋)
void grantReward(Player& player, const RoomReward& reward) {
    cout << "\n[战利品]\n";
    if (reward.gold > 0) {
        player.coin += reward.gold;
        cout << "  +" << reward.gold << " 金币 (当前 " << player.coin << ")\n";
    }
    if (reward.exp > 0) cout << "  +" << reward.exp << " 经验\n";
    if (!reward.item_name.empty())
        cout << "  获得: " << reward.item_name << " (" << reward.item_effect << ")\n";
    if (!reward.flavor_text.empty())
        cout << "  " << reward.flavor_text << "\n";
}

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
    cin >> seed;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    FloorManager manager;
    manager.setGameMode(mode_choice == 1 ? GameMode::Story : GameMode::Endless);
    try {
        manager.loadFloor(1, seed);
    }
    catch (const exception& e) {
        cerr << "\n加载楼层失败: " << e.what() << "\n";
        return 1;
    }

    // 玩家:一条命贯穿整座塔
    Player player;
    player.NewRun();

    cout << "\n地图种子: " << seed << "\n";
    cout << "当前模式: "
        << (manager.getGameMode() == GameMode::Story ? "故事模式" : "无尽模式") << "\n";
    cout << "初始牌组 " << player.startDeck.size() << " 张"
        << " / 生命 " << player.hp << "/" << player.maxHp
        << " / 遗物 " << player.relics.size()
        << " / 药水 " << player.potions.size() << "\n";
    cout << "地图加载完成！\n\n请按Enter键继续...\n";
    cin.get();

    cout << "\033[2J\033[2;1H";

    string dummy;          // 各处"按 Enter 继续"用
    string entered_shop;   // 已进过的商店房 id(避免原地反复开店)
    bool quit = false;

    while (!quit) {
        const Room& cur = manager.getCurrentRoom();
        cout << "\n========== [第 " << manager.getCurrentFloor() << " 层] "
            << cur.getName() << " ==========\n";
        cout << "[生命 " << player.hp << "/" << player.maxHp
            << "   金币 " << player.coin << "]\n";
        cout << "\n" << cur.getDesc() << "\n";

        // ---------- 战斗房(战斗 / 精英 / 首领):真打一场 ----------
        const bool battle_room = cur.isCombat() || cur.isBoss();
        if (battle_room && !cur.isCompleted()) {
            if (!runBattle(player, manager.getCurrentFloor(), cur.getType())) {
                cout << "\n你在第 " << manager.getCurrentFloor() << " 层倒下了。\n";
                cout << "—— 登塔失败 ——\n";
                return 0;
            }
            cout << "\n[战斗胜利] 生命 " << player.hp << "/" << player.maxHp << "\n";
            // 爬塔续航:每场胜利后恢复一小部分生命,让连续作战成为可能
            {
                const int before = player.hp;
                player.hp = std::min(player.maxHp, player.hp + player.maxHp / 8);
                if (player.hp > before)
                    cout << "[休整] 恢复 " << (player.hp - before)
                         << " 点生命 (" << player.hp << "/" << player.maxHp << ")\n";
            }
            auto reward = manager.clearCurrentRoom();
            if (reward.has_value()) grantReward(player, *reward);
            cout << "\n按 Enter 继续...";
            getline(cin, dummy);
            continue;
        }

        // ---------- 非战斗房(宝箱 / 事件):直接领奖 ----------
        if (!battle_room && cur.hasReward() && !cur.isCompleted()) {
            auto reward = manager.clearCurrentRoom();
            if (reward.has_value()) grantReward(player, *reward);
        }

        // ---------- 商店房:进入战斗系统的商店 ----------
        if (cur.getType() == RoomType::Shop && cur.getId() != entered_shop) {
            entered_shop = cur.getId();
            cout << "\n你走进了一间商店...按 Enter 进店。\n";
            getline(cin, dummy);
            Shop shop;
            shop.Enter(player);
            continue;
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
            cout << "\n前方有几个可能的去路：\n";
            for (size_t i = 0; i < options.size(); ++i) {
                const Room& next = manager.getRoomById(options[i]);
                cout << "  " << (i + 1) << ". " << next.getName() << "\n";
                printIndented(next.getDesc(), "     ");
            }
            cout << "\n请输入数字选择，或输入 'map'、'up'、'quit': ";

            string input;
            getline(cin, input);
            if (cin.eof()) { quit = true; break; }   // 输入结束(如管道用尽)则退出
            const string cmd = normalizeCommand(input);
            if (cmd == "quit" || cmd == "q") {
                quit = true;
                break;
            }
            if (cmd == "map" || cmd == "m") {
                cout << "\033[2J\033[H";
                manager.drawMap();
                cout << "\n请按Enter键继续...";
                getline(cin, dummy);
                if (cin.eof()) { quit = true; break; }
                cout << "\033[2J\033[H";
                continue;
            }
            if (cmd == "up" || cmd == "u") {
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
                const int choice = stoi(cmd);
                if (choice >= 1 && choice <= static_cast<int>(options.size())) {
                    manager.moveTo(options[choice - 1]);
                    cout << "\033[2J\033[2;1H";
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
            getline(cin, input);
            if (cin.eof()) { quit = true; break; }   // 输入结束则退出
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

        cout << "\033[2J\033[H";
    }
    return 0;
}
