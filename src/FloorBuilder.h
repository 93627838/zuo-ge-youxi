#pragma once
#include <unordered_map>
#include <string>
#include "Room.h"
#include "MapTemplates.h"

struct FloorData {
    std::unordered_map<std::string, Room> rooms;
    MapTemplate tmpl;
};

class FloorBuilder {
public:
    static std::unordered_map<std::string, Room> buildFloor1();
    static std::unordered_map<std::string, Room> buildFloor2();
    static std::unordered_map<std::string, Room> buildFloor3();
    static std::unordered_map<std::string, Room> buildFloor4();
    static std::unordered_map<std::string, Room> buildFloor5();
    static std::unordered_map<std::string, Room> buildFloor6();
    static std::unordered_map<std::string, Room> buildFloor7();

    static void buildFloorData(int floor, FloorData& data);
};