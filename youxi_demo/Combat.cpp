#include "Combat.h"
#include "Data.h"
#include "DeckView.h"
#include "UI.h"
#include <algorithm>
#include <string>
#include <cstdlib>
#include <utility>

Combat::Combat(Player& player, std::vector<Enemy> enemies) : p(player), es(std::move(enemies)) {
    p.block = 0;
    for (auto& en : es) {              // 群怪:每只敌人血量/格挡/出招步数单独重置
        en.hp = en.maxHp;
        en.block = 0;
        en.step = 0;
    }
    p.deck.Reset(p.startDeck);   // 开局洗牌发生在挂回调之前 → 不算进日晷/重织
    p.deck.onShuffle = [this] {  // 之后战斗中的每次洗牌都通知遗物
        for (auto& r : p.relics) r->OnShuffle(p, this);
        };
}

Combat::~Combat() {
    p.deck.onShuffle = nullptr;   // 摘掉回调,防止下一场战斗 Reset 时误调到已销毁的 Combat
}

void Combat::GainEnergy(int n) {
    energy += n;
    UI::Print("能量 +" + std::to_string(n) + " (剩余 " + std::to_string(energy) + ")");
}

int Combat::AliveCount() const {
    int c = 0;
    for (const auto& en : es) if (en.hp > 0) ++c;
    return c;
}

int Combat::AliveIndex(int k) const {
    for (size_t i = 0; i < es.size(); ++i)
        if (es[i].hp > 0) { if (k == 0) return (int)i; --k; }
    return -1;
}

bool Combat::CardHitsEnemy(const Card& c) const {
    auto hits = [](CardEffect ef) {
        return ef == CardEffect::DEAL_DMG || ef == CardEffect::APPLY_WEAK
            || ef == CardEffect::APPLY_VULNERABLE;
        };
    return hits(c.effect) || hits(c.effect2) || hits(c.effect3);
}

void Combat::ChooseTarget(const std::string& cardName) {
    if (AliveCount() <= 1) { cur = AliveIndex(0); return; }   // 只剩一只就不用问
    UI::Print("—— 为【" + cardName + "】选择目标 ——");
    int no = 1;
    for (size_t i = 0; i < es.size(); ++i) {
        if (es[i].hp <= 0) continue;
        UI::Print(std::to_string(no) + ". " + es[i].name
            + "   HP " + std::to_string(es[i].hp) + "/" + std::to_string(es[i].maxHp)
            + "   格挡 " + std::to_string(es[i].block));
        ++no;
    }
    UI::Print("请选择目标 (1~" + std::to_string(AliveCount()) + "):");
    int sel = UI::GetInt();
    if (sel < 1) sel = 1;
    if (sel > AliveCount()) sel = AliveCount();
    cur = AliveIndex(sel - 1);
}

bool Combat::Run() {
    while (true) {
        PlayerTurn();
        if (AliveCount() == 0) {   // 敌人全部倒下 -> 胜利
            for (auto& r : p.relics) r->OnCombatEnd(p);   // 燃烧之血
            return true;
        }
        if (p.hp <= 0) return false;  // 玩家死了 -> 失败

        EnemyTurn();
        if (p.hp <= 0) return false;
    }
}

