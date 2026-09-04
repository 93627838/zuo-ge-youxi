// FloorGenerator.cpp
#include "FloorGenerator.h"
#include "MapTemplates.h"
#include <random>
#include <algorithm>
#include <stdexcept>
#include <vector>

using namespace std;

// ---------- 工具函数：生成房间ID ----------
string FloorGenerator::genId(int floor, int nodeIndex) {
    return "F" + to_string(floor) + "_N" + to_string(nodeIndex);
}

// ---------- 文本池：房间名称 ----------
static const vector<string>& getNamesImpl(RoomType type, int floor) {
    // ----- 普通战斗名称池（按楼层）-----
    static const vector<string> combat_f1 = {
        "裂缝战场", "骨堆凹室", "浸血走廊", "灰烬岗哨", "焦土之径"
    };
    static const vector<string> combat_f2 = {
        "垂泪岩洞", "浮光浅滩", "暗流漩涡", "珊瑚裂谷", "泡沫之渊"
    };
    static const vector<string> combat_f3 = {
        "黄金回廊", "镜面大厅", "熔岩裂隙", "虚空节点", "齿轮墓穴"
    };
    // 普通战斗3层以后统一使用 combat_f3（可扩展）

    // ----- 精英名称池（按楼层）-----
    static const vector<string> elite_f1 = {
        "巨像守卫", "无面者营地", "烬骨巨像"
    };
    static const vector<string> elite_f2 = {
        "回声深渊", "深影猎鲨", "珊瑚之王"
    };
    static const vector<string> elite_f3 = {
        "黄金熔炉", "黄金赌场", "镀金守卫"
    };
    static const vector<string> elite_f4 = {
        "沉默猎手巢穴", "暗影之径", "遗忘真名者"
    };
    static const vector<string> elite_f5 = {
        "锻锤回廊", "铁水领主", "契约烙印者"
    };
    static const vector<string> elite_f6 = {
        "命运之枢", "镜像之核", "织网者之影"
    };

    // ----- 通用池（事件、宝藏、商店）-----
    static const vector<string> event_names = {
        "沉船残骸", "风暴中心", "流浪者营地", "遗忘祭坛", "时空裂隙"
    };
    static const vector<string> treasure_names = {
        "沉没宝箱", "深母之泪", "古代遗物", "神骸碎片", "星尘宝匣"
    };
    static const vector<string> shop_names = {
        "幻影集市", "漂流商贩", "黑市裂隙", "地精黑店"
    };
    static const vector<string> empty = { "无名空洞" };

    // ----- 类型分发 -----
    if (type == RoomType::Elite) {
        if (floor == 1) return elite_f1;
        else if (floor == 2) return elite_f2;
        else if (floor == 3) return elite_f3;
        else if (floor == 4) return elite_f4;
        else if (floor == 5) return elite_f5;
        else if (floor == 6) return elite_f6;
        else return elite_f6;  // 超出6层回退到第6层
    }
    else if (type == RoomType::Combat) {
        if (floor == 1) return combat_f1;
        else if (floor == 2) return combat_f2;
        else if (floor >= 3) return combat_f3;  // 3层及以上统一用第三套
        else return combat_f3;
    }
    else if (type == RoomType::Event) {
        return event_names;
    }
    else if (type == RoomType::Treasure) {
        return treasure_names;
    }
    else if (type == RoomType::Shop) {
        return shop_names;
    }
    return empty;
}

const vector<string>& FloorGenerator::getNames(RoomType type, int floor) {
    return getNamesImpl(type, floor);
}

