// FloorManager.cpp
#include "FloorManager.h"
#include "FloorBuilder.h"
#include <stdexcept>
#include <iostream>

using namespace std;

void FloorManager::loadFloor(int floor) {
    current_floor = floor;
    boss_defeated = false;
    buildFloorData(floor);
    // 默认进入该层的第一个房间（约定ID为 "F{floor}_Start"）
    string start_id = "F" + to_string(floor) + "_Start";
    if (rooms.find(start_id) != rooms.end()) {
        current_room_id = start_id;
    }
    else if (!rooms.empty()) {
        current_room_id = rooms.begin()->first; // 降级方案：取第一个
    }
    else {
        throw runtime_error("楼层数据为空！");
    }
}

void FloorManager::buildFloorData(int floor) {
    switch (floor) {
    case 1: rooms = FloorBuilder::buildFloor1(); break;
    case 2: rooms = FloorBuilder::buildFloor2(); break;
        // ... 后续楼层
    default:
        throw runtime_error("未实现的楼层编号");
    }
}

const Room& FloorManager::getCurrentRoom() const {
    auto it = rooms.find(current_room_id);
    if (it == rooms.end()) {
        throw runtime_error("当前房间ID无效");
    }
    return it->second;
}

const Room& FloorManager::getRoomById(const string& id) const {
    auto it = rooms.find(id);
    if (it == rooms.end()) {
        throw runtime_error("房间ID " + id + " 不存在");
    }
    return it->second;
}

vector<string> FloorManager::getCurrentOptions() const {
    const auto& room = getCurrentRoom();
    return room.getNextIds(); // 直接返回后续节点列表
}

bool FloorManager::moveTo(const string& target_id) {
    // 1. 检查目标ID是否在当前层地图中
    if (rooms.find(target_id) == rooms.end()) {
        return false;
    }

    // 2. 检查目标ID是否在当前房间的出口列表中（防止玩家跳到不连通的节点）
    const auto& current = getCurrentRoom();
    const auto& nexts = current.getNextIds();
    if (find(nexts.begin(), nexts.end(), target_id) == nexts.end()) {
        return false; // 目标不是当前房间的合法后续
    }

    // 3. 执行移动
    current_room_id = target_id;
    return true;
}

optional<RoomReward> FloorManager::clearCurrentRoom() {
    auto& room = rooms[current_room_id]; // 需要非常量引用修改状态
    if (room.isCompleted()) {
        return nullopt; // 已经领过了
    }

    // 标记已清理
    room.markCleared();

    // 如果当前房间是BOSS，标记BOSS已击败
    if (room.isBoss()) {
        boss_defeated = true;
    }

    // 返回奖励（拷贝）
    return room.getReward();
}

bool FloorManager::tryGoUp() {
    if (!boss_defeated) {
        return false;
    }
    // 加载下一层
    loadFloor(current_floor + 1);
    return true;
}