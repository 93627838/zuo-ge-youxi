#include "Item.h"
#include "Player.h"
#include "UI.h"
#include "Data.h"     // g_cards(预知之滴要显示抽牌堆里的牌名)
#include "Combat.h"   // 精炼混沌要调 Combat::PlayTopDrawCard
#include <algorithm>  // std::min

/* ============================ 遗物 ============================ */

SnakeRing::SnakeRing() {
    name = "蛇之戒指";
    desc = "每场战斗开始时,额外抽取 " + std::to_string(drawN) + " 张牌";
}

void SnakeRing::OnBattleStart(Player& p) {
    p.deck.Draw(drawN);
    UI::Print("遗物【" + name + "】生效,额外抽 " + std::to_string(drawN) + " 张");
}

AegisShield::AegisShield() {
    name = "埃癸斯神盾";
    desc = "每场战斗开始时,获得 " + std::to_string(gain) + " 点格挡";
}

void AegisShield::OnBattleStart(Player& p) {
    p.block += gain;
    UI::Print("遗物【" + name + "】生效,获得 " + std::to_string(gain)
        + " 点格挡 (当前 " + std::to_string(p.block) + ")");
}

RhythmRemnant::RhythmRemnant() {
    name = "律动残余";
    desc = "你在一回合内失去的生命值不会超过 " + std::to_string(cap) + " 点";
}

void RhythmRemnant::OnTurnStart(Player& p) {
    lostThisTurn = 0;   // 每个玩家回合开始时,本回合的伤害计数归零
}

int RhythmRemnant::BeforeHPLoss(Player& p, int loss) {
    int allowed = cap - lostThisTurn;
    if (allowed < 0) allowed = 0;
    int reduced = std::min(loss, allowed);
    lostThisTurn += reduced;
    if (reduced < loss)
        UI::Print("遗物【" + name + "】压制伤害:本次仅损失 " + std::to_string(reduced)
            + " 点 (本回合累计损失 " + std::to_string(lostThisTurn) + ")");
    return reduced;
}

BloodVial::BloodVial() {
    name = "小血瓶";
    desc = "每场战斗开始时,回复 " + std::to_string(healN) + " 点生命";
}

void BloodVial::OnBattleStart(Player& p) {
    p.hp = std::min(p.maxHp, p.hp + healN);
    UI::Print("遗物【" + name + "】生效,回复 " + std::to_string(healN)
        + " 点生命 (当前 HP " + std::to_string(p.hp) + "/" + std::to_string(p.maxHp) + ")");
}

CentennialPuzzle::CentennialPuzzle() {
    name = "百年积木";
    desc = "每场战斗第一次损失生命值时,抽 " + std::to_string(drawN) + " 张牌";
}

void CentennialPuzzle::OnBattleStart(Player& p) {
    used = false;   // 每场战斗重置
}

void CentennialPuzzle::AfterHPLoss(Player& p, int loss) {
    if (used) return;
    used = true;
    p.deck.Draw(drawN);
    UI::Print("遗物【" + name + "】触发:第一次受伤,抽 " + std::to_string(drawN)
        + " 张牌 (手牌 " + std::to_string(p.deck.hand.size()) + " 张)");
}

/* ============ 后加的 6 个遗物 ============ */

BurningBlood::BurningBlood() {
    name = "燃烧之血";
    desc = "战斗胜利结束时,回复 " + std::to_string(healN) + " 点生命";
}

void BurningBlood::OnCombatEnd(Player& p) {
    int before = p.hp;
    p.hp = std::min(p.maxHp, p.hp + healN);
    UI::Print("遗物【" + name + "】生效,战斗结束回复 " + std::to_string(healN)
        + " 点生命 (" + std::to_string(before) + " → " + std::to_string(p.hp) + ")");
}

BattleContinuation::BattleContinuation() {
    name = "战斗续行";
    desc = "你没有用完的能量可以保留到下一回合";
}

bool BattleContinuation::CarryEnergy() {
    return true;
}

