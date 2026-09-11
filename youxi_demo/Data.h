#pragma once
#include <vector>
#include <string>
#include "Card.h"
#include "Enemy.h"

// 卡牌表 / 敌人表 / 出场组合(数据驱动的入口,改这里就是改游戏内容)
extern const std::vector<Card>  g_cards;
extern const std::vector<Enemy> g_enemies;

// 一次战斗的出场名单:name 是选择菜单里显示的名字,enemy 是 g_enemies 的下标(可重复=同种怪上多只)
struct Encounter {
    std::string name;
    std::vector<int> enemy;
};
extern const std::vector<Encounter> g_encounters;