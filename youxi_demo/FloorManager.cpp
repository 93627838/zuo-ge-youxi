// FloorManager.cpp
#include "FloorManager.h"
#include "FloorSource.h"
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>
#include <optional>
#include <queue>
#include <stdexcept>

#ifdef _WIN32
#include <windows.h>
#endif

// ---------- 颜色定义 ----------
namespace Color {
    const std::string RESET = "\033[0m";
    const std::string RED = "\033[31m";
    const std::string GREEN = "\033[32m";
    const std::string YELLOW = "\033[33m";
    const std::string BLUE = "\033[34m";
    const std::string MAGENTA = "\033[35m";
    const std::string CYAN = "\033[36m";
    const std::string WHITE = "\033[37m";
    const std::string GRAY = "\033[90m";
    const std::string BOLD = "\033[1m";
    const std::string BRIGHT = "\033[92m";

    void init() {
#ifdef _WIN32
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        if (hOut != INVALID_HANDLE_VALUE) {
            DWORD dwMode = 0;
            if (GetConsoleMode(hOut, &dwMode)) {
                dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
                SetConsoleMode(hOut, dwMode);
            }
        }
#endif
    }
}

using namespace std;

// ---------- 构造函数 ----------
FloorManager::FloorManager() {}

// ---------- 公有方法 ----------
void FloorManager::loadFloor(int floor, unsigned int seed) {
    current_floor = floor;
    boss_defeated = false;
    map_seed = seed;

    LoadedFloor loaded = FloorSource::load(floor, game_mode, seed);
    rooms = std::move(loaded.rooms);
    current_tmpl = std::move(loaded.tmpl);
    current_is_fixed = loaded.fixed_layout;

    current_room_id.clear();
    for (const auto& pair : rooms) {
        if (pair.second.getType() == RoomType::Start) {
            current_room_id = pair.first;
            break;
        }
    }
    if (rooms.find(current_room_id) == rooms.end()) {
        if (!rooms.empty()) {
            current_room_id = rooms.begin()->first;
        }
        else {
            throw runtime_error("楼层数据为空！");
        }
    }
}

const Room& FloorManager::getCurrentRoom() const {
    auto it = rooms.find(current_room_id);
    if (it == rooms.end())
        throw runtime_error("当前房间不存在");
    return it->second;
}

const Room& FloorManager::getRoomById(const string& id) const {
    auto it = rooms.find(id);
    if (it == rooms.end())
        throw runtime_error("房间 " + id + " 不存在");
    return it->second;
}

vector<string> FloorManager::getCurrentOptions() const {
    return getCurrentRoom().getNextIds();
}

bool FloorManager::moveTo(const string& target_id) {
    if (rooms.find(target_id) == rooms.end())
        return false;
    const auto& cur = getCurrentRoom();
    const auto& nexts = cur.getNextIds();
    if (find(nexts.begin(), nexts.end(), target_id) == nexts.end())
        return false;
    current_room_id = target_id;
    return true;
}

optional<RoomReward> FloorManager::clearCurrentRoom() {
    auto& room = rooms[current_room_id];
    if (room.isCompleted())
        return nullopt;
    room.markCleared();
    if (room.isBoss())
        boss_defeated = true;
    return room.getReward();
}

bool FloorManager::tryGoUp() {
    if (!boss_defeated)
        return false;
    if (game_mode == GameMode::Story && current_floor >= 7)
        return false;
    loadFloor(current_floor + 1, map_seed);
    return true;
}

bool FloorManager::isStoryEnd() const {
    return game_mode == GameMode::Story && current_floor >= 7 && boss_defeated;
}

