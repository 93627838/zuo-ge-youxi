// FloorBuilder.cpp
#include "FloorBuilder.h"
#include "Room.h"

using namespace std;

namespace FloorBuilder {

    unordered_map<string, Room> buildFloor1() {
        unordered_map<string, Room> rooms;
        // ---------- 1. 起始节点 ----------
        Room start("F1_Start", 1, "灰烬之门",
            "你从一道骨裂的缝隙中跌入塔内。身后是闭合的苍白骨壁，\n"
            "前方是灰白色的荒原。远处隐约有两条小径，分别通向左右。",
            RoomType::Start);
        start.addNext("F1_L1");   // 左路
        start.addNext("F1_R1");   // 右路
        rooms["F1_Start"] = move(start);

        // ---------- 2. 左路 ① 普通战斗 ----------
        Room l1("F1_L1", 1, "裂缝战场",
            "地面裂开一道深邃的沟壑，两个枯皮游荡者正在沟边撕扯一具残骸。\n"
            "它们嗅到了你的气味，缓缓转过身来。",
            RoomType::Combat);
        l1.addNext("F1_L2");
        RoomReward r_l1;
        r_l1.gold = 8;
        r_l1.exp = 15;
        r_l1.flavor_text = "你从尸体旁捡到几枚磨损的银币。";
        l1.setReward(r_l1);
        rooms["F1_L1"] = move(l1);

        // ---------- 3. 左路 ② 普通战斗 ----------
        Room l2("F1_L2", 1, "骨堆凹室",
            "凹室里堆满了人骨，其中几根还在微微颤动。\n"
            "一只饥饿幻影从骨堆中升起，它渴望你的生命气息。",
            RoomType::Combat);
        l2.addNext("F1_L3");
        RoomReward r_l2;
        r_l2.gold = 12;
        r_l2.exp = 20;
        r_l2.item_name = "残响: 渴望";
        r_l2.item_effect = "战斗中获得吸血效果（攻击恢复少量生命）";
        r_l2.flavor_text = "幻影消散时留下一段破碎的记忆碎片。";
        l2.setReward(r_l2);
        rooms["F1_L2"] = move(l2);

        // ---------- 4. 左路 ③ 精英 ----------
        Room l3("F1_L3", 1, "巨像守卫",
            "一尊由烬骨拼凑而成的巨像挡在路中央，它的眼眶里燃烧着暗红火焰。\n"
            "地面随着它的步伐微微震动。",
            RoomType::Elite);
        l3.addNext("F1_Merge");
        RoomReward r_l3;
        r_l3.gold = 25;
        r_l3.exp = 40;
        r_l3.item_name = "战神之瞳碎片";
        r_l3.item_effect = "可植入的神骸碎片（物理伤害+8%）";
        r_l3.flavor_text = "巨像崩解时，一颗闪烁的瞳孔碎片落入你手中。";
        l3.setReward(r_l3);
        rooms["F1_L3"] = move(l3);

        // ---------- 5. 右路 ① 商店 ----------
        Room r1("F1_R1", 1, "幻影集市",
            "灰烬之中，一个披着斗篷的模糊身影蹲坐在一堆杂物旁。\n"
            "它向你招手，发出沙子摩擦般的声音：'想交易吗？'",
            RoomType::Shop);
        r1.addNext("F1_R2");
        // 商店无奖励，由外部交易系统处理
        rooms["F1_R1"] = move(r1);

        // ---------- 6. 右路 ② 宝藏 ----------
        Room r2("F1_R2", 1, "沉没宝箱",
            "灰烬中半埋着一只铁质宝箱，锁扣已经锈蚀断裂。\n"
            "隐约有微光从缝隙中透出。",
            RoomType::Treasure);
        r2.addNext("F1_R3");
        RoomReward r_r2;
        r_r2.gold = 0;
        r_r2.exp = 0;
        r_r2.item_name = "锈蚀护符";
        r_r2.item_effect = "每场战斗开始时获得 3 点护甲";
        r_r2.flavor_text = "你打开箱子，里面躺着一枚温热的护符。";
        r2.setReward(r_r2);
        rooms["F1_R2"] = move(r2);

        // ---------- 7. 右路 ③ 精英 ----------
        Room r3("F1_R3", 1, "无面者营地",
            "三个无面战团成员围坐在一堆篝火旁，它们的脸是平滑的骨面。\n"
            "其中两个站起身来，握住了武器。",
            RoomType::Elite);
        r3.addNext("F1_Merge");
        RoomReward r_r3;
        r_r3.gold = 30;
        r_r3.exp = 35;
        r_r3.item_name = "残响: 协同";
        r_r3.item_effect = "获得群体增益技能（本回合内所有队友攻击+20%）";
        r_r3.flavor_text = "它们的残响在你脑海中交织成一段战歌。";
        r3.setReward(r_r3);
        rooms["F1_R3"] = move(r3);

        // ---------- 8. 汇合节点 (事件) ----------
        Room merge("F1_Merge", 1, "风暴中心",
            "两条小径交汇于一处低洼地。这里的灰烬像活物一样旋转成漩涡。\n"
            "你听见风中夹杂着无数登塔者的低语。",
            RoomType::Event);
        merge.addNext("F1_Boss");
        RoomReward r_merge;
        r_merge.gold = 5;
        r_merge.exp = 10;
        r_merge.item_name = "骨灰印记";
        r_merge.item_effect = "本层内攻击+10% (持续到击败BOSS)";
        r_merge.flavor_text = "你伸手探入漩涡，一枚灰烬凝成的徽记烙在你手心。";
        merge.setReward(r_merge);
        rooms["F1_Merge"] = move(merge);

        // ---------- 9. BOSS ----------
        Room boss("F1_Boss", 1, "烬骨王座",
            "灰烬平原的尽头，一张由肋骨与脊椎骨铸成的王座矗立在高台上。\n"
            "王座上坐着战神的残影——它没有实体，只是一张飘动的、燃烧的皮肤。\n"
            "‘又一个来赴圣餐的……’ 它的声音从四面八方传来。",
            RoomType::Boss);
        // BOSS 无后续节点
        RoomReward r_boss;
        r_boss.gold = 80;
        r_boss.exp = 120;
        r_boss.item_name = "战神·烬骨的皮肤 (核心)";
        r_boss.item_effect = "可植入核心：物理伤害+30%，但无法逃跑";
        r_boss.flavor_text = "皮肤被剥离时，你感到战神的怒火涌入血管。整个第一层开始崩塌。";
        boss.setReward(r_boss);
        rooms["F1_Boss"] = move(boss);

        return rooms;
    }

