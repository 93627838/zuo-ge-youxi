// main.cpp
#include "FloorManager.h"
#include <iostream>
#include <string>
#include <limits>

using namespace std;

int main() {
    
    Color::init();

    cout << "========================================\n";
    cout << "       逆神者之塔 - 肉鸽爬塔           \n";
    cout << "========================================\n\n";

    unsigned int seed;
    cout << "请输入地图种子 (0 表示随机): ";
    cin >> seed;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    FloorManager manager;
    try {
        manager.loadFloor(1, seed);
    }
    catch (const exception& e) {
        cerr << "\n加载楼层失败: " << e.what() << "\n";
        return 1;
    }
	cout << "\n您输入的地图种子为: " << seed << "\n";
	cout << "地图加载完成！\n\n请按Enter键继续...\n";
	cin.get();
    cout << "\033[2J\033[2;1H";

    bool quit = false;
    while (!quit) {
        const Room& cur = manager.getCurrentRoom();
        cout << "\n========== " << cur.getColoredName() << " ==========\n";
        cout << "\n" << cur.getDesc() << "\n";
        cout << "[类型: " << cur.getTypeNameWithColor() << "]\n";

        // 自动领奖
        if (cur.hasReward() && !cur.isCompleted()) {
            auto reward = manager.clearCurrentRoom();
            if (reward.has_value()) {
                cout << "\n[奖励]\n";
                if (reward->gold > 0) cout << "  +" << reward->gold << " 金币\n";
                if (reward->exp > 0)  cout << "  +" << reward->exp << " 经验\n";
                if (!reward->item_name.empty())
                    cout << "  获得: " << reward->item_name << " (" << reward->item_effect << ")\n";
                if (!reward->flavor_text.empty())
                    cout << "  " << reward->flavor_text << "\n";
            }
        }

        if (cur.isBoss() && manager.isBossDefeated()) {
            cout << "\n[提示] BOSS已击败！输入 'up' 前往下一层。\n";
        }

        auto options = manager.getCurrentOptions();

        if (options.empty() && !cur.isBoss()) {
            cout << "\n此路已尽，但未遇到BOSS？请检查地图模板。\n";
            break;
        }

        if (!options.empty()) {
            cout << "\n可选路径:\n";
            for (size_t i = 0; i < options.size(); ++i) {
                const Room& next = manager.getRoomById(options[i]);
                cout << "  " << (i + 1) << ". " << next.getColoredName()
                    << " (" << next.getTypeNameWithColor() << ")\n";
            }
            cout << "\n请输入数字选择，或输入 'map' 查看地图，'up' 上楼，'quit' 退出: ";
            string input;
            cin >> input;

            if (input == "quit" || input == "q") {
                quit = true;
                cout << "\033[2J\033[2;1H";
                cout << "感谢游玩！\n";
                break;
            }
            else if (input == "map" || input == "m") {
                cout << "\033[2J\033[2;1H";
                manager.drawMap();   // 绘制地图
                continue;            // 重新显示当前房间
            }
            else if (input == "up") {
                if (manager.tryGoUp()) {
                    cout << "\n你踏入了下一层...\n";
                }
                else {
                    cout << "\n必须击败BOSS才能上楼！\n";
                }
            }
            else {
                try {
                    int choice = stoi(input);
                    if (choice >= 1 && choice <= (int)options.size()) {
                        manager.moveTo(options[choice - 1]);
                    }
                    else {
                        std::cout << "\033[2J\033[H";
                        cout << "无效选择。\n";
                        continue;
                    }
                }
                catch (...) {
                    cout << "\033[2J\033[H";
                    cout << "无效输入，请输入数字、'map'、'up' 或 'quit'。\n";
                    continue;
                }
            }
        }
        else if (cur.isBoss() && manager.isBossDefeated()) {
            cout << "输入 'up' 上楼，或 'quit' 退出: ";
            string input;
            cin >> input;
            if (input == "quit" || input == "q") {
                quit = true;
            }
            else if (input == "up") {
                manager.tryGoUp();
                cout << "\033[2J\033[H";
                cout << "你踏入了下一层...\n";
                continue;
            }
            else {
                cout << "\033[2J\033[H";
                cout << "无效输入。\n";
                continue;
            }
        }
        cout << "\033[2J\033[2;1H";
    }
    return 0;
} 