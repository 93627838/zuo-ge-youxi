#pragma once
#include <string>
#include <vector>
#include <unordered_map>

enum class RoomType : unsigned char {
    Start,      // 起始点（无战斗，无奖励）
    Combat,     // 普通战斗
    Elite,      // 精英战斗
    Boss,       // 首领战斗
    Treasure,   // 直接领取的宝箱
    Event,      // 事件（选择后获得奖励）
    Shop        // 商店（交互由外部系统处理）
};

// 奖励数据结构
struct RoomReward {
    int gold = 0;
    int exp = 0;
    std::string item_name;      // 获得的道具ID
    std::string item_effect;    // 效果描述（用于显示）
    std::string flavor_text;    // 领取时的旁白
};

class Room {
private:
    std::string id;
    int floor;
    std::string name;
    std::string desc;
    RoomType type;
    RoomReward reward;
    bool is_cleared = false;          // 是否已完成（防止重复战斗/领奖）
    std::vector<std::string> next_ids; // 后续节点ID列表（顺序即选项顺序）

public:
    Room() = default; // 供容器使用

    Room(const std::string& id, int f, const std::string& name,
        const std::string& desc, RoomType t)
        : id(id), floor(f), name(name), desc(desc), type(t) {
    }

    // ---------- 查询接口 ----------
    const std::string& getId() const { return id; }
    int getFloor() const { return floor; }
    const std::string& getName() const { return name; }
    const std::string& getDesc() const { return desc; }
    RoomType getType() const { return type; }
    const RoomReward& getReward() const { return reward; }
    const std::vector<std::string>& getNextIds() const { return next_ids; }
    bool isCompleted() const { return is_cleared; }

    // ---------- 便捷类型判断 ----------
    bool isCombat() const { return type == RoomType::Combat || type == RoomType::Elite; }
    bool isBoss() const { return type == RoomType::Boss; }
    bool hasReward() const { return type != RoomType::Start && type != RoomType::Shop; }

    // ---------- 修改接口（由FloorManager调用） ----------
    void setReward(const RoomReward& r) { reward = r; }
    void addNext(const std::string& next_id) { next_ids.push_back(next_id); }
    void markCleared() { is_cleared = true; }

    // ---------- 工具函数：获取类型的中文描述 ----------
    std::string getTypeNameWithColor() const {
        switch (type) {
        case RoomType::Start:    return "\033[37m起点\033[0m";   // 白色
        case RoomType::Combat:   return "\033[90m战斗\033[0m";   // 灰色
        case RoomType::Elite:    return "\033[33m精英\033[0m";   // 黄色
        case RoomType::Boss:     return "\033[31m首领\033[0m";   // 红色
        case RoomType::Treasure: return "\033[36m宝藏\033[0m";   // 青色
        case RoomType::Event:    return "\033[35m事件\033[0m";   // 品红
        case RoomType::Shop:     return "\033[32m商店\033[0m";   // 绿色
        default: return "未知";
        }
    }

    std::string getColoredName() const {
        std::string color;
        switch (type) {
        case RoomType::Start:    color = "\033[37m"; break; // 白色
        case RoomType::Combat:   color = "\033[90m"; break; // 灰色
        case RoomType::Elite:    color = "\033[33m"; break; // 黄色
        case RoomType::Boss:     color = "\033[31m"; break; // 红色
        case RoomType::Treasure: color = "\033[36m"; break; // 青色
        case RoomType::Event:    color = "\033[35m"; break; // 品红
        case RoomType::Shop:     color = "\033[32m"; break; // 绿色
        default: return name;
        }
        return color + name + "\033[0m";
    }
};