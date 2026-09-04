// FloorManager.h
#pragma once
#include <unordered_map>
#include <string>
#include <optional>
#include <vector>
#include "Room.h"
#include "MapTemplates.h"
#include "GameMode.h"

namespace Color {
    void init();   // 声明初始化函数
}

class FloorManager {
private:
    std::unordered_map<std::string, Room> rooms;   // 当前楼层所有房间 (ID -> Room)
    std::string current_room_id;                    // 玩家当前所在房间ID
    int current_floor = 1;                          // 当前楼层
    bool boss_defeated = false;                     // BOSS是否已被击败
    unsigned int map_seed = 0;                      // 地图种子 (0表示随机)
    MapTemplate current_tmpl;                       // 当前楼层的地图模板 (含坐标和连接)
    bool current_is_fixed = false;                  // 当前楼层是否为固定剧情地图
    GameMode game_mode = GameMode::Story;            // 当前游戏模式

public:
    FloorManager();

    // ---------- 楼层加载 ----------
    void loadFloor(int floor, unsigned int seed = 0);

    // ---------- 查询 ----------
    const Room& getCurrentRoom() const;
    const Room& getRoomById(const std::string& id) const;
    int getCurrentFloor() const { return current_floor; }
    bool isBossDefeated() const { return boss_defeated; }
    GameMode getGameMode() const { return game_mode; }
    bool isStoryEnd() const;                         // 故事模式是否已在第7层通关

    // ---------- 模式设置 ----------
    void setGameMode(GameMode mode) { game_mode = mode; }

    // ---------- 导航 ----------
    bool moveTo(const std::string& target_id);
    std::vector<std::string> getCurrentOptions() const;

    // ---------- 房间操作 ----------
    std::optional<RoomReward> clearCurrentRoom();
    bool tryGoUp();

    // ---------- 地图显示 ----------
    void drawMap() const;

private:
    void drawFixedMap() const;
};
