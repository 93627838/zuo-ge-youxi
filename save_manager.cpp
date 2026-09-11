#include "save_manager.h"

#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cstddef>

namespace {

bool parseCards(std::istream& is, std::vector<Card>& out) {
    std::vector<std::string> tokens;
    std::string tok;
    while (is >> tok) {
        tokens.push_back(tok);
    }

    if (tokens.size() % 5 != 0) {
        return false;
    }

    for (std::size_t i = 0; i < tokens.size(); i += 5) {
        Card c;
        try {
            c.id = std::stoi(tokens[i]);
            c.name = tokens[i + 1];
            c.cost = std::stoi(tokens[i + 2]);
            c.damage = std::stoi(tokens[i + 3]);
            c.block = std::stoi(tokens[i + 4]);
        } catch (...) {
            return false;
        }
        out.push_back(c);
    }

    return true;
}

}  // namespace

bool SaveManager::saveGame(const std::string& path, const GameState& state) {
    std::ofstream out(path);
    if (!out) {
        return false;
    }

    out << "PLAYER " << state.player.maxHp << ' ' << state.player.str << ' '
        << state.player.def << ' ' << state.player.gold << '\n';

    out << "POS " << state.pos.layer << ' ' << state.pos.x << ' '
        << state.pos.y << '\n';

    out << "FLOOR " << state.currentFloor << '\n';

    out << "DECK";
    for (const Card& c : state.deck) {
        out << ' ' << c.id << ' ' << c.name << ' ' << c.cost << ' '
            << c.damage << ' ' << c.block;
    }
    out << '\n';

    out << "HAND";
    for (const Card& c : state.hand) {
        out << ' ' << c.id << ' ' << c.name << ' ' << c.cost << ' '
            << c.damage << ' ' << c.block;
    }
    out << '\n';

    out << "INVENTORY";
    for (const std::string& item : state.inventory) {
        out << ' ' << item;
    }
    out << '\n';

    return out.good();
}

bool SaveManager::loadGame(const std::string& path, GameState& outState) {
    std::ifstream in(path);
    if (!in) {
        return false;
    }

    bool hasPlayer = false;
    bool hasPos = false;
    bool hasFloor = false;
    bool hasDeck = false;
    bool hasHand = false;
    bool hasInventory = false;

    std::string line;
    while (std::getline(in, line)) {
        std::istringstream iss(line);
        std::string key;
        if (!(iss >> key)) {
            continue;  // skip blank lines
        }

        if (key == "PLAYER") {
            if (!(iss >> outState.player.maxHp >> outState.player.str
                      >> outState.player.def >> outState.player.gold)) {
                return false;
            }
            hasPlayer = true;
        } else if (key == "POS") {
            if (!(iss >> outState.pos.layer >> outState.pos.x
                      >> outState.pos.y)) {
                return false;
            }
            hasPos = true;
        } else if (key == "FLOOR") {
            if (!(iss >> outState.currentFloor)) {
                return false;
            }
            hasFloor = true;
        } else if (key == "DECK") {
            outState.deck.clear();
            if (!parseCards(iss, outState.deck)) {
                return false;
            }
            hasDeck = true;
        } else if (key == "HAND") {
            outState.hand.clear();
            if (!parseCards(iss, outState.hand)) {
                return false;
            }
            hasHand = true;
        } else if (key == "INVENTORY") {
            outState.inventory.clear();
            std::string item;
            while (iss >> item) {
                outState.inventory.push_back(item);
            }
            hasInventory = true;
        }
        // Unknown keys are ignored.
    }

    return hasPlayer && hasPos && hasFloor && hasDeck && hasHand && hasInventory;
}
