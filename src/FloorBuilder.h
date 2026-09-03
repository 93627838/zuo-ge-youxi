#pragma once
#include <unordered_map>
#include <string>

class Room;

namespace FloorBuilder {
    // 声明各层的构建函数
    std::unordered_map<std::string, Room> buildFloor1();
    std::unordered_map<std::string, Room> buildFloor2();
    std::unordered_map<std::string, Room> buildFloor3();
    std::unordered_map<std::string, Room> buildFloor4();
    std::unordered_map<std::string, Room> buildFloor5();
    std::unordered_map<std::string, Room> buildFloor6();
    std::unordered_map<std::string, Room> buildFloor7();
}