void Combat::PlayerTurn() {
    energy = 3;
    for (auto& r : p.relics) energy += r->EnergyAtTurnStart(p);  // 露滴圣杯瓶:每份 +1(可叠)
    energy += carryEnergy; carryEnergy = 0;   // 战斗续行:把上回合留的能量加上
    p.block = 0;              // 格挡在你的回合开始时清空
    for (auto& r : p.relics) r->OnTurnStart(p);   // 回合开始遗物(律动残余归零等)
    p.deck.Draw(5);

    while (true) {
        ShowState();
        if (firstTurn) {      // 战斗开始(仅第 1 回合):先进战斗界面,再触发遗物
            firstTurn = false;
            BattleStartRelics();
            continue;         // 遗物抽完牌后重新展示一次界面
        }
        UI::Print("输入手牌编号出牌 / 0 结束回合 / P 使用药水 / R 查看遗物 / D 查看牌组:");
        std::string line = UI::GetLine();
        if (line == "0") break;
        if (line == "P" || line == "p") {   // 打开药水背包
            UsePotionMenu();
            UI::Pause();
            continue;
        }
        if (line == "R" || line == "r") {   // 查看遗物说明
            ShowRelicInfo();
            continue;
        }
        if (line == "D" || line == "d") {   // 查看牌组构成 + 伤害预览
            ShowDeckView();
            continue;
        }
        int sel = std::atoi(line.c_str());
        if (sel < 1 || sel > static_cast<int>(p.deck.hand.size())) {
            UI::Print("无效输入,请重新选择。");
            continue;
        }
        PlayCard(sel - 1);
        if (AliveCount() == 0) break;  // 全灭才跳出回合
        UI::Pause();
    }
    carryEnergy = 0;                           // 默认不把能量留到下一回合
    for (auto& r : p.relics)                   // 战斗续行:没花完的能量留到下回合
        if (r->CarryEnergy() && energy > 0) {
            carryEnergy = energy;
            UI::Print("遗物【战斗续行】保留 " + std::to_string(energy) + " 点能量至下回合");
        }
    p.deck.DiscardHand();
    if (p.weak > 0) --p.weak;                  // 你身上的虚弱在你回合结束 -1
    for (auto& en : es)                        // 每只敌人的易伤在你回合结束 -1
        if (en.vulnerable > 0) --en.vulnerable;
}

void Combat::BattleStartRelics() {
    for (auto& r : p.relics)
        r->OnBattleStart(p);
    if (!p.relics.empty()) UI::Pause();   // 让遗物效果多停留一下
}

void Combat::UsePotionMenu() {
    // 收集可主动使用的药水(usable==true)
    std::vector<int> list;   // 存 p.potions 里的下标
    for (size_t i = 0; i < p.potions.size(); ++i)
        if (p.potions[i]->usable) list.push_back((int)i);

    if (list.empty()) {
        UI::Print("没有可主动使用的药水。");
        return;
    }
    UI::Clear();
    UI::Print("========== 药水背包 ==========");
    for (size_t k = 0; k < list.size(); ++k) {
        Item* it = p.potions[list[k]].get();
        UI::Print(std::to_string(k + 1) + ". 【" + it->name + "】 " + it->desc);
    }
    UI::Print("0. 返回");
    int sel = UI::GetInt();
    if (sel < 1 || sel > static_cast<int>(list.size())) return;

    Item* it = p.potions[list[sel - 1]].get();
    if (it->Use(p, this)) {
        p.potions.erase(p.potions.begin() + list[sel - 1]);   // 用掉即消耗
        UI::Print("(该药水已消耗)");
    }
}
void Combat::ShowRelicInfo() {
    if (p.relics.empty()) {
        UI::Print("你没有遗物。");
        return;
    }
    UI::Clear();
    UI::Print("========== 你的遗物 ==========");
    for (size_t i = 0; i < p.relics.size(); ++i) {
        const Item& it = *p.relics[i];
        UI::Print(std::to_string(i + 1) + ". 【" + it.name + "】");
        UI::Print("   " + it.desc);
    }
    UI::Pause();
}

void Combat::ShowDeckView() {
    // 战斗内的"整副牌" = 手牌 + 抽牌堆 + 弃牌堆
    std::vector<int> all = p.deck.hand;
    all.insert(all.end(), p.deck.draw.begin(), p.deck.draw.end());
    all.insert(all.end(), p.deck.discard.begin(), p.deck.discard.end());

    UI::Clear();
    DeckView::PrintGroupedCounts(all, "你的牌组");
    UI::Pause();
}

