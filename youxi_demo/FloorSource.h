#pragma once
#include <string>
#include <unordered_map>
#include "GameMode.h"
#include "MapTemplates.h"
#include "Room.h"
#include "FloorData.h"

// FloorSource 负责回答“当前模式和楼层应该使用哪一份楼层内容”
namespace FloorSource {

    // 故事模式：始终使用 FloorBuilder 的固定文本。
    // 无尽模式：前 7 层使用固定文本，第 8 层起使用 FloorGenerator，
    //           并循环复用前面第 1~6 层的主题文本。
    // 返回统一的楼层数据（含房间、地图模板与是否固定剧情标记）
    FloorData load(int floor, GameMode mode, unsigned int seed);
}