    unordered_map<string, Room> buildFloor2() {
        unordered_map<string, Room> rooms;
        // ---------- 1. 起始节点 ----------
        Room start("F2_Start", 2, "倒悬之穹",
            "你踏上通往第二层的阶梯，然后踏空了。\n"
            "不是坠落，是翻转。你站在一片倒悬的海的底部。\n"
            "海面在你头顶，光线透过水层洒下摇晃的光斑。\n"
            "空气里有咸腥味，每一次呼吸都像在喝进薄雾。\n"
            "两条路延伸出去：左边是被海水浸透的岩洞，右边是泛着磷光的浅滩。",
            RoomType::Start);
        start.addNext("F2_L1");
        start.addNext("F2_R1");
        rooms["F2_Start"] = move(start);

        // ---------- 2. 左路 ① 垂泪岩洞（战斗） ----------
        Room l1("F2_L1", 2, "垂泪岩洞",
            "洞壁光滑如镜，水珠从钟乳石上逆流而上。\n"
            "洞中央站着一个水汽凝聚的身影——半透明，形状似人。\n"
            "它伸出手，手心朝上，像在索要什么。\n"
            "你知道它在等你的呼吸。",
            RoomType::Combat);
        l1.addNext("F2_L2");
        RoomReward r_l1;
        r_l1.gold = 15;
        r_l1.exp = 22;
        r_l1.item_name = "海神泪滴 (碎片)";
        r_l1.item_effect = "每回合恢复 2 点生命";
        r_l1.flavor_text = "水形幻影散成小雨，你从水洼中捡到光滑的石片。";
        l1.setReward(r_l1);
        rooms["F2_L1"] = move(l1);

        // ---------- 3. 左路 ② 沉船残骸（事件） ----------
        Room l2("F2_L2", 2, "沉船残骸",
            "一艘折断的木制帆船搁浅在湖中央。\n"
            "船身覆盖着白色珊瑚，船尾舷窗透出微弱灯光。\n"
            "舱内传来低沉沙哑的声音：‘……有人吗……’",
            RoomType::Event);
        l2.addNext("F2_Merge");
        RoomReward r_l2;
        r_l2.gold = 0;
        r_l2.exp = 10;
        r_l2.item_name = "深海标记";
        r_l2.item_effect = "本层内先手率 +20%";
        r_l2.flavor_text = "老人递给你一卷海图，然后永远闭上了眼睛。";
        l2.setReward(r_l2);
        rooms["F2_L2"] = move(l2);

        // ---------- 4. 右路 ① 浮光浅滩（战斗） ----------
        Room r1("F2_R1", 2, "浮光浅滩",
            "水面泛着磷光，像无数颗星星在水底闪烁。\n"
            "你每走一步，脚下的光就波动一次。\n"
            "一只由海水和骨骼构成的猎鲨破水而出，\n"
            "它绕着你游了一圈，留下一道发光的尾迹。",
            RoomType::Combat);
        r1.addNext("F2_R2");
        RoomReward r_r1;
        r_r1.gold = 18;
        r_r1.exp = 25;
        r_r1.item_name = "猎鲨鳞片";
        r_r1.item_effect = "本层内快速类技能伤害 +15%";
        r_r1.flavor_text = "猎鲨碎裂成光点，你在浅滩中摸到一块光滑的鳞片。";
        r1.setReward(r_r1);
        rooms["F2_R1"] = move(r1);

        // ---------- 5. 右路 ② 珊瑚迷宫（战斗） ----------
        Room r2("F2_R2", 2, "珊瑚迷宫",
            "灰白色的珊瑚高过头顶，枝杈交错。\n"
            "它们像活物一样缓慢移动，堵住你的退路。\n"
            "迷宫深处传来低沉的共鸣——它在消化你。\n"
            "碎片中爬出一只由珊瑚拼凑成的怪物。",
            RoomType::Combat);
        r2.addNext("F2_Merge");
        RoomReward r_r2;
        r_r2.gold = 20;
        r_r2.exp = 28;
        r_r2.item_name = "血红珊瑚";
        r_r2.item_effect = "本层内生命恢复效果 +20%";
        r_r2.flavor_text = "珊瑚崩塌，你捡到一小块暗红色珊瑚。";
        r2.setReward(r_r2);
        rooms["F2_R2"] = move(r2);

        // ---------- 6. 汇合节点：深母之泪（宝藏） ----------
        Room merge("F2_Merge", 2, "深母之泪",
            "左右两路交汇于一座圆形水潭。\n"
            "潭水深蓝不透明，中心沉着一颗暗蓝色的泪滴形晶体。\n"
            "它发出脉动，像在呼吸。\n"
            "你伸手触碰时，水纹自动散开，晶体缓缓浮起。",
            RoomType::Treasure);
        merge.addNext("F2_Elite");
        RoomReward r_merge;
        r_merge.gold = 0;
        r_merge.exp = 0;
        r_merge.item_name = "深母之泪 (器官)";
        r_merge.item_effect = "主动：消耗少量生命，恢复已损生命的30%（每层限一次）";
        r_merge.flavor_text = "晶体在你手中融化成暗蓝色光流，渗入你的手臂。";
        merge.setReward(r_merge);
        rooms["F2_Merge"] = move(merge);

        // ---------- 7. 回声深渊（精英） ----------
        Room elite("F2_Elite", 2, "回声深渊",
            "水从脚踝漫到膝盖，再到胸口。\n"
            "这里的水是黑色的，光透不进去。\n"
            "你听到回声——有人模仿你每一个动作的声音。\n"
            "你停下来时，水下的影子也停了下来。\n"
            "但它睁着眼睛，而你的倒影应该是闭着眼的。",
            RoomType::Elite);
        elite.addNext("F2_Boss");
        RoomReward r_elite;
        r_elite.gold = 35;
        r_elite.exp = 45;
        r_elite.item_name = "镜面碎片";
        r_elite.item_effect = "被动：受到致命伤害时20%概率转移此伤害到下次攻击（每层限一次）";
        r_elite.flavor_text = "倒影裂成碎片，你再也看不到自己的倒影了。";
        elite.setReward(r_elite);
        rooms["F2_Elite"] = move(elite);

        // ---------- 8. BOSS：深母王座 ----------
        Room boss("F2_Boss", 2, "深母王座",
            "珊瑚和船骸堆成的高台高出水面一人。\n"
            "台上坐着一个女人的轮廓——由水凝聚而成。\n"
            "她眼角有一滴永不坠落的泪水。\n"
            "她站起来时，整片倒悬海开始倒流，向地面压缩而来。",
            RoomType::Boss);
        RoomReward r_boss;
        r_boss.gold = 100;
        r_boss.exp = 150;
        r_boss.item_name = "深母之泪 (核心)";
        r_boss.item_effect = "生命<30%时自动恢复20%并获得隐身一回合（每层限一次）\n代价：杀死有名字的怪物后下一回合无法行动";
        r_boss.flavor_text = "眼泪没入你的手背，留下蓝色印记。整个第二层翻转回正立。";
        boss.setReward(r_boss);
        rooms["F2_Boss"] = move(boss);

        return rooms;
    }

} // namespace FloorBuilder