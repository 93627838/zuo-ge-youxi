#pragma once
#include <unordered_map>
#include <string>
#include <optional>
#include "Room.h"

class FloorManager {
private:
    std::unordered_map<std::string, Room> rooms;  // 当前层所有房间 (ID -> Room)
    std::string current_room_id;                  // 玩家当前所在节点ID
    int current_floor = 1;
    bool boss_defeated = false;

public:
    // ---------- 楼层加载 ----------
    void loadFloor(int floor);

    // ---------- 查询 ----------
    const Room& getCurrentRoom() const;           // 获取当前房间（只读）
    const Room& getRoomById(const std::string& id) const;
    int getCurrentFloor() const { return current_floor; }
    bool isBossDefeated() const { return boss_defeated; }

    // ---------- 导航 ----------
    // 尝试移动到指定ID的房间，返回是否成功
    bool moveTo(const std::string& target_id);

    // 获取当前房间的后续可选节点列表（用于显示选项）
    std::vector<std::string> getCurrentOptions() const;

    // ---------- 状态修改 ----------
    // 完成当前房间（战斗/事件/宝箱），返回其奖励；若已清除则返回 nullopt
    std::optional<RoomReward> clearCurrentRoom();

    // 尝试上楼（必须在击败BOSS后）
    bool tryGoUp();

private:
    // 构建房间数据（内部调用 FloorBuilder）
    void buildFloorData(int floor);
};