OrnHorn::OrnHorn() {
    name = "奥恩之角";
    desc = "每当有敌人死亡时,获得 " + std::to_string(gainEnergy)
        + " 点能量并抽 " + std::to_string(drawN) + " 张牌";
}

void OrnHorn::OnEnemyDeath(Player& p, Combat* c) {
    // 敌人死亡时触发(群怪里死一只就触发一次)
    p.deck.Draw(drawN);
    if (c) c->GainEnergy(gainEnergy);
    UI::Print("遗物【" + name + "】触发:敌人死亡,获得 " + std::to_string(gainEnergy)
        + " 点能量并抽取 " + std::to_string(drawN) + " 张牌");
}

Sundial::Sundial() {
    name = "日晷";
    desc = "每 " + std::to_string(needShuffles) + " 次洗牌,获得 " + std::to_string(gainEnergy) + " 点能量";
}

void Sundial::OnBattleStart(Player& p) {
    shuffles = 0;   // 每场战斗的洗牌计数清零
}

void Sundial::OnShuffle(Player& p, Combat* c) {
    if (++shuffles >= needShuffles) {
        shuffles = 0;
        UI::Print("遗物【" + name + "】触发:" + std::to_string(needShuffles) + " 次洗牌达成!");
        if (c) c->GainEnergy(gainEnergy);
    }
}

Weaver::Weaver() {
    name = "重织";
    desc = "每次将抽牌堆洗牌时,获得 " + std::to_string(blockGain) + " 点格挡";
}

void Weaver::OnShuffle(Player& p, Combat* c) {
    p.block += blockGain;
    UI::Print("遗物【" + name + "】触发:抽牌堆重新洗牌,获得 " + std::to_string(blockGain)
        + " 点格挡 (当前 " + std::to_string(p.block) + ")");
}

HolyVial::HolyVial() {
    name = "露滴圣杯瓶";
    desc = "每回合开始时获得 " + std::to_string(bonus) + " 点能量(效果可叠加)";
}

int HolyVial::EnergyAtTurnStart(Player& p) {
    UI::Print("遗物【" + name + "】生效:本回合能量 +" + std::to_string(bonus));
    return bonus;
}

/* ============================ 药水 ============================ */

BottledFairy::BottledFairy() {
    name = "瓶装精灵";
    desc = "生命值将被减少至 0 或以下时,丢弃本药水,回复至最大生命值的 "
        + std::to_string(pct) + "%";
}

bool BottledFairy::OnLethalHit(Player& p) {
    int heal = p.maxHp * pct / 100;
    p.hp = heal;
    UI::Print("【" + name + "】碎裂,你免于一死,回复到 " + std::to_string(heal) + " HP!");
    return true;
}

DrawPotion::DrawPotion() {
    name = "抽牌药水";
    desc = "使用后,本回合抽取 3 张牌";
    usable = true;
}

bool DrawPotion::Use(Player& p, Combat* c) {
    p.deck.Draw(3);
    UI::Print("你喝下【" + name + "】,抽取 3 张牌 (手牌 " + std::to_string(p.deck.hand.size()) + " 张)");
    return true;
}

BloodPotion::BloodPotion() {
    name = "鲜血药水";
    desc = "回复你最大生命值的 " + std::to_string(pct) + "%";
    usable = true;
}

bool BloodPotion::Use(Player& p, Combat* c) {
    int h = p.maxHp * pct / 100;
    if (h <= 0) return false;
    p.hp = std::min(p.maxHp, p.hp + h);
    UI::Print("你喝下【" + name + "】,回复 " + std::to_string(h)
        + " 点生命 (当前 HP " + std::to_string(p.hp) + "/" + std::to_string(p.maxHp) + ")");
    return true;
}

ChaosPotion::ChaosPotion() {
    name = "精炼混沌";
    desc = "打出你抽牌堆顶部的 " + std::to_string(playN) + " 张牌";
    usable = true;
}

