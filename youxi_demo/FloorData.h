#pragma once
#include <string>
#include <unordered_map>
#include "Room.h"
#include "MapTemplates.h"

// Unified floor-data contract. Every content source (FloorBuilder hand-authored
// or FloorGenerator procedural) returns this single struct, replacing the three
// previously duplicated definitions FloorData / GenerationResult / LoadedFloor.
struct FloorData {
    std::unordered_map<std::string, Room> rooms;   // roomId -> Room
    MapTemplate tmpl;                               // map topology (coords + edges)
    bool fixed_layout = false;                      // true = hand-authored fixed story floor
};
