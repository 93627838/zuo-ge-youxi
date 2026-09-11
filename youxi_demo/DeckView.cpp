#include "DeckView.h"
#include "Data.h"   // g_cards
#include "UI.h"

namespace DeckView {

    std::string CardTypeName(CardType t) {
        switch (t) {
        case CardType::ATTACK: return "攻击";
        case CardType::SKILL:  return "技能";
        case CardType::POWER:  return "能力";
        }
        return "?";
    }

    void PrintGroupedCounts(const std::vector<int>& cards, const std::string& title) {
        // 按卡名归并,保持首次出现的顺序
        std::vector<std::pair<std::string, int>> uniq;
        for (int idx : cards) {
            if (idx < 0 || idx >= static_cast<int>(g_cards.size())) continue;
            const std::string& nm = g_cards[idx].name;
            bool found = false;
            for (auto& u : uniq)
                if (u.first == nm) { ++u.second; found = true; break; }
            if (!found) uniq.push_back({ nm, 1 });
        }

        UI::Print("===== " + title + " (" + std::to_string(cards.size()) + " 张) =====");
        if (uniq.empty()) {
            UI::Print("(空)");
            return;
        }
        std::string line;
        for (size_t i = 0; i < uniq.size(); ++i) {
            if (i > 0) line += "  ";
            line += uniq[i].first + "×" + std::to_string(uniq[i].second);
        }
        UI::Print(line);
    }

    // 单张卡的伤害文案。系数必须和 Combat::ApplyEffect 里的顺序与整数路径完全一致,
    // 否则预览会骗人(浮点相乘再取整和 dmg*num/den 在部分数值上结果不同)。
    static std::string DamageText(const Card& c, int x, const Player& p, int enemyVulnerable) {
        const int base = x + p.strength;                       // 力量加到攻击上
        const bool triple = (c.type == CardType::ATTACK && p.nextAtkMult > 0);  // 超巨化

        int num = 1, den = 1;
        std::string factors;
        if (p.weak > 0)          { num *= 3; den *= 4; factors += "×0.75"; }  // 你虚弱
        if (p.apotheosis)        { num *= 3; den *= 2; factors += "×1.5"; }   // 登神
        if (enemyVulnerable > 0) { num *= 3; den *= 2; factors += "×1.5"; }   // 敌人易伤

        int dmg = base;
        if (triple) dmg *= 3;
        dmg = dmg * num / den;                                 // ★先乘后除,会截断

        std::string s;
        if (c.isX) s += "每点能量 ";
        s += "造成 " + std::to_string(base);
        if (triple) s += "×3";
        s += factors;
        s += " 点伤害 (" + std::to_string(dmg) + ")";
        return s;
    }

    static std::string BlockText(int x, const Player& p) {
        const int total = x + p.toughness;                     // 坚韧加成:每点 +1 格挡
        if (p.toughness > 0)
            return "获得 " + std::to_string(x) + "+" + std::to_string(p.toughness)
            + " 点格挡 (" + std::to_string(total) + ")";
        return "获得 " + std::to_string(x) + " 点格挡";
    }

    std::string FormatCardPreview(const Card& c, const Player& p, int enemyVulnerable) {
        std::vector<std::string> pieces;
        auto scan = [&](CardEffect ef, int x) {
            switch (ef) {
            case CardEffect::DEAL_DMG:
                pieces.push_back(DamageText(c, x, p, enemyVulnerable)); break;
            case CardEffect::GAIN_BLOCK:
                pieces.push_back(BlockText(x, p)); break;
            case CardEffect::GAIN_STRENGTH:
                pieces.push_back("获得 " + std::to_string(x) + " 点力量"); break;
            case CardEffect::GAIN_TOUGHNESS:
                pieces.push_back("获得 " + std::to_string(x) + " 点坚韧"); break;
            case CardEffect::GAIN_ENERGY:
                pieces.push_back("获得 " + std::to_string(x) + " 点能量"); break;
            case CardEffect::DRAW_CARD:
                pieces.push_back("抽 " + std::to_string(x) + " 张牌"); break;
            case CardEffect::APPLY_WEAK:
                pieces.push_back("给予 " + std::to_string(x) + " 层虚弱"); break;
            case CardEffect::APPLY_VULNERABLE:
                pieces.push_back("给予 " + std::to_string(x) + " 层易伤"); break;
            default:
                break;
            }
            };

        scan(c.effect, c.x);
        scan(c.effect2, c.x2);
        scan(c.effect3, c.x3);

        std::string line = "【" + c.name + "】("
            + (c.isX ? std::string("X") : std::to_string(c.cost))
            + "费, " + CardTypeName(c.type) + ") ";
        if (pieces.empty()) {
            line += c.desc;
            return line;
        }
        for (size_t i = 0; i < pieces.size(); ++i) {
            if (i > 0) line += ", ";
            line += pieces[i];
        }
        return line;
    }

    void ShowDeckOnly(const Player& p) {
        UI::Clear();
        PrintGroupedCounts(p.startDeck, "你的牌组");
        UI::Pause();
    }

}
