#pragma once
#include <string>
#include <memory>
#include <vector>

struct Player;   // 前置声明:事件上下文用 Player&,避免和 Player.h 互相包含
class Combat;    // 前置声明:个别药水(精炼混沌)需要战斗引擎来打出抽牌堆的牌

// 所有道具(遗物/药水)统一继承这一个基类:
// 主动效果 override Use,被动效果 override 事件钩子,互不干扰
class Item {
public:
    std::string name;
    std::string desc;
    bool usable = false;   // true = 出现在玩家回合的"使用药水"列表里

    virtual ~Item() = default;

    // ---- 主动使用:玩家在自己的回合选中它时调用 ----
    // c:当前战斗引擎。多数药水用不到(可传 nullptr);"精炼混沌"要靠它打出抽牌堆顶的牌
    virtual bool Use(Player& p, Combat* c) { return false; }   // true = 使用成功,应消耗

    // ---- 被动触发:事件自动调用,用不到就不 override ----
    virtual void OnBattleStart(Player& p) {}        // 战斗开始(仅第 1 回合)
    virtual void OnTurnStart(Player& p) {}          // 每个玩家回合开始
    virtual bool OnLethalHit(Player& p) { return false; }   // 濒死:true=已救下
    // 扣血前调用:可削减本次损失,返回值才是真正会扣的数值(律动残余:每回合≤cap)
    virtual int  BeforeHPLoss(Player& p, int loss) { return loss; }
    // 扣血后调用:loss 为本次实际损失的生命(百年积木:首次受伤抽牌)
    virtual void AfterHPLoss(Player& p, int loss) {}

    // ---- 下面 5 个是后加遗物用到的被动事件(用不到就不 override) ----
    virtual void OnCombatEnd(Player& p) {}                       // 战斗胜利结束时(燃烧之血回血)
    virtual void OnEnemyDeath(Player& p, Combat* c) {}           // 有敌人死亡时(奥恩之角;多敌人战斗才有实义)
    virtual int  EnergyAtTurnStart(Player& p) { return 0; }      // 每个玩家回合开始的能量加成(露滴圣杯瓶每份 +1)
    virtual bool CarryEnergy() { return false; }                 // true = 回合结束多余能量留到下回合(战斗续行)
    virtual void OnShuffle(Player& p, Combat* c) {}              // 抽牌堆每洗一次(日晷计数 / 重织加格挡)
};


//遗物
// 蛇之戒指:遗物,战斗开始抽牌
class SnakeRing : public Item {
public:
    int drawN = 2;
    SnakeRing();
    void OnBattleStart(Player& p) override;
};

// 埃癸斯神盾:遗物,战斗开始获得格挡
class AegisShield : public Item {
public:
    int gain = 10;
    AegisShield();
    void OnBattleStart(Player& p) override;
};

// 律动残余:遗物,每回合损失的生命不超过 cap 点
class RhythmRemnant : public Item {
public:
    int cap = 20;
    int lostThisTurn = 0;             // 本回合已损失的生命
    RhythmRemnant();
    void OnTurnStart(Player& p) override;
    int BeforeHPLoss(Player& p, int loss) override;
};

// 小血瓶:遗物,战斗开始回血
class BloodVial : public Item {
public:
    int healN = 2;
    BloodVial();
    void OnBattleStart(Player& p) override;
};

// 百年积木:遗物,每场战斗第一次受伤时抽牌
class CentennialPuzzle : public Item {
public:
    int drawN = 3;
    bool used = false;
    CentennialPuzzle();
    void OnBattleStart(Player& p) override;
    void AfterHPLoss(Player& p, int loss) override;
};

// 燃烧之血:遗物,战斗胜利结束时回复 6 点生命
class BurningBlood : public Item {
public:
    int healN = 6;
    BurningBlood();
    void OnCombatEnd(Player& p) override;
};

// 战斗续行:遗物,多余能量保留到下一回合
class BattleContinuation : public Item {
public:
    BattleContinuation();
    bool CarryEnergy() override;
};

