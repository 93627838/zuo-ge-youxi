#pragma once

#include <string>
#include <vector>

enum class Direction {
    NORTH,
    EAST,
    SOUTH,
    WEST
};

enum class CardType {
    ATTACK,
    DEFEND,
    BUFF,
    DRAW
};

struct Position {
    int layer;
    int x;
    int y;
};

struct Card {
    int id;
    std::string name;
    int cost;
    int damage;
    int block;
};

struct PlayerBase {
    int maxHp;
    int str;
    int def;
    int gold;
};

struct EnemyBase {
    int hp;
    int maxHp;
    int str;
    int def;
};

struct GameState {
    PlayerBase player;
    std::vector<Card> deck;
    std::vector<Card> hand;
    Position pos;
    int currentFloor;
    std::vector<std::string> inventory;
};