void Combat::PlayCard(int handIdx) {
    int cardIdx = p.deck.hand[handIdx];
    const Card& c = g_cards[cardIdx];

    bool isX = c.isX;                   // isX 标记:X 费卡
    int pay = isX ? energy : c.cost;    // X 费卡:花费全部能量

    if (isX) {
        if (pay <= 0) { UI::Print("没有能量,X 费卡无法使用!"); return; }
    }
    else {
        if (energy < pay) { UI::Print("能量不足! (需要 " + std::to_string(pay) + " 点)"); return; }
    }

    energy -= pay;
    bool triple = (c.type == CardType::ATTACK && p.nextAtkMult > 0);
    if (triple) { --p.nextAtkMult; cardTriple = true; }   // 超巨化药水:整张攻击牌×3
    UI::Print("你打出了【" + c.name + "】");
    if (triple) UI::Print("   超巨化效果:本张攻击牌伤害 ×3 !");
    // ===== 登神:攻击牌攒进度;非攻击牌解除形态 =====
    if (c.type == CardType::ATTACK) {
        GainApoth(10);                  // 出一张攻击牌 +10%
    }
    else if (p.apotheosis) {
        p.apotheosis = false;           // 打出非攻击牌(技能/防御/能力)解除形态
        p.apothGauge = 0;
        UI::Print("打出【" + c.name + "】,神座在你退守的一瞬崩塌。共鸣归零。");
    }
    if (CardHitsEnemy(c)) ChooseTarget(c.name);   // 要对敌人下手的牌先选目标(群怪用)
    if (isX) {
        // X 费卡:力量加到每一次攻击上,即打 X 次,每次 c.x + 力量
        for (int i = 0; i < pay && es[cur].hp > 0; ++i)   // 目标被这段打死,后面段数停手
            ApplyEffect(c.effect, c.x);
    }
    else {
        ApplyEffect(c.effect, c.x);
    }
    ApplyEffect(c.effect2, c.x2);
    ApplyEffect(c.effect3, c.x3);
    cardTriple = false;   // 本张牌的×3 效果到此为止

    // 打出后:手牌移除,进弃牌堆
    p.deck.hand.erase(p.deck.hand.begin() + handIdx);
    if (c.type != CardType::POWER)
        p.deck.discard.push_back(cardIdx);  // 能力卡直接消失,不进弃牌堆
}

void Combat::PlayTopDrawCard() {
    if (p.deck.draw.empty()) return;
    int cardIdx = p.deck.draw.back();      // 抽牌堆顶(最后一张)
    p.deck.draw.pop_back();
    const Card& c = g_cards[cardIdx];
    cur = AliveIndex(0);                    // 精炼混沌自动打向第一只存活敌人
    if (cur < 0) { p.deck.discard.push_back(cardIdx); return; }  // 没有敌人可打
    UI::Print("【精炼混沌】打出抽牌堆顶的【" + c.name + "】");
    if (!c.isX) {                          // X 费卡此时无能量,效果不结算,只算打出了
        ApplyEffect(c.effect, c.x);
        ApplyEffect(c.effect2, c.x2);
        ApplyEffect(c.effect3, c.x3);
    }
    if (c.type != CardType::POWER)
        p.deck.discard.push_back(cardIdx);  // 能力卡打出后消失,其余进弃牌堆
}

// 攒登神进度:打攻击牌 +10 / 实际受伤 +10;满 100 进入登神形态
void Combat::GainApoth(int amt) {
    if (p.apotheosis) return;                 // 形态中不重复累计
    p.apothGauge += amt;
    if (p.apothGauge >= 100) {
        p.apothGauge = 100;
        p.apotheosis = true;
        UI::Print("【登神】神骸共鸣达到临界——你短暂地触碰到了神座。攻击伤害 ×1.5!");
    }
    else {
        UI::Print("【登神】进度 " + std::to_string(p.apothGauge) + "%");
    }
}

