#include "FloorSource.h"
#include "FloorBuilder.h"
#include "FloorGenerator.h"
#include <stdexcept>
#include <queue>

namespace FloorSource {

namespace {

// 根据手写固定关卡的房间连接关系，自动推算地图拓扑（BFS 分层布局）。
// 这样故事模式无需手写坐标模板，也能在 drawMap 中渲染出 ASCII 拓扑图。
MapTemplate buildFixedTemplate(const std::unordered_map<std::string, Room>& rooms) {
    MapTemplate tmpl;
    tmpl.total_rooms = static_cast<int>(rooms.size());

    std::string start_id, boss_id;
    for (const auto& kv : rooms) {
        if (kv.second.getType() == RoomType::Start)     start_id = kv.first;
        else if (kv.second.getType() == RoomType::Boss)  boss_id = kv.first;
    }

    // BFS 顺序即节点序号（起点为 0）
    std::vector<std::string> order;
    std::unordered_map<std::string, int> index_of;
    std::vector<int> node_layer;

    if (!start_id.empty()) {
        std::queue<std::string> q;
        q.push(start_id);
        index_of[start_id] = 0;
        order.push_back(start_id);
        node_layer.push_back(0);
        while (!q.empty()) {
            std::string cur = q.front(); q.pop();
            int cur_idx = index_of[cur];
            int cur_layer = node_layer[cur_idx];
            for (const auto& nxt : rooms.at(cur).getNextIds()) {
                if (index_of.find(nxt) == index_of.end()) {
                    index_of[nxt] = static_cast<int>(order.size());
                    order.push_back(nxt);
                    node_layer.push_back(cur_layer + 1);
                    q.push(nxt);
                }
            }
        }
    }
    // 兜底：把未被 BFS 访问到的房间补到末尾（正常不会发生）
    for (const auto& kv : rooms) {
        if (index_of.find(kv.first) == index_of.end()) {
            index_of[kv.first] = static_cast<int>(order.size());
            order.push_back(kv.first);
            node_layer.push_back(0);
        }
    }

    tmpl.total_rooms = static_cast<int>(order.size());
    tmpl.start_node = start_id.empty() ? 0 : index_of[start_id];
    tmpl.boss_node  = boss_id.empty()  ? 0 : index_of[boss_id];

    // 坐标：列 = 层号*3，行 = 同层内序号*2（保证非负）
    std::vector<int> layer_count;
    for (int i = 0; i < tmpl.total_rooms; ++i) {
        int l = node_layer[i];
        if (l >= static_cast<int>(layer_count.size())) layer_count.resize(l + 1, 0);
        int pos = layer_count[l]++;
        tmpl.coords.push_back({ l * 3, pos * 2 });
    }

    // 边：按房间连接关系
    for (int i = 0; i < tmpl.total_rooms; ++i) {
        const auto& room = rooms.at(order[i]);
        for (const auto& nxt : room.getNextIds()) {
            auto it = index_of.find(nxt);
            if (it != index_of.end())
                tmpl.edges.push_back({ i, it->second });
        }
    }

    tmpl.node_ids = order;
    return tmpl;
}

FloorData loadFixed(int floor) {
    FloorData result;
    result.fixed_layout = true;

    switch (floor) {
    case 1:  result.rooms = FloorBuilder::buildFloor1(); break;
    case 2:  result.rooms = FloorBuilder::buildFloor2(); break;
    case 3:  result.rooms = FloorBuilder::buildFloor3(); break;
    case 4:  result.rooms = FloorBuilder::buildFloor4(); break;
    case 5:  result.rooms = FloorBuilder::buildFloor5(); break;
    case 6:  result.rooms = FloorBuilder::buildFloor6(); break;
    case 7:  result.rooms = FloorBuilder::buildFloor7(); break;
    default: throw std::runtime_error("固定剧情地图只支持第1~7层");
    }

    result.tmpl = buildFixedTemplate(result.rooms);
    return result;
}

FloorData loadGenerated(int floor, unsigned int seed) {
    // 楼层号继续增长，但文本主题循环前面 1~6 层
    int theme_floor = ((floor - 8) % 6) + 1;

    // 固定种子下每层也不完全相同
    unsigned int effective_seed =
        (seed == 0) ? 0 : seed + static_cast<unsigned int>(floor);

    FloorGenerator generator;
    auto generated = generator.generateFloor(floor, effective_seed, theme_floor);

    FloorData result;
    result.rooms = std::move(generated.rooms);
    result.tmpl = std::move(generated.tmpl);
    return result;
}

} // namespace

FloorData load(int floor, GameMode mode, unsigned int seed) {
    if (mode == GameMode::Story || floor <= 7)
        return loadFixed(floor);
    return loadGenerated(floor, seed);
}

} // namespace FloorSource
