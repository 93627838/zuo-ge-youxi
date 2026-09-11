#pragma once
#include "Player.h"

// 顶层流程:主菜单状态机 -> 选择对手 -> 进入战斗 -> 结算
class Game {
public:
    void Run();

private:
    void MainMenu();
    void NewBattle();

    Player player;
    bool freshRun = true;   // true = 下一次开始战斗要开新一局(初始化遗物/药水/牌组/金币)
};
