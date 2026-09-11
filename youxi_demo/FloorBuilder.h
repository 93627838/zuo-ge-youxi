#pragma once
#include <unordered_map>
#include <string>
#include "Room.h"
#include "MapTemplates.h"
#include "FloorData.h"

class FloorBuilder {
public:
    static std::unordered_map<std::string, Room> buildFloor1();
    static std::unordered_map<std::string, Room> buildFloor2();
    static std::unordered_map<std::string, Room> buildFloor3();
    static std::unordered_map<std::string, Room> buildFloor4();
    static std::unordered_map<std::string, Room> buildFloor5();
    static std::unordered_map<std::string, Room> buildFloor6();
    static std::unordered_map<std::string, Room> buildFloor7();
    // NOTE: buildFloorData(int, FloorData&) was legacy dead code and has been removed.
};