bool ChaosPotion::Use(Player& p, Combat* c) {
    if (!c) return false;                 // 只在战斗里能用
    if (p.deck.draw.empty()) {
        UI::Print("抽牌堆是空的,没有牌可打出。");
        return false;
    }
    for (int i = 0; i < playN && !p.deck.draw.empty(); ++i)
        c->PlayTopDrawCard();
    return true;
}

SolidifyPotion::SolidifyPotion() {
    name = "固化药水";
    desc = "将你的格挡变为 " + std::to_string(mult) + " 倍";
    usable = true;
}

bool SolidifyPotion::Use(Player& p, Combat* c) {
    if (p.block <= 0) {
        UI::Print("你没有格挡,固化药水没有效果。");
        return false;
    }
    p.block *= mult;
    UI::Print("你喝下【" + name + "】,格挡变为 " + std::to_string(mult)
        + " 倍 (当前 " + std::to_string(p.block) + ")");
    return true;
}

ForesightPotion::ForesightPotion() {
    name = "预知之滴";
    desc = "选择你抽牌堆中的一张牌加入你的手牌";
    usable = true;
}

bool ForesightPotion::Use(Player& p, Combat* c) {
    std::vector<int>& draw = p.deck.draw;
    if (draw.empty()) {
        UI::Print("抽牌堆是空的,没有牌可选。");
        return false;
    }
    UI::Clear();
    UI::Print("========== 抽牌堆(顶在最后) ==========");
    for (size_t i = 0; i < draw.size(); ++i) {
        const Card& cd = g_cards[draw[i]];
        UI::Print(std::to_string(i + 1) + ". 【" + cd.name + "】 " + cd.desc);
    }
    UI::Print("选择一张加入手牌 (0 取消):");
    int sel = UI::GetInt();
    if (sel < 1 || sel >(int)draw.size()) {
        UI::Print("取消了。");
        return false;
    }
    int idx = draw[sel - 1];
    draw.erase(draw.begin() + sel - 1);
    p.deck.hand.push_back(idx);
    UI::Print("【" + g_cards[idx].name + "】加入了你的手牌。");
    return true;
}

GigantifyPotion::GigantifyPotion() {
    name = "超巨化药水";
    desc = "你打出的下一张攻击牌将会造成 " + std::to_string(mult) + " 倍伤害";
    usable = true;
}

bool GigantifyPotion::Use(Player& p, Combat* c) {
    ++p.nextAtkMult;
    UI::Print("你喝下【" + name + "】,下一张攻击牌伤害 ×" + std::to_string(mult)
        + " (剩余 " + std::to_string(p.nextAtkMult) + " 次)");
    return true;
}

StrengthPotion::StrengthPotion() {
    name = "力量药水";
    desc = "获得 " + std::to_string(gain) + " 点力量";
    usable = true;
}

bool StrengthPotion::Use(Player& p, Combat* c) {
    p.strength += gain;
    UI::Print("你喝下【" + name + "】,获得 " + std::to_string(gain)
        + " 点力量 (当前 " + std::to_string(p.strength) + ")");
    return true;
}

/* ============================ 商店道具目录 ============================ */

