#pragma once
#include <string>
#include <unordered_map>
#include "GameMode.h"
#include "MapTemplates.h"
#include "Room.h"

// 一次加载完成的楼层数据
struct LoadedFloor {
    std::unordered_map<std::string, Room> rooms;
    MapTemplate tmpl;
    bool fixed_layout = false;   // 固定剧情地图没有坐标模板，只能文字显示
};

// FloorSource 负责回答“当前模式和楼层应该使用哪一份楼层内容”
namespace FloorSource {

    // 故事模式：始终使用 FloorBuilder 的固定文本。
    // 无尽模式：前 7 层使用固定文本，第 8 层起使用 FloorGenerator，
    //           并循环复用前面第 1~6 层的主题文本。
    LoadedFloor load(int floor, GameMode mode, unsigned int seed);
}