// 奥恩之角:遗物,敌人死亡时获得 1 点能量并抽 1 张牌
class OrnHorn : public Item {
public:
    int drawN = 1;
    int gainEnergy = 1;
    OrnHorn();
    void OnEnemyDeath(Player& p, Combat* c) override;
};

// 日晷:遗物,每洗牌 3 次获得 2 点能量
class Sundial : public Item {
public:
    int needShuffles = 3;
    int gainEnergy = 2;
    int shuffles = 0;        // 本场战斗已洗牌次数(战斗开始清零)
    Sundial();
    void OnBattleStart(Player& p) override;
    void OnShuffle(Player& p, Combat* c) override;
};

// 重织:遗物,抽牌堆每次洗牌时获得 6 点格挡
class Weaver : public Item {
public:
    int blockGain = 6;
    Weaver();
    void OnShuffle(Player& p, Combat* c) override;
};

// 露滴圣杯瓶:遗物,每回合开始时获得 1 点能量(效果可叠加)
class HolyVial : public Item {
public:
    int bonus = 1;
    HolyVial();
    int EnergyAtTurnStart(Player& p) override;
};

//药水
// 瓶装精灵:药水,濒死自动触发
class BottledFairy : public Item {
public:
    int pct = 30;
    BottledFairy();
    bool OnLethalHit(Player& p) override;
};

// 迅捷药水:药水,主动使用抽 3 张
class DrawPotion : public Item {
public:
    DrawPotion();
    bool Use(Player& p, Combat* c) override;
};

// 鲜血药水:主动,回复最大生命的 20%
class BloodPotion : public Item {
public:
    int pct = 20;
    BloodPotion();
    bool Use(Player& p, Combat* c) override;
};

// 精炼混沌:主动,打出抽牌堆顶的 3 张牌(需要战斗引擎)
class ChaosPotion : public Item {
public:
    int playN = 3;
    ChaosPotion();
    bool Use(Player& p, Combat* c) override;
};

// 固化药水:主动,格挡变三倍
class SolidifyPotion : public Item {
public:
    int mult = 3;
    SolidifyPotion();
    bool Use(Player& p, Combat* c) override;
};

// 预知之滴:主动,从抽牌堆选一张加入手牌
class ForesightPotion : public Item {
public:
    ForesightPotion();
    bool Use(Player& p, Combat* c) override;
};

// 超巨化药水:主动,下一张攻击牌伤害三倍
class GigantifyPotion : public Item {
public:
    int mult = 3;
    GigantifyPotion();
    bool Use(Player& p, Combat* c) override;
};

// 力量药水:主动,获得力量
class StrengthPotion : public Item {
public:
    int gain = 2;
    StrengthPotion();
    bool Use(Player& p, Combat* c) override;
};

// ===== 商店道具目录:让遗物/药水能被列出、随机、购买 =====
enum class ItemType {
    SNAKE_RING,
    BOTTLED_FAIRY,
    DRAW_POTION,
    AEGIS_SHIELD,
    RHYTHM_REMNANT,
    BLOOD_VIAL,
    CENTENNIAL_PUZZLE,
    BLOOD_POTION,
    CHAOS_POTION,
    SOLIDIFY_POTION,
    FORESIGHT_POTION,
    GIGANTIFY_POTION,
    STRENGTH_POTION,
    BURNING_BLOOD,
    BATTLE_CONTINUATION,
    ORN_HORN,
    SUNDIAL,
    WEAVER,
    HOLY_VIAL,
};

struct ItemDef {
    ItemType    type = ItemType::SNAKE_RING;  // 买下时造哪种
    bool        isRelic = false;              // true=遗物; false=药水
    std::string name;                          // 商店显示的名字(和构造函数里写的保持一致)
    std::string desc;                          // 商店显示的描述(同上)
    int         price = 0;                     // 售价
};

extern const std::vector<ItemDef> g_itemCatalog;  // 目录本体在 Item.cpp
std::unique_ptr<Item> makeItem(ItemType t);       // 买下时造出一份新实例