// ---------- 文本池：房间描述 ----------
static const vector<string>& getDescsImpl(RoomType type, int floor) {
    // ----- 普通战斗描述池（按楼层）-----
    static const vector<string> desc_combat_f1 = {
        "地面裂开一道沟壑，两个枯皮游荡者正在撕扯残骸。它们嗅到了你的气味。",
        "凹室里堆满白骨，一只饥饿幻影从骨堆中升起，渴望你的生命气息。",
        "走廊两侧墙壁渗出暗红色液体，空气中弥漫着铁锈与腐败的味道。",
        "灰烬中站立着一具无头铠甲，手中握着一柄断剑，剑刃还在滴血。",
        "焦黑的地面上插满了断裂的武器，像一座无名战士的集体墓地。"
    };
    static const vector<string> desc_combat_f2 = {
        "洞壁光滑如镜，水珠逆流而上。一个水汽凝聚的身影伸出手，像在索要什么。",
        "水面泛着磷光，一只由海水和骨骼构成的猎鲨破水而出，绕着您游弋。",
        "海底漩涡缓缓旋转，中心有黑影在游动，散发着深寒的恶意。",
        "珊瑚枝杈间闪动着数双发光的眼睛，那是某种古老而饥饿的存在。",
        "泡沫从深水区涌起，带着腐朽的气味，水底有东西在缓慢逼近。"
    };
    static const vector<string> desc_combat_f3 = {
        "金色墙壁上布满裂痕，每一道裂痕中都透出刺眼的光芒，像神的注视。",
        "无数面镜子组成迷宫，每面镜子中都有一个不同的您，有的已经死去。",
        "熔岩在脚下流淌，空气中热浪扭曲了视线，一个由铁水构成的生物正在成形。",
        "虚空在墙壁中涌动，您能听到宇宙在低语，说着不属于人类的语言。",
        "齿轮与发条在墙壁内转动，整个房间像一座巨大的机械心脏在跳动。"
    };

    // ----- 精英描述池（按楼层）-----
    static const vector<string> desc_elite_f1 = {
        "一尊由烬骨拼凑而成的巨像挡在路中央，它的眼眶里燃烧着暗红火焰。地面随着它的步伐微微震动。",
        "三个无面战团成员围坐在篝火旁，它们的脸是平滑的骨面。其中两个站起身，握住了武器。",
        "巨大的烬骨之影从灰烬中升起，它的身躯遮蔽了天空，每一次呼吸都带起一阵骨灰风暴。"
    };
    static const vector<string> desc_elite_f2 = {
        "水是黑色的，光透不进去。你的倒影从水下浮起，与你的动作完全一致，但它的眼睛是睁开的。",
        "一头由海水和深影构成的猎鲨在深渊中游弋，它的速度超越了视觉，只留下一道暗蓝色的尾迹。",
        "珊瑚丛中崛起一座王座，上面端坐着珊瑚之王，它的身体由血红色的珊瑚构成，周身缠绕着溺亡者的魂灵。"
    };
    static const vector<string> desc_elite_f3 = {
        "一座巨大的黄金熔炉矗立在大厅中央，炉中流淌的液态黄金像血液一样循环。一个金色的人形从炉中升起，它向你伸出一只手，等待你的贡品。",
        "圆形大厅里有一座金色的命运轮盘，周围坐着三个戴金色面具的身影。它们邀请你下注，赌注是你身上最珍贵的东西。",
        "镀金守卫从墙壁中走出，它的身体由金砖和宝石拼接而成，每一步都让地面震颤。它沉默地注视着你，像在等待你的屈服。"
    };
    static const vector<string> desc_elite_f4 = {
        "一个由发丝编织成的巨大茧状巢穴，内部挂满半透明的丝囊，每一个都蜷缩着某种生物的轮廓。一只沉默猎手蹲坐中央，它的眼睛是两团苍白的火焰。",
        "你走在一条狭窄的暗影之径上，两侧的树木像活物一样向中间挤压。一个身穿黑袍的身影堵住了去路，它用你的真名念诵诅咒。",
        "遗忘真名者悬浮于井口之上，它手持一本灰皮古书，书中记录着所有登塔者的真名。它每念出一个名字，对应的灵魂就会在井中显现。"
    };
    static const vector<string> desc_elite_f5 = {
        "锻锤回廊两侧的墙壁上挂着无数柄战锤，每一柄都沾着干涸的血迹。一个由铁水凝聚的铁匠巨人挡住了去路，它手中的战锤比你整个人还大。",
        "铁水领主从熔岩之河中站起，它的身体完全由滚烫的铁水构成，任何物理攻击都会使它溅射，灼伤攻击者。",
        "契约烙印者站在铁砧前，它用一支烧红的铁笔在空气中写下契约。它向你提出交易：献上你的鲜血，换取力量；或者拒绝，承受它的怒火。"
    };
    static const vector<string> desc_elite_f6 = {
        "一座圆形大殿，地面刻着巨大的命运罗盘。一条银白色的命运残线在空中流动，它已经预见了你的每一步行动。",
        "无数面镜子组成一个球形空间，每一面镜中都映出不同版本的你。镜像之核悬浮在中央，它能反射你的攻击，并将你推入你从未走过的岔路。",
        "织网者之影坐在命运丝线交织的王座上，它的面容在不停地变化——时而像男人，时而像女人，时而像你自己。它轻声说着：'你相信命运吗？'"
    };

    // ----- 通用事件/宝藏/商店描述 -----
    static const vector<string> desc_event = {
        "一艘折断的帆船搁浅在湖中央，舷窗透着微弱的灯光，舱内传来低沉的叹息。",
        "灰烬旋转成巨大的漩涡，无数低语声在呼唤您，像是在念诵一个古老的名字。",
        "一堆未熄灭的篝火旁坐着一个模糊的身影，它抬头看了您一眼，又低下了头。",
        "一座残破的石碑立在路旁，碑文已模糊不清，但您能感觉到它记录着一个重大的牺牲。",
        "空气中裂开一道缝隙，缝隙中透出不属于这个世界的光芒和声音。"
    };
    static const vector<string> desc_treasure = {
        "半埋在灰烬中的铁质宝箱，锁扣已经锈蚀断裂，缝隙中透出淡金色的微光。",
        "水潭中央沉着一颗暗蓝色的泪滴形晶体，它发出均匀的脉动，像在呼吸。",
        "一具无名骷髅的指骨上套着一枚古旧的戒指，戒面刻着永不磨损的纹章。",
        "地面裂开一道缝隙，露出下方发光的石室，里面堆满了早已消逝文明的遗物。",
        "一道彩虹色的光柱从天花板射下，照在正中央的一件悬空器物上。"
    };
    static const vector<string> desc_shop = {
        "披着破旧斗篷的商人蹲坐在一堆杂物旁，它向你招手：'过来看看……'",
        "一只巨大的贝壳被改造成货架，里面陈列着各种在阳光下不会存在的事物。",
        "黑暗中漂浮着一团柔和的光，光晕中隐约可见货架和柜台，一个声音在迎接您。",
        "地面上铺着一张旧毯子，毯子上散落着硬币、瓶子和奇怪的雕像。"
    };
    static const vector<string> empty = { "空空如也。您什么都没找到。" };

    // ----- 分发 -----
    if (type == RoomType::Elite) {
        if (floor == 1) return desc_elite_f1;
        else if (floor == 2) return desc_elite_f2;
        else if (floor == 3) return desc_elite_f3;
        else if (floor == 4) return desc_elite_f4;
        else if (floor == 5) return desc_elite_f5;
        else if (floor == 6) return desc_elite_f6;
        else return desc_elite_f6;
    }
    else if (type == RoomType::Combat) {
        if (floor == 1) return desc_combat_f1;
        else if (floor == 2) return desc_combat_f2;
        else if (floor >= 3) return desc_combat_f3;
        else return desc_combat_f3;
    }
    else if (type == RoomType::Event) {
        return desc_event;
    }
    else if (type == RoomType::Treasure) {
        return desc_treasure;
    }
    else if (type == RoomType::Shop) {
        return desc_shop;
    }
    return empty;
}