void Combat::ApplyEffect(CardEffect ef, int x) {
    if (ef == CardEffect::NONE) return;

    if (ef == CardEffect::DEAL_DMG) {
        if (cur < 0 || (size_t)cur >= es.size() || es[cur].hp <= 0) return;  // 目标已死,打空
        Enemy& en = es[cur];
        int dmg = x + p.strength;                      // 力量加到攻击上
        if (cardTriple) dmg *= 3;                      // 超巨化药水:整张攻击牌×3
        bool crit = (std::rand() / (double)RAND_MAX) < p.chr;   // 暴击判定
        if (crit) dmg *= 2;                            // 暴击:伤害翻倍
        int num = 1, den = 1;
        if (p.weak > 0) { num *= 3; den *= 4; }   // 你虚弱:攻击 -25%
        if (p.apotheosis) { num *= 3; den *= 2; }   // 登神形态 ×1.5
        if (en.vulnerable > 0) { num *= 3; den *= 2; }   // 敌人易伤 ×1.5(与暴击/登神/超巨化连乘)
        dmg = dmg * num / den;
        int absorbed = std::min(en.block, dmg);        // 敌人格挡先吸收
        en.block -= absorbed;
        bool wasAlive = en.hp > 0;
        en.hp = std::max(0, en.hp - (dmg - absorbed));
        UI::Print("对 " + en.name + " 造成 " + std::to_string(dmg)
            + " 点伤害 " + (crit ? "(暴击!) " : " ")
            + "(格挡吸收 " + std::to_string(absorbed) + ")");
        if (wasAlive && en.hp <= 0)                    // 这一下正好把它打死
            for (auto& r : p.relics) r->OnEnemyDeath(p, this);   // 奥恩之角(群怪里一只只触发)
    }
    else if (ef == CardEffect::GAIN_BLOCK) {
        int gained = x + p.toughness;                  // 坚韧加成:每点 +1 格挡
        p.block += gained;
        UI::Print("获得 " + std::to_string(gained) + " 点格挡 (当前 " + std::to_string(p.block) + ")"
            + (p.toughness > 0 ? " (坚韧 +" + std::to_string(p.toughness) + ")" : ""));
    }
    else if (ef == CardEffect::GAIN_STRENGTH) {
        p.strength += x;
        UI::Print("获得 " + std::to_string(x) + " 点力量 (当前 " + std::to_string(p.strength) + ")");
    }
    else if (ef == CardEffect::GAIN_TOUGHNESS) {
        p.toughness += x;
        UI::Print("获得 " + std::to_string(x) + " 点坚韧 (当前 " + std::to_string(p.toughness) + ")");
    }
    else if (ef == CardEffect::GAIN_ENERGY) {
        energy += x;
        UI::Print("获得 " + std::to_string(x) + " 点能量 (剩余 " + std::to_string(energy) + ")");
    }
    else if (ef == CardEffect::DRAW_CARD) {
        p.deck.Draw(x);
        UI::Print("抽取 " + std::to_string(x) + " 张牌 (手牌 " + std::to_string(p.deck.hand.size()) + " 张)");
    }
    else if (ef == CardEffect::APPLY_WEAK) {
        if (cur < 0 || (size_t)cur >= es.size() || es[cur].hp <= 0) return;  // 目标已死
        Enemy& en = es[cur];
        en.weak = std::max(en.weak, x);            // 刷新不叠加
        UI::Print(en.name + " 陷入虚弱,其攻击 -25% (" + std::to_string(en.weak) + " 层)");
    }
    else if (ef == CardEffect::APPLY_VULNERABLE) {
        if (cur < 0 || (size_t)cur >= es.size() || es[cur].hp <= 0) return;  // 目标已死
        Enemy& en = es[cur];
        en.vulnerable = std::max(en.vulnerable, x);
        UI::Print(en.name + " 陷入易伤,你对其伤害 +50% (" + std::to_string(en.vulnerable) + " 层)");
    }
}

