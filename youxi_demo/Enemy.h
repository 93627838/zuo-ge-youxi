#pragma once
#include <string>
#include <vector>

// 敌人意图类型
enum class IntentType {
    ATTACK,            // 造成伤害(自动加上敌人力量)
    MULTI_ATTACK,      // 多段攻击:打 hits 次,每次 value(也加力量)
    BLOCK,             // 自己获得格挡
    BUFF_STRENGTH,     // 自己 +value 力量
    APPLY_WEAK,        // 使你虚弱 value 回合:你的攻击 -25%
    APPLY_VULNERABLE   // 使你易伤 value 回合:你受到的伤害 +50%
};

// 一个意图:本回合敌人要做什么
struct Intent {
    IntentType type;
    int value;
    int hits = 1;     // 多段攻击的段数,其它意图保持 1
};

// 敌人:任意步数出招循环,每回合按 pattern[step % 长度] 行动
struct Enemy {
    std::string name;
    int maxHp = 0;
    int hp = 0;
    int block = 0;
    int strength = 0;                 // 敌人力量,攻击时自动加上
    int weak = 0;                   // 虚弱:它造成的攻击 -25%
    int vulnerable = 0;             // 易伤:你打它的伤害 +50%
    std::vector<Intent> pattern;      // 出招表,想写几步写几步
    int step = 0;
};
