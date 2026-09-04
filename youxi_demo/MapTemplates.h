// MapTemplates.h
#pragma once
#include <vector>
#include <utility>

struct MapTemplate {
    int total_rooms;
    std::vector<std::pair<int, int>> edges;      // (from, to)
    int start_node;
    int boss_node;
    std::vector<std::pair<int, int>> coords;     // 每个节点的 (列, 行) 坐标
};

namespace MapTemplates {

    // ---------- 样式1：直链 + 单分支 ----------
    // 7个节点，起始 -> 分支 -> 汇合 -> 精英 -> BOSS
    inline MapTemplate getStraightBranch() {
        return {
            7,
            {{0,1}, {1,2}, {1,3}, {2,4}, {3,4}, {4,5}, {5,6}},
            0, 6,
            // 坐标 (列, 行) 故意留出间距便于画线
            {{0,0}, {2,0}, {4,0}, {4,2}, {6,1}, {8,1}, {10,1}}
        };
    }

    // ---------- 样式2：双叉树（经典爬塔风格）----------
    // 10个节点：起始 -> 左右各2 -> 汇合 -> 精英 -> BOSS
    inline MapTemplate getDoubleTree() {
        return {
            10,
            {{0,1}, {0,2}, {1,3}, {1,4}, {2,5}, {2,6},
             {3,7}, {4,7}, {5,7}, {6,7}, {7,8}, {8,9}},
            0, 9,
            // 坐标布局：左路和右路对称，汇合在中间，带斜线连接
            {{0,0}, {2,1}, {2,3}, {4,0}, {4,2}, {4,4}, {4,6}, {6,3}, {8,3}, {10,3}}
        };
    }

    // ---------- 样式3：菱形环（带回溯）----------
    // 9个节点：起始 -> 左右 -> 环中连通 -> 汇合 -> BOSS
    inline MapTemplate getDiamondLoop() {
        return {
            9,
            {{0,1}, {0,2}, {1,3}, {2,4}, {3,5}, {4,5},
             {5,6}, {5,7}, {6,8}, {7,8}},
            0, 8,
            // 菱形布局，左右两条路在中段可互相切换
            {{0,0}, {2,1}, {2,3}, {4,0}, {4,4}, {6,2}, {8,1}, {8,3}, {10,2}}
        };
    }

    // ---------- 样式4：3×3网格 ----------
    // 9个节点：从左上到右下，网格状，多路径
    inline MapTemplate getGrid3x3() {
        return {
            9,
            {{0,1}, {0,3}, {1,2}, {1,4}, {2,5},
             {3,4}, {3,6}, {4,5}, {4,7}, {5,8},
             {6,7}, {7,8}},
            0, 8,
            // 等距网格，方便显示水平和垂直线
            {{0,0}, {2,0}, {4,0}, {0,2}, {2,2}, {4,2}, {0,4}, {2,4}, {4,4}}
        };
    }

    // ---------- 根据楼层选择模板池 ----------
    inline std::vector<MapTemplate> getPoolForFloor(int floor) {
        if (floor <= 2) {
            return { getStraightBranch(), getDoubleTree() };
        }
        else if (floor <= 4) {
            return { getDoubleTree(), getDiamondLoop() };
        }
        else {
            return { getDiamondLoop(), getGrid3x3() };
        }
    }

} // namespace MapTemplates
