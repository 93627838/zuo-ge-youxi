#include "shared_types.h"
#include "console_ui.h"
#include "command_parser.h"
#include "save_manager.h"
#include <iostream>
#include <string>
#include <windows.h>

int main() {
    SetConsoleOutputCP(CP_UTF8); // 或 system("chcp 65001 > nul");
    GameState state;
    state.currentFloor = 3;
    state.player.maxHp = 100;
    state.player.str = 10;
    state.player.def = 5;
    state.player.gold = 999;
    state.pos = {1, 2, 3};
    state.inventory = {"生命药水", "神骸碎片"};

    ConsoleUI::printHeader("=== 逆神者之塔 - 测试模式 ===");
    ConsoleUI::printStatus("当前楼层", std::to_string(state.currentFloor));
    ConsoleUI::printStatus("金币", std::to_string(state.player.gold));

    CommandParser::Command cmd = CommandParser::getInstance().parse("攻击 哥布林");
    ConsoleUI::printStatus("解析命令", cmd.type + " -> " + (cmd.args.empty() ? "" : cmd.args[0]));

    if (SaveManager::saveGame("save.txt", state)) {
        ConsoleUI::printStatus("存档", "成功保存到 save.txt");
    } else {
        ConsoleUI::printStatus("存档", "保存失败！");
    }

    GameState loadedState;
    if (SaveManager::loadGame("save.txt", loadedState)) {
        ConsoleUI::printStatus("读档", "成功加载，楼层：" + std::to_string(loadedState.currentFloor));
    } else {
        ConsoleUI::printStatus("读档", "加载失败！");
    }

    ConsoleUI::printDivider('=');
    std::cout << "\n测试完成！按 Enter 退出...";
    std::cin.get();
    return 0;
}