const vector<string>& FloorGenerator::getDescs(RoomType type, int floor) {
    return getDescsImpl(type, floor);
}

// ---------- 奖励生成 ----------
RoomReward FloorGenerator::generateReward(RoomType type, int floor, mt19937& rng) {
    RoomReward reward;
    uniform_int_distribution<int> d_gold(5, 30);
    uniform_int_distribution<int> d_exp(10, 40);

    switch (type) {
    case RoomType::Combat: {
        reward.gold = d_gold(rng) + floor * 2;
        reward.exp = d_exp(rng) + floor * 3;
        reward.flavor_text = "敌人化为尘埃，您从残骸中捡到了战利品。";
        break;
    }
    case RoomType::Elite: {
        reward.gold = d_gold(rng) * 2 + floor * 5;
        reward.exp = d_exp(rng) * 2 + floor * 8;
        reward.item_name = "神骸碎片";
        reward.item_effect = "植入后获得随机被动效果（需在安全区使用）";
        reward.flavor_text = "精英消散时，一块闪烁着暗红光芒的碎片落在您手中。";
        break;
    }
    case RoomType::Treasure: {
        reward.gold = 0;
        reward.exp = 0;
        reward.item_name = "古代遗物";
        reward.item_effect = "生命上限永久 +8";
        reward.flavor_text = "您打开宝箱，一股温暖的气息涌入体内，您感觉更坚韧了。";
        break;
    }
    case RoomType::Event: {
        reward.gold = d_gold(rng) / 2 + 2;
        reward.exp = d_exp(rng) / 2 + 5;
        reward.flavor_text = "事件平息后，您从余波中拾得一些零碎的资源。";
        break;
    }
    case RoomType::Boss: {
        reward.gold = 80 + floor * 20;
        reward.exp = 120 + floor * 30;
        reward.item_name = "神之核心";
        reward.item_effect = "植入后获得强大神性能力，但伴随代价（需确认植入）";
        reward.flavor_text = "守护者轰然倒下，它的核心在您手中剧烈跳动，像是另一颗心脏。";
        break;
    }
    default: {
        // Start / Shop 无奖励
        break;
    }
    }
    return reward;
}

