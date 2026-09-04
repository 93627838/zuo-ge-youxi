// FloorGenerator.h
#pragma once
#include <unordered_map>
#include <string>
#include <random>
#include "Room.h"
#include "MapTemplates.h"

struct GenerationResult {
    std::unordered_map<std::string, Room> rooms;
    MapTemplate tmpl;
};

class FloorGenerator {
public:
    // text_floor：生成楼层使用的文本主题；0 表示与楼层号一致
    GenerationResult generateFloor(int floor, unsigned int seed = 0, int text_floor = 0);

private:
    std::string genId(int floor, int nodeIndex);
    const std::vector<std::string>& getNames(RoomType type, int floor);
    const std::vector<std::string>& getDescs(RoomType type, int floor);
    RoomReward generateReward(RoomType type, int floor, std::mt19937& rng);
};
