// main.cpp
#include "FloorManager.h"
#include <iostream>
#include <limits>
#include <string>

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

int main() {
    Color::init();

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

        if (cur.hasReward() && !cur.isCompleted()) {
            auto reward = manager.clearCurrentRoom();
            if (reward.has_value()) {
                cout << "\n[奖励]\n";
                if (reward->gold > 0) cout << "  +" << reward->gold << " 金币\n";
                if (reward->exp > 0) cout << "  +" << reward->exp << " 经验\n";
                if (!reward->item_name.empty())
                    cout << "  获得: " << reward->item_name
                        << " (" << reward->item_effect << ")\n";
                if (!reward->flavor_text.empty())
                    cout << "  " << reward->flavor_text << "\n";
            }
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
                cout << "  " << (i + 1) << ". " << next.getColoredName() << "\n";
                printIndented(next.getDesc(), "     ");
            }
            cout << "\n请输入数字选择，或输入 'map'、'up'、'quit': ";

            string input;
            cin >> input;
            if (input == "quit" || input == "q" || input == "QUIT" || input == "Q") {
                quit = true;
                break;
            }
            if (input == "map" || input == "m" || input == "MAP" || input == "M") {
                cout << "\033[2J\033[H";
                manager.drawMap();
                cout << "\n请按Enter键继续...\n";
                cin.get();
                cin.get();
                cout << "\033[2J\033[H";
                continue;
            }
            if (input == "up" || input == "u"|| input == "UP"|| input == "U") {
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
            cin >> input;
            if (input == "quit" || input == "q" || input == "QUIT" || input == "Q") {
                quit = true;
            }
            else if (input == "up" || input == "u" || input == "UP" || input == "U") {
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