// ---------- 地图绘制 ----------
void FloorManager::drawMap() const {
    if (rooms.empty()) {
        cout << "当前楼层没有地图数据。\n";
        return;
    }

    if (current_is_fixed || current_tmpl.coords.empty()) {
        drawFixedMap();
        return;
    }

    // 1. 计算画布尺寸
    int max_col = 0, max_row = 0;
    // 注意：这里使用的是 current_tmpl，不是 rooms
    for (const auto& coord : current_tmpl.coords) {
        if (coord.first > max_col) max_col = coord.first;
        if (coord.second > max_row) max_row = coord.second;
    }
    const int WIDTH = max_col + 3;
    const int HEIGHT = max_row + 3;

    // 2. 初始化画布
    vector<string> canvas(HEIGHT, string(WIDTH, ' '));
    vector<vector<string>> colorMap(HEIGHT, vector<string>(WIDTH, Color::RESET));

    // 3. 绘制连接线（包括斜线）
    // 注意：使用的是 current_tmpl.edges
    for (const auto& edge : current_tmpl.edges) {
        int from = edge.first;
        int to = edge.second;
        if (from >= (int)current_tmpl.coords.size() || to >= (int)current_tmpl.coords.size())
            continue;
        int x1 = current_tmpl.coords[from].first;
        int y1 = current_tmpl.coords[from].second;
        int x2 = current_tmpl.coords[to].first;
        int y2 = current_tmpl.coords[to].second;

        int dx = x2 - x1;
        int dy = y2 - y1;
        int steps = max(abs(dx), abs(dy));
        if (steps == 0) continue;

        for (int i = 1; i < steps; ++i) {
            int x = x1 + dx * i / steps;
            int y = y1 + dy * i / steps;
            if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT) continue;
            if (canvas[y][x] != ' ') continue; // 节点优先

            if (dx == 0) {
                canvas[y][x] = '|';
            }
            else if (dy == 0) {
                canvas[y][x] = '-';
            }
            else {
                if ((dx > 0 && dy > 0) || (dx < 0 && dy < 0))
                    canvas[y][x] = '\\';
                else
                    canvas[y][x] = '/';
            }
        }
    }

    // 4. 绘制节点（覆盖线条）
    for (const auto& pair : rooms) {
        const Room& room = pair.second;
        int node = -1;
        string id = room.getId();
        size_t pos = id.find('_');
        if (pos != string::npos) {
            string numStr = id.substr(pos + 2);
            try {
                node = stoi(numStr);
            }
            catch (...) { continue; }
        }
        if (node < 0 || node >= (int)current_tmpl.coords.size()) continue;

        int x = current_tmpl.coords[node].first;
        int y = current_tmpl.coords[node].second;
        if (x < 0 || y < 0 || x >= WIDTH || y >= HEIGHT) continue;

        char symbol;
        string color;
        if (room.getId() == current_room_id) {
            symbol = '@';
            color = Color::BOLD + Color::BRIGHT;
        }
        else {
            switch (room.getType()) {
            case RoomType::Start:    symbol = 'P'; color = Color::WHITE;   break;
            case RoomType::Combat:   symbol = 'C'; color = Color::GRAY;    break;
            case RoomType::Elite:    symbol = 'E'; color = Color::YELLOW;  break;
            case RoomType::Boss:     symbol = 'B'; color = Color::RED;     break;
            case RoomType::Treasure: symbol = 'T'; color = Color::CYAN;    break;
            case RoomType::Event:    symbol = 'V'; color = Color::MAGENTA; break;
            case RoomType::Shop:     symbol = 'S'; color = Color::GREEN;   break;
            default:                 symbol = '?'; color = Color::WHITE;   break;
            }
        }
        canvas[y][x] = symbol;
        colorMap[y][x] = color;
    }

    // 5. 输出画布
    cout << "\n===== 当前楼层地图 (楼层 " << current_floor << ") =====\n";
    for (int y = 0; y < HEIGHT; ++y) {
        cout << "\n";
        for (int x = 0; x < WIDTH; ++x) {
            char ch = canvas[y][x];
            if (ch == ' ')
                cout << ' ';
            else
                cout << colorMap[y][x] << ch << Color::RESET;
        }
    }

    // 6. 图例
    cout << "\n图例: ";
    cout << Color::WHITE << "P起点 " << Color::RESET;
    cout << Color::GRAY << "C战斗 " << Color::RESET;
    cout << Color::YELLOW << "E精英 " << Color::RESET;
    cout << Color::RED << "B首领 " << Color::RESET;
    cout << Color::CYAN << "T宝藏 " << Color::RESET;
    cout << Color::MAGENTA << "V事件 " << Color::RESET;
    cout << Color::GREEN << "S商店 " << Color::RESET;
    cout << Color::BOLD << Color::BRIGHT << "@你 " << Color::RESET;
    cout << "\n当前位于: " << getCurrentRoom().getName() << "\n\n";
}

