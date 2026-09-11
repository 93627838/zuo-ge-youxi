#pragma once
#include "Player.h"
#include "Enemy.h"
#include "Card.h"
#include <vector>
#include <string>

// 战斗引擎:回合时序、能量、意图执行、伤害结算(支持 1 对多的"群怪")
class Combat {
public:
    Combat(Player& player, std::vector<Enemy> enemies);
    ~Combat();                          // 战斗结束:摘掉洗牌回调,避免指向已销毁的 Combat
    bool Run();  // true=玩家胜利, false=玩家被击败
    void PlayTopDrawCard();   // 打出抽牌堆顶的一张(0 费,给"精炼混沌"药水用)
    void GainEnergy(int n);   // 遗物往当前回合能量里加(日晷每 3 次洗牌 +2)

private:
    void PlayerTurn();
    void EnemyTurn();
    void PlayCard(int handIdx);
    void ApplyEffect(CardEffect ef, int x);
    void ShowState();
    void BattleStartRelics();    // 战斗开始(仅第 1 回合)触发遗物
    void UsePotionMenu();        // 打开药水背包(主动使用)
    void ShowRelicInfo();//查看遗物信息
    void ShowDeckView(); //查看牌组构成 + 本回合每张手牌的伤害预览
    void HurtPlayer(int rawDmg, const std::string& attackerName); // 结算对玩家的伤害(格挡→濒死药水→扣血)

    int AliveCount() const;                    // 存活敌人数量
    int AliveIndex(int k) const;               // 第 k 只(0 基)存活敌人在 es 里的下标
    bool CardHitsEnemy(const Card& c) const;   // 这张牌是否需要对敌人指定目标
    void ChooseTarget(const std::string& cardName);   // 群怪:选这一张牌打哪只敌人
    Player& p;                      // 玩家引用
    std::vector<Enemy> es;          // 当前战斗的敌人列表(支持 1 对多群怪)
    int cur = 0;                    // 当前这张牌打向 es 里的哪只(-1=无目标)
    int energy = 0;                 // 本回合能量
    int maxEnergy = 3;              // 本回合能量上限(基础 3 + 遗物加成,如露滴圣杯瓶)
    int carryEnergy = 0;            // 战斗续行:上回合留下来的能量
    bool firstTurn = true;          // 是否战斗第 1 回合(用来触发开局遗物)
    bool cardTriple = false;        // 本张攻击牌是否被超巨化 ×3
    bool enemyHitPlayer = false;    // 本敌人回合是否真打到了玩家
    void GainApoth(int amt);        // 攒登神共鸣进度(打攻击牌/被真打到 +10)
};