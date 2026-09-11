#pragma once
#include <string>
#include <vector>
#include "Enemy.h"
#include "Room.h"

// ============================================================
//  EncounterBuilder —— 爬塔楼层 与 战斗系统 之间的桥接层
//
//  职责:爬塔那边只知道"这是一间战斗房(RoomType)";战斗系统那边只有
//        一张按内容分好组的出场表(g_encounters)。本模块把两者接起来:
//          楼层 + 房间类型  ->  这一场该打哪些敌人(实例化 + 数值强化)
//
//  之所以单独成一层,是为了不改动战斗系统原作者的 Data.h/.cpp;
//  以后要调整难度曲线,只动这一个文件即可。
// ============================================================
namespace EncounterBuilder {

    struct Setup {
        std::string group_name;       // 这一场的显示名(如"烬灰尸群")
        std::vector<Enemy> enemies;   // 实例化好的敌人(已做数值强化)
    };

    // floor: 爬塔当前楼层(从 1 起;无尽模式第 8 层起会循环映射回 1~6 层的内容)
    // type:  房间类型(Combat / Elite / Boss)
    Setup build(int floor, RoomType type);

}