// 以后新增遗物/药水:写好子类后,在这里加一行即可自动进店。
// name/desc 要和对应构造函数里写的保持一致(商店列表用这里的,背包用构造函数的)。
const std::vector<ItemDef> g_itemCatalog = {
    { ItemType::SNAKE_RING,         true,  "蛇之戒指",     "每场战斗开始时,额外抽取 2 张牌",                      150 },
    { ItemType::AEGIS_SHIELD,       true,  "埃癸斯神盾",   "每场战斗开始时,获得 10 点格挡",                         120 },
    { ItemType::RHYTHM_REMNANT,     true,  "律动残余",     "你在一回合内失去的生命值不会超过 20 点",                 180 },
    { ItemType::BLOOD_VIAL,         true,  "小血瓶",       "每场战斗开始时,回复 2 点生命",                            60 },
    { ItemType::CENTENNIAL_PUZZLE,  true,  "百年积木",     "每场战斗第一次损失生命值时,抽 3 张牌",                  110 },
    { ItemType::BURNING_BLOOD,      true,  "燃烧之血",     "战斗胜利结束时,回复 6 点生命",                           160 },
    { ItemType::BATTLE_CONTINUATION,true,  "战斗续行",     "你没有用完的能量可以保留到下一回合",                       150 },
    { ItemType::ORN_HORN,           true,  "奥恩之角",     "每当有敌人死亡时,获得 1 点能量并抽 1 张牌",               130 },
    { ItemType::SUNDIAL,            true,  "日晷",         "每 3 次洗牌,获得 2 点能量",                               140 },
    { ItemType::WEAVER,             true,  "重织",         "每次将抽牌堆洗牌时,获得 6 点格挡",                        120 },
    { ItemType::HOLY_VIAL,          true,  "露滴圣杯瓶",   "每回合开始时获得 1 点能量(效果可叠加)",                   250 },

    { ItemType::BOTTLED_FAIRY,      false, "瓶装精灵",     "生命值将被减少至 0 或以下时,丢弃本药水,回复至最大生命值的 30%", 90 },
    { ItemType::DRAW_POTION,        false, "抽牌药水",     "使用后,本回合抽取 3 张牌",                               45 },
    { ItemType::BLOOD_POTION,       false, "鲜血药水",     "回复你最大生命值的 20%",                                 70 },
    { ItemType::CHAOS_POTION,       false, "精炼混沌",     "打出你抽牌堆顶部的 3 张牌",                             120 },
    { ItemType::SOLIDIFY_POTION,    false, "固化药水",     "将你的格挡变为 3 倍",                                    75 },
    { ItemType::FORESIGHT_POTION,   false, "预知之滴",     "选择你抽牌堆中的一张牌加入你的手牌",                     80 },
    { ItemType::GIGANTIFY_POTION,   false, "超巨化药水",   "你打出的下一张攻击牌将会造成 3 倍伤害",                   95 },
    { ItemType::STRENGTH_POTION,    false, "力量药水",     "获得 2 点力量",                                          75 },
};

std::unique_ptr<Item> makeItem(ItemType t) {
    switch (t) {
    case ItemType::SNAKE_RING:         return std::make_unique<SnakeRing>();
    case ItemType::AEGIS_SHIELD:       return std::make_unique<AegisShield>();
    case ItemType::RHYTHM_REMNANT:     return std::make_unique<RhythmRemnant>();
    case ItemType::BLOOD_VIAL:         return std::make_unique<BloodVial>();
    case ItemType::CENTENNIAL_PUZZLE:  return std::make_unique<CentennialPuzzle>();
    case ItemType::BOTTLED_FAIRY:      return std::make_unique<BottledFairy>();
    case ItemType::DRAW_POTION:        return std::make_unique<DrawPotion>();
    case ItemType::BLOOD_POTION:       return std::make_unique<BloodPotion>();
    case ItemType::CHAOS_POTION:       return std::make_unique<ChaosPotion>();
    case ItemType::SOLIDIFY_POTION:    return std::make_unique<SolidifyPotion>();
    case ItemType::FORESIGHT_POTION:   return std::make_unique<ForesightPotion>();
    case ItemType::GIGANTIFY_POTION:   return std::make_unique<GigantifyPotion>();
    case ItemType::STRENGTH_POTION:    return std::make_unique<StrengthPotion>();
    case ItemType::BURNING_BLOOD:      return std::make_unique<BurningBlood>();
    case ItemType::BATTLE_CONTINUATION:return std::make_unique<BattleContinuation>();
    case ItemType::ORN_HORN:           return std::make_unique<OrnHorn>();
    case ItemType::SUNDIAL:            return std::make_unique<Sundial>();
    case ItemType::WEAVER:             return std::make_unique<Weaver>();
    case ItemType::HOLY_VIAL:          return std::make_unique<HolyVial>();
    }
    return nullptr;
}
