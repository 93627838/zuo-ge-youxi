#pragma once

#include <string>

#include "shared_types.h"

class SaveManager {
public:
    static bool saveGame(const std::string& path, const GameState& state);
    static bool loadGame(const std::string& path, GameState& outState);
};
