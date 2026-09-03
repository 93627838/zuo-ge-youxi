#include "FloorManager.h"
#include <iostream>

int main() {
    

    FloorManager manager;
    manager.loadFloor(1);  // 加载第一层
    std::cout << "\033[2J\033[2;1H";

    while (true) {
        
        const Room& cur = manager.getCurrentRoom();
		
        // 1. 显示房间
        std::cout << "========== " << cur.getName() << " ==========\n";
        std::cout << cur.getDesc() << "\n";

        // 2. 如果当前房间有奖励且未领取，自动触发（战斗/事件/宝箱）
        if (cur.hasReward() && !cur.isCompleted()) {
            auto reward_opt = manager.clearCurrentRoom();
            if (reward_opt.has_value()) {
                auto& reward = reward_opt.value();
                std::cout << "\n[奖励]\n";
                if (reward.gold > 0) std::cout << "  +" << reward.gold << " 金币\n";
                if (reward.exp > 0)  std::cout << "  +" << reward.exp << " 经验\n";
                if (!reward.item_name.empty())
                    std::cout << "  获得: " << reward.item_name << " (" << reward.item_effect << ")\n";
                if (!reward.flavor_text.empty())
                    std::cout << "  " << reward.flavor_text << "\n";
            }
        }

        // 3. 检查是否击败BOSS
        if (cur.isBoss() && manager.isBossDefeated()) {
            std::cout << "\n[提示] 你击败了BOSS，可以输入 'up' 前往下一层。\n";
        }

        // 4. 显示后续路径
        auto options = manager.getCurrentOptions();
        if (options.empty() && !cur.isBoss()) {
            std::cout << "\n此路已尽，但还未遇到BOSS，请检查地图设计。\n";
            break;
        }
        if (!options.empty()) {
            std::cout << "\n前方可选路径:\n";
            for (size_t i = 0; i < options.size(); ++i) {
                const Room& next = manager.getRoomById(options[i]);
                std::cout << "  " << (i + 1) << ". " << next.getName()
                    << " (" << next.getTypeName() << ")\n";
            }
            if (manager.tryGoUp()) {
                std::cout << "请输入数字  (或输入 'up' 上楼 ) : ";
            }
            else {
                std::cout << "请输入数字 : ";
            }
            std::string input;
            std::cin >> input;

            if (input == "up") {
                if (manager.tryGoUp()) {
                    std::cout << "你踏入上一层...\n";
                    continue;
                }
                else {
                    std::cout << "BOSS未击败，无法上楼！\n";
                }
            }
            else {
                int choice = std::stoi(input);
                if (choice >= 1 && choice <= (int)options.size()) {
                    manager.moveTo(options[choice - 1]);
                }
                else {
                    std::cout << "\033[2J\033[H";
                    std::cout << "无效输入。\n";
                    
					continue;
                }
            }
        }
        else if (cur.isBoss() && manager.isBossDefeated()) {
            // BOSS已打完，等待玩家输入 up
            std::cout << "输入 'up' 前往下一层: ";
            std::string input;
            std::cin >> input;
            if (input == "up") {
                manager.tryGoUp();
                std::cout << "你踏入上一层...\n";
            }
        }
        std::cout << "\033[2J\033[2;1H";
    }
    return 0;
}