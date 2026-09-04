# 逆神者之塔 · youxi_demo

一个用 C++20 开发的**控制台肉鸽（Roguelike）爬塔游戏**。

> 你不是英雄，你是被塔"选中"的倒霉蛋。一块神骸碎片刺入身体，不爬塔就会在三个月内被吞噬成半神半尸的怪物。
> 登上的每一层都对应一位主神的一部分躯体与权柄，逐层挑战战斗、精英、事件与宝藏房间，击败每层的 BOSS 后方能上楼。
> 直到第七层「空王座」——那里坐着的，是"如果从一开始就选择另一条路"的你。

## 🎮 当前实现（双模式 · v2）

游戏启动后**先选择模式**，再输入地图种子（`0` 为随机）：

- **故事模式**：走完整的 1~7 层固定剧情（`FloorBuilder` 手写房间，从灰烬平原到空王座），第 7 层击败最终 BOSS 后进入 **空王座结局**——封神 / 碎塔 / 永囚 三选一。
- **无尽模式**：前 7 层复用固定剧情文本，第 8 层起由 `FloorGenerator` 无限随机生成，并循环复用前 6 层的主题，可反复刷高层数。

楼层内部为 **Slay the Spire 式节点地图**：起点 → 分支路径（战斗 / 事件 / 宝箱 / 商店 / 精英）→ BOSS，路径间可自由选择走向。

## 🧩 架构（`youxi_demo/`）

| 文件 | 职责 |
|------|------|
| `Room.h` | 房间数据与奖励结构、房间类型枚举（起点/战斗/精英/首领/宝藏/事件/商店） |
| `MapTemplates.h` | 4 种地图拓扑模板（直链分支 / 双叉树 / 菱形环 / 3×3 网格）+ 按层选文本池 |
| `GameMode.h` | 游戏模式枚举：`Story`（故事）/ `Endless`（无尽） |
| `FloorSource.h/.cpp` | ★楼层来源决策层：按"模式 + 楼层"决定用固定剧情还是随机生成 |
| `FloorBuilder.h/.cpp` | 故事模式的 1~7 层手写世界观房间（含空王座） |
| `FloorGenerator.h/.cpp` | 无尽模式第 8 层起的程序化生成：抽模板 → 分配节点类型 → 填名字/描述/奖励 |
| `FloorManager.h/.cpp` | 楼层管家：加载楼层、玩家导航、BOSS 通关判定、上楼、`drawMap` 绘图 |
| `main.cpp` | 入口：选模式 + 种子 → 主循环 → 故事模式空王座结局 |

**楼层加载规则**（`FloorSource::load`）：`Story` 模式或楼层 ≤ 7 → 固定剧情；`Endless` 且楼层 ≥ 8 → 随机生成并循环主题。

## 🛠 环境与运行

- **语言/标准**：C++20（`stdcpp20`）
- **工具链**：Visual Studio 18（`v145` / PlatformToolset），或任意支持 C++20 的编译器（g++/clang 亦可）
- **打开方式**：用 Visual Studio 打开根目录的 `youxi_demo.slnx`，选 `x64` / `Debug` 生成并运行。
- **命令行编译（MSVC 示例）**：
  ```bat
  cl /std:c++20 /EHsc youxi_demo\main.cpp youxi_demo\FloorManager.cpp youxi_demo\FloorBuilder.cpp youxi_demo\FloorGenerator.cpp youxi_demo\FloorSource.cpp
  ```

### 操作指令（游戏内）
- 启动时：数字 `1` 选故事模式、`2` 选无尽模式；随后输入种子
- 游戏中：数字 `1` `2` … 选择可走的路径
- `map` —— 查看当前楼层地图（`@` 为玩家，颜色区分房间类型）
- `up` —— 击败 BOSS 后上楼
- `quit` —— 退出

## 📁 目录结构

```
youxi_demo/
├── youxi_demo/                 # 游戏源码（14 个文件）
│   ├── main.cpp
│   ├── Room.h
│   ├── MapTemplates.h
│   ├── GameMode.h
│   ├── FloorSource.h / .cpp
│   ├── FloorGenerator.h / .cpp
│   ├── FloorManager.h / .cpp
│   └── FloorBuilder.h / .cpp
├── youxi_demo.slnx             # Visual Studio 解决方案入口
└── README.md
```

## 🚧 规划（Roadmap）

- [x] 地图系统骨架（模板拓扑 + 程序化生成 + ASCII 地图绘制）
- [x] **双模式**：故事模式（固定 1~7 层 + 空王座结局） / 无尽模式（8 层起随机循环）
- [ ] 玩家属性（生命/金币/经验）与全局状态管理
- [ ] 真实战斗 / 事件 / 商店交互（目前房间完成即发奖，为占位）
- [ ] 神骸系统（植入 / 熔炼 / 拒绝）+ 记忆残响技能
- [ ] 存档与读档、遗产生态（登塔者残影）

> 项目为小组合作：本仓库分支整理后以 `main` 为主干；其中地图 / 房间设计与基础框架由我负责，战斗等其余系统由队友协作完成。
