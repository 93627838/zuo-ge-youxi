#include "EncounterBuilder.h"
#include "Data.h"
#include <cstdlib>

namespace {

    // 每层可用的出场组合下标区间(对应 Data.cpp 里的 g_encounters)
    struct FloorPool {
        int normal_lo, normal_hi;   // 普通战斗的候选范围
        int elite_lo, elite_hi;     // 精英战斗的候选范围
        int boss_idx;               // 首领固定用哪一条
    };

    // g_encounters 共 28 条(下标 0~27),按楼层依次排布:
    //   第1层 0~8 (0~4 单体怪 / 5~8 群怪)   第2层 9~11
    //   第3层 12~15   第4层 16~19   第5层 20~23   第6层 24~27
    //   每层内部都是"普通在前、精英在后"。
    FloorPool poolFor(int content_floor) {
        switch (content_floor) {
        case 1:  return { 0,  7,  3,  7,  3  };   // 第1层:精英/首领用"无面战团"一档
        case 2:  return { 9,  10, 11, 11, 11 };
        case 3:  return { 12, 13, 14, 15, 15 };
        case 4:  return { 16, 17, 18, 19, 19 };
        case 5:  return { 20, 21, 22, 23, 23 };
        case 6:  return { 24, 25, 26, 27, 27 };
        default: return { 26, 27, 27, 27, 27 };   // 第7层(空王座):全用最强段
        }
    }

    // 在闭区间 [lo, hi] 里随机取一个
    int pickInRange(int lo, int hi) {
        if (hi <= lo) return lo;
        return lo + std::rand() % (hi - lo + 1);
    }

}

namespace EncounterBuilder {

Setup build(int floor, RoomType type) {
    // 无尽模式:第 8 层起循环复用第 1~6 层的内容(与爬塔的主题循环保持一致)
    int content_floor = floor;
    if (content_floor > 7)
        content_floor = ((content_floor - 8) % 6) + 1;
    if (content_floor < 1)
        content_floor = 1;

    const FloorPool pool = poolFor(content_floor);

    int enc_idx = 0;
    double hp_scale = 1.0;
    switch (type) {
    case RoomType::Boss:
        enc_idx = pool.boss_idx;
        hp_scale = 1.10;    // 首领:略强于同层普通怪(初版数值,可在此统一调平衡)
        break;
    case RoomType::Elite:
        enc_idx = pickInRange(pool.elite_lo, pool.elite_hi);
        hp_scale = 1.00;
        break;
    default:   // Combat 以及其它非精英/首领的战斗房
        enc_idx = pickInRange(pool.normal_lo, pool.normal_hi);
        hp_scale = 0.85;    // 普通遭遇战调弱一点,保证起始牌组打得动
        break;
    }

    // 无尽模式:越往上越强(每超过 7 层 +12% 血量)
    if (floor > 7)
        hp_scale *= 1.0 + 0.12 * (floor - 7);

    const Encounter& ec = g_encounters[enc_idx];

    Setup setup;
    setup.group_name = ec.name;
    for (int enemy_idx : ec.enemy) {
        Enemy en = g_enemies[enemy_idx];   // 从模板拷一份,不改动全局数据
        en.maxHp = static_cast<int>(en.maxHp * hp_scale);
        en.hp = en.maxHp;
        en.block = 0;
        en.step = 0;
        setup.enemies.push_back(en);
    }
    return setup;
}

}   // namespace EncounterBuilder