// 固定剧情楼层没有坐标模板，这里按房间连接关系自动生成一张图形地图
void FloorManager::drawFixedMap() const {
    if (rooms.empty())
        return;

    // 给每个房间分配稳定索引
    vector<string> ids;
    unordered_map<string, int> id_index;
    id_index.reserve(rooms.size());
    for (const auto& pair : rooms) {
        id_index[pair.first] = static_cast<int>(ids.size());
        ids.push_back(pair.first);
    }

    // 从起点广度优先分层，后续按层布置位置
    int start = 0;
    for (int i = 0; i < static_cast<int>(ids.size()); ++i) {
        if (rooms.at(ids[i]).getType() == RoomType::Start) {
            start = i;
            break;
        }
    }

    vector<int> layer(ids.size(), -1);
    queue<int> pending;
    pending.push(start);
    layer[start] = 0;
    while (!pending.empty()) {
        const int u = pending.front();
        pending.pop();
        for (const auto& next_id : rooms.at(ids[u]).getNextIds()) {
            auto it = id_index.find(next_id);
            if (it == id_index.end())
                continue;
            const int v = it->second;
            if (layer[v] < 0) {
                layer[v] = layer[u] + 1;
                pending.push(v);
            }
        }
    }

    int max_layer = 0;
    for (int v : layer) {
        if (v > max_layer) max_layer = v;
    }

    vector<vector<int>> by_layer(max_layer + 1);
    for (int i = 0; i < static_cast<int>(ids.size()); ++i) {
        if (layer[i] >= 0)
            by_layer[layer[i]].push_back(i);
    }

    vector<pair<int, int>> coords(ids.size());
    for (int y = 0; y <= max_layer; ++y) {
        for (int p = 0; p < static_cast<int>(by_layer[y].size()); ++p) {
            const int node = by_layer[y][p];
            coords[node] = { p * 4, y * 2 };
        }
    }

    // 若有无法到达的房间，兜底放到最下方，避免下标越界
    int fallback_y = max_layer + 1;
    int fallback_x = 0;
    for (int i = 0; i < static_cast<int>(ids.size()); ++i) {
        if (layer[i] < 0) {
            coords[i] = { fallback_x, fallback_y * 2 };
            ++fallback_x;
        }
    }

    vector<pair<int, int>> edges;
    for (int u = 0; u < static_cast<int>(ids.size()); ++u) {
        for (const auto& next_id : rooms.at(ids[u]).getNextIds()) {
            auto it = id_index.find(next_id);
            if (it != id_index.end())
                edges.push_back({ u, it->second });
        }
    }

    int max_col = 0, max_row = 0;
    for (const auto& coord : coords) {
        max_col = max(max_col, coord.first);
        max_row = max(max_row, coord.second);
    }
    const int WIDTH = max_col + 3;
    const int HEIGHT = max_row + 3;

    vector<string> canvas(HEIGHT, string(WIDTH, ' '));
    vector<vector<string>> color_map(HEIGHT, vector<string>(WIDTH, Color::RESET));

    for (const auto& edge : edges) {
        const int x1 = coords[edge.first].first;
        const int y1 = coords[edge.first].second;
        const int x2 = coords[edge.second].first;
        const int y2 = coords[edge.second].second;
        const int dx = x2 - x1;
        const int dy = y2 - y1;
        const int steps = max(abs(dx), abs(dy));
        if (steps == 0)
            continue;
        for (int i = 1; i < steps; ++i) {
            const int x = x1 + dx * i / steps;
            const int y = y1 + dy * i / steps;
            if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT)
                continue;
            if (canvas[y][x] != ' ')
                continue;
            if (dx == 0)
                canvas[y][x] = '|';
            else if (dy == 0)
                canvas[y][x] = '-';
            else if ((dx > 0 && dy > 0) || (dx < 0 && dy < 0))
                canvas[y][x] = '\\';
            else
                canvas[y][x] = '/';
        }
    }

    for (int i = 0; i < static_cast<int>(ids.size()); ++i) {
        const Room& room = rooms.at(ids[i]);
        const int x = coords[i].first;
        const int y = coords[i].second;
        if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT)
            continue;

        char symbol;
        string color;
        if (ids[i] == current_room_id) {
            symbol = '@';
            color = Color::BOLD + Color::BRIGHT;
        }
        else {
            switch (room.getType()) {
            case RoomType::Start:    symbol = 'P'; color = Color::WHITE;   break;
            case RoomType::Combat:   symbol = 'C'; color = Color::GRAY;    break;
            case RoomType::Elite:    symbol = 'E'; color = Color::YELLOW;  break;
            case RoomType::Boss:     symbol = 'B'; color = Color::RED;     break;
            case RoomType::Treasure: symbol = 'T'; color = Color::CYAN;    break;
            case RoomType::Event:    symbol = 'V'; color = Color::MAGENTA; break;
            case RoomType::Shop:     symbol = 'S'; color = Color::GREEN;   break;
            default:                 symbol = '?'; color = Color::WHITE;   break;
            }
        }
        canvas[y][x] = symbol;
        color_map[y][x] = color;
    }

    cout << "\n===== 当前楼层地图 (第 " << current_floor << " 层 · 固定剧情) =====\n";
    for (int y = 0; y < HEIGHT; ++y) {
        cout << "\n";
        for (int x = 0; x < WIDTH; ++x) {
            const char ch = canvas[y][x];
            if (ch == ' ')
                cout << ' ';
            else
                cout << color_map[y][x] << ch << Color::RESET;
        }
    }

    cout << "\n图例: ";
    cout << Color::WHITE << "P起点 " << Color::RESET;
    cout << Color::GRAY << "C战斗 " << Color::RESET;
    cout << Color::YELLOW << "E精英 " << Color::RESET;
    cout << Color::RED << "B首领 " << Color::RESET;
    cout << Color::CYAN << "T宝藏 " << Color::RESET;
    cout << Color::MAGENTA << "V事件 " << Color::RESET;
    cout << Color::GREEN << "S商店 " << Color::RESET;
    cout << Color::BOLD << Color::BRIGHT << "@你 " << Color::RESET;
    cout << "\n当前位于: " << getCurrentRoom().getName() << "\n\n";
}