void Combat::EnemyTurn() {
    UI::Print("");
    UI::Print("========== 敌人回合 ==========");
    enemyHitPlayer = false;                    // 本回合是否真伤到你(多段只记一次)
    bool hadVul = (p.vulnerable > 0);          // 本回合开始时已易伤才在末尾 -1
    for (auto& en : es) en.block = 0;          // 敌人格挡在敌方阶段开始全部清空

    for (size_t i = 0; i < es.size(); ++i) {
        Enemy& en = es[i];
        if (en.hp <= 0) continue;              // 已死的跳过,不占出招名额

        const Intent& it = en.pattern[en.step % en.pattern.size()];
        auto effAtk = [&](int base) {          // 攻击力 = 基础 + 力量,虚弱时再 -25%
            int d = base + en.strength;
            if (en.weak > 0) d -= d / 4;
            return d;
            };

        switch (it.type) {
        case IntentType::ATTACK:
            HurtPlayer(effAtk(it.value), en.name);
            break;
        case IntentType::MULTI_ATTACK:
            UI::Print(en.name + " 连续攻击 " + std::to_string(it.hits) + " 次!");
            for (int k = 0; k < it.hits; ++k)
                HurtPlayer(effAtk(it.value), en.name);
            break;
        case IntentType::BLOCK:
            en.block += it.value;
            UI::Print(en.name + " 进入防御,获得 " + std::to_string(it.value) + " 点格挡");
            break;
        case IntentType::BUFF_STRENGTH:
            en.strength += it.value;
            UI::Print(en.name + " 积蓄力量,力量 +" + std::to_string(it.value)
                + " (当前 " + std::to_string(en.strength) + ")");
            break;
        case IntentType::APPLY_WEAK:
            p.weak = std::max(p.weak, it.value);   // 刷新,不叠加
            UI::Print(en.name + " 使你虚弱,你的攻击 -25% (" + std::to_string(p.weak) + " 回合)");
            break;
        case IntentType::APPLY_VULNERABLE:
            p.vulnerable = std::max(p.vulnerable, it.value);
            UI::Print(en.name + " 使你易伤,你受到的伤害 +50% (" + std::to_string(p.vulnerable) + " 回合)");
            break;
        }
        en.step++;                                  // 出完招,步数前进(下一轮换下一个意图)
        if (en.weak > 0) --en.weak;                 // 它行动完,自己的虚弱 -1
    }

    if (enemyHitPlayer) GainApoth(10);                 // 被真打到过 → 登神 +10%(多名敌人也只 +10)
    if (hadVul && p.vulnerable > 0) --p.vulnerable;   // 你易伤在敌人回合结束 -1(刚叠的不扣)
    UI::Print("你的 HP: " + std::to_string(p.hp) + "/" + std::to_string(p.maxHp));
    UI::Pause();
}

void Combat::HurtPlayer(int rawDmg, const std::string& attackerName) {
    if (p.vulnerable > 0) {                      // 易伤:受到的伤害 +50%
        rawDmg += rawDmg / 2;
        UI::Print("(你处于易伤状态,受到的伤害提升到 " + std::to_string(rawDmg) + ")");
    }
    int absorbed = std::min(p.block, rawDmg);  // 你的格挡先吸收
    p.block -= absorbed;
    int loss = rawDmg - absorbed;
    UI::Print(attackerName + " 对你造成 " + std::to_string(rawDmg)
        + " 点伤害 (格挡吸收 " + std::to_string(absorbed) + ")");

    for (auto& r : p.relics) loss = r->BeforeHPLoss(p, loss);  // 遗物可能削减损失(律动残余)
    if (loss <= 0) return;                                     // 完全被遗物挡下

    // 濒死药水:这一击会扣到 0 或以下时,逐个问药水,谁救下谁消耗
    if (p.hp - loss <= 0) {
        for (size_t i = 0; i < p.potions.size(); ++i) {
            if (p.potions[i]->OnLethalHit(p)) {
                p.potions.erase(p.potions.begin() + i);
                return;
            }
        }
    }
    enemyHitPlayer = true;             // 真的扣到血了(格挡挡住的不算),多段多次扣也只在回合末算一次
    p.hp = std::max(0, p.hp - loss);
    for (auto& r : p.relics) r->AfterHPLoss(p, loss);   // 受伤后遗物(百年积木)
}