// ---------- 核心生成函数 ----------
GenerationResult FloorGenerator::generateFloor(int floor, unsigned int seed, int text_floor) {
    // 1. 初始化随机数生成器
    mt19937 rng(seed ? seed : random_device{}());

    // 2. 从池中随机选择一个地图模板
    auto pool = MapTemplates::getPoolForFloor(floor);
    if (pool.empty()) {
        throw runtime_error("当前楼层没有可用的地图模板！");
    }
    uniform_int_distribution<int> d_tmpl(0, static_cast<int>(pool.size() - 1));
    MapTemplate tmpl = pool[d_tmpl(rng)];

    // 无尽模式中楼层号继续增长，但文字主题可以循环复用前面的楼层
    const int actual_floor = floor;
    if (text_floor <= 0)
        text_floor = floor;
    floor = text_floor;

    // 3. 为每个节点分配房间类型
    vector<RoomType> node_types(tmpl.total_rooms);
    for (int i = 0; i < tmpl.total_rooms; ++i) {
        if (i == tmpl.start_node) {
            node_types[i] = RoomType::Start;
        }
        else if (i == tmpl.boss_node) {
            node_types[i] = RoomType::Boss;
        }
        else {
            int dist_to_boss = tmpl.boss_node - i;
            uniform_int_distribution<int> d_type(0, 99);
            int roll = d_type(rng);

            if (roll < 50) {
                node_types[i] = RoomType::Combat;
            }
            else if (roll < 70) {
                node_types[i] = RoomType::Event;
            }
            else if (roll < 85) {
                node_types[i] = RoomType::Treasure;
            }
            else if (roll < 95) {
                node_types[i] = RoomType::Shop;
            }
            else {
                node_types[i] = RoomType::Elite;
            }

            if (dist_to_boss <= 2 && node_types[i] != RoomType::Elite) {
                uniform_int_distribution<int> d_force(0, 4);
                if (d_force(rng) == 0) {
                    node_types[i] = RoomType::Elite;
                }
            }
        }
    }

    // 额外安全措施：如果整层一个精英都没有，把BOSS前一个节点强制改为精英
    bool has_elite = false;
    for (auto type : node_types) {
        if (type == RoomType::Elite) { has_elite = true; break; }
    }
    if (!has_elite && tmpl.boss_node > 0) {
        int prev = tmpl.boss_node - 1;
        if (prev >= 0 && node_types[prev] != RoomType::Start && node_types[prev] != RoomType::Boss) {
            node_types[prev] = RoomType::Elite;
        }
    }

    // 4. 构建每个房间对象
    unordered_map<string, Room> rooms;
    for (int i = 0; i < tmpl.total_rooms; ++i) {
        RoomType type = node_types[i];
        string id = genId(actual_floor, i);

        const auto& names = getNames(type, floor);
        const auto& descs = getDescs(type, floor);
        uniform_int_distribution<int> d_text(0, static_cast<int>(names.size() - 1));
        int idx = d_text(rng);

        string name = names.empty() ? "无名之地" : names[idx];
        string desc = descs.empty() ? "这里什么也没有。" : descs[idx];

        // ---------- 特殊节点：起始节点 ----------
        if (type == RoomType::Start) {
            if (floor == 1) {
                name = "灰烬之门";
                desc = "您从骨裂的缝隙中跌入塔内，前方是灰白色的荒原。";
            }
            else if (floor == 2) {
                name = "倒悬之穹";
                desc = "您踏入一片倒悬的海，海面在您头顶，光斑摇晃。";
            }
            else if (floor == 3) {
                name = "黄金之门";
                desc = "您推开一扇由纯金铸造的巨门，门后是黄金迷宫的起点。";
            }
            else if (floor == 4) {
                name = "沉默之门";
                desc = "您掀开黑色发丝织成的门帘，踏入了寂静的低语森林。";
            }
            else if (floor == 5) {
                name = "铁心之门";
                desc = "您推开沉重的铁门，热浪扑面而来，这是熔炉胸腔的入口。";
            }
            else if (floor == 6) {
                name = "镜面之门";
                desc = "您推开由无数镜面拼合而成的门，走进了自我的颅腔。";
            }
            else {
                name = "楼层入口";
                desc = "您站在新楼层的起点，前方的道路在迷雾中延伸。";
            }
        }
        // ---------- 特殊节点：BOSS节点 ----------
        else if (type == RoomType::Boss) {
            if (floor == 1) {
                name = "烬骨王座";
                desc = "一张由肋骨与脊椎铸成的王座矗立在高台上，战神的残影站了起来。";
            }
            else if (floor == 2) {
                name = "深母王座";
                desc = "由水凝聚而成的女人坐在珊瑚高台上，她的泪水从未坠落。";
            }
            else if (floor == 3) {
                name = "镀金王座";
                desc = "一座由纯金铸造的王座，镀金者之影正端坐其上，手中把玩着一枚永远旋转的金币。";
            }
            else if (floor == 4) {
                name = "沉默王座";
                desc = "一张由黑色发丝编织的王座，沉默者之影静坐其上，没有面孔，只有无尽的寂静。";
            }
            else if (floor == 5) {
                name = "铁心王座";
                desc = "一座由熔铁浇铸的王座，铁心之影立于熔炉之巅，手中握着燃烧的契约书。";
            }
            else if (floor == 6) {
                name = "织网王座";
                desc = "一张由命运丝线织成的王座，织网者之影端坐中央，周围环绕着无数银白色的时间线。";
            }
            else {
                name = "领主王座";
                desc = "此层的守护者正端坐于王座之上，等待着挑战者的到来。";
            }
        }

        // ---------- 创建房间 ----------
        Room room(id, actual_floor, name, desc, type);
        RoomReward reward = generateReward(type, actual_floor, rng);
        room.setReward(reward);
        rooms[id] = move(room);
    }

    // 5. 根据模板的边（edges）连接房间
    for (const auto& edge : tmpl.edges) {
        int from = edge.first;
        int to = edge.second;
        string from_id = genId(actual_floor, from);
        string to_id = genId(actual_floor, to);

        if (rooms.find(from_id) != rooms.end() && rooms.find(to_id) != rooms.end()) {
            rooms[from_id].addNext(to_id);
        }
    }

    // 6. 返回 GenerationResult（包含房间地图和模板）
    return { rooms, tmpl };
}