void Combat::ShowState() {

    UI::Clear();
    UI::Print("========== 你的状态 ==========");
    UI::Print("HP " + std::to_string(p.hp) + "/" + std::to_string(p.maxHp)
        + "   格挡 " + std::to_string(p.block)
        + "   坚韧 " + std::to_string(p.toughness)
        + "   力量 " + std::to_string(p.strength)
        + "   能量 " + std::to_string(energy) + "/3");
    UI::Print("   神骸共鸣: " + (p.apotheosis ? std::string("形态中 (攻击 ×1.5)") : (std::to_string(p.apothGauge) +
        "%")));
    UI::Print("   抽牌堆 " + std::to_string(p.deck.draw.size())
        + "   弃牌堆 " + std::to_string(p.deck.discard.size()));
    if (p.weak > 0 || p.vulnerable > 0) {
        std::string st;
        if (p.weak > 0)       st += " 虚弱" + std::to_string(p.weak);
        if (p.vulnerable > 0) st += " 易伤" + std::to_string(p.vulnerable);
        UI::Print("   状态:" + st);
    }
    if (p.potions.empty()) {
        UI::Print("   药水: (空)");
    }
    else {
        std::string pots;
        for (size_t i = 0; i < p.potions.size(); ++i) {
            const Item& it = *p.potions[i];
            if (i > 0) pots += " | ";
            pots += it.name + (it.usable ? "" : "(自动)");
        }
        UI::Print("   药水: " + pots);
    }
    if (p.relics.empty()) {
        UI::Print("   遗物: (无)");
    }
    else {
        std::string rels;
        for (size_t i = 0; i < p.relics.size(); ++i) {
            const Item& it = *p.relics[i];
            if (i > 0) rels += " | ";
            rels += it.name;
        }
        UI::Print("   遗物: " + rels);
    }
    UI::Print("");

    UI::Print("========== 敌人 (存活 " + std::to_string(AliveCount()) + " 只) ==========");
    int no = 1;                                    // 场上显示的编号 = 选目标时用的编号
    for (size_t i = 0; i < es.size(); ++i) {
        Enemy& en = es[i];
        if (en.hp <= 0) continue;
        const Intent& it = en.pattern[en.step % en.pattern.size()];
        std::string intentDesc;
        switch (it.type) {
        case IntentType::ATTACK: {
            int a = it.value + en.strength;
            if (en.weak > 0) a -= a / 4;            // 预览也扣掉虚弱
            intentDesc = "攻击 " + std::to_string(a); break;
        }
        case IntentType::MULTI_ATTACK: {
            int a = it.value + en.strength;
            if (en.weak > 0) a -= a / 4;
            intentDesc = "多段攻击 " + std::to_string(it.hits) + "×" + std::to_string(a); break;
        }
        case IntentType::BLOCK:         intentDesc = "格挡 " + std::to_string(it.value); break;
        case IntentType::BUFF_STRENGTH: intentDesc = "力量 +" + std::to_string(it.value); break;
        case IntentType::APPLY_WEAK:    intentDesc = "使你虚弱 " + std::to_string(it.value) + " 回合"; break;
        case IntentType::APPLY_VULNERABLE: intentDesc = "使你易伤 " + std::to_string(it.value) + " 回合"; break;
        }
        std::string str = (en.strength > 0) ? ("   力量 " + std::to_string(en.strength)) : "";
        UI::Print(std::to_string(no) + ". " + en.name + "   HP " + std::to_string(en.hp) + "/"
            + std::to_string(en.maxHp) + "   格挡 " + std::to_string(en.block)
            + str + "   意图: " + intentDesc);
        if (en.weak > 0 || en.vulnerable > 0) {
            std::string st;
            if (en.weak > 0)       st += " 虚弱" + std::to_string(en.weak);
            if (en.vulnerable > 0) st += " 易伤" + std::to_string(en.vulnerable);
            UI::Print("   状态:" + st);
        }
        ++no;
    }
    UI::Print("");

    UI::Print("========== 你的手牌 ==========");
    if (p.deck.hand.empty()) {
        UI::Print("(手牌为空,输入 0 结束回合)");
    }
    // 每张手牌直接显示实算后的伤害/格挡,而不是卡面的固定描述
    // 用当前选定目标;还没选过/目标已死就退回第一只存活敌人
    int vuln = 0;
    const int tgt = (cur >= 0 && cur < static_cast<int>(es.size()) && es[cur].hp > 0)
        ? cur : AliveIndex(0);
    if (tgt >= 0) vuln = es[tgt].vulnerable;
    for (size_t i = 0; i < p.deck.hand.size(); ++i)
        UI::Print(std::to_string(i + 1) + ". "
            + DeckView::FormatCardPreview(g_cards[p.deck.hand[i]], p, vuln));
    UI::Print("");
}