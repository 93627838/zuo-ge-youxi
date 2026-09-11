# 逆神者之塔

用 **C++20** 开发的控制台 **肉鸽爬塔 × 卡牌战斗** 游戏。当前版本：**v2.0 合并版**（tag `v2.0`）。

> 你不是英雄，你是被塔选中的倒霉蛋。一块神骸碎片刺入身体，不爬塔就会在三个月内被吞噬成半神半尸的怪物。
> 每层对应一位主神的躯体与权柄，逐层挑战战斗 / 精英 / 事件 / 宝藏 / 商店，击败该层 BOSS 方能上楼。
> 直到第七层「空王座」——那里坐着的，是"如果当初选择另一条路"的你。

## 📌 现状

地图系统与卡牌战斗系统已完成合并，**可完整通关**（故事 7 层 / 无尽无限层）。

- **双模式**：故事模式（1~7 层固定剧情 + 空王座三结局：封神 / 碎塔 / 永囚）；无尽模式（8 层起程序化生成，循环复用前 6 层主题）
- **爬塔地图**：Slay the Spire 式分支节点图，ASCII 拓扑渲染 + **战争迷雾**（未探索显示 `?`）
- **卡牌战斗**：回合制、每回合 3 能量、敌人**意图**系统、1 对多群怪、格挡 / 虚弱 / 易伤 / 暴击，以及「**登神共鸣**」（攒满进入强化形态）；HP / 金币 / 牌组 / 遗物 / 药水跨楼层保留
- **卡组构筑**：13 张卡（攻击 / 技能 / 能力）、抽 / 手 / 弃三堆、战斗奖励
- **敌人**：27 种敌人 + 27 个出场组合，按七层世界观命名
- **遗物 / 药水**：11 件遗物（事件钩子驱动）+ 8 种药水
- **商店**：买卡 / 删牌 / 药水 / 遗物

## 🧩 项目结构

源码位于 `youxi_demo/`，分层依赖（上层调用下层）：

```
main.cpp                          入口：选模式 + 种子 → 主循环
 │
 ├─ FloorManager                  单层引擎：导航 / 清房 / 上楼 / drawMap（含战争迷雾）
 │   └─ FloorSource               ★楼层来源决策：按「模式 + 楼层」分发
 │       ├─ FloorBuilder            故事模式 1~7 层手写房间
 │       └─ FloorGenerator          无尽模式 8 层+ 程序化生成
 │           └─ FloorData           统一楼层数据契约 rooms + tmpl
 │
 ├─ EncounterBuilder              楼层 ↔ 战斗桥接：按楼层 + 房间类型编组敌人并强化数值
 │   └─ Combat                     卡牌战斗引擎
 │       ├─ Card / Deck / Enemy / Player   卡牌 · 牌堆 · 敌人 · 玩家状态
 │       ├─ Data                            卡牌 / 敌人 / 出场组合数据表
 │       └─ Item                            遗物 / 药水（事件钩子）
 │
 ├─ shop                          商店：买卡 / 删牌 / 药水 / 遗物
 └─ UI                            控制台输入输出封装
```

| 层 | 文件 | 职责 |
|----|------|------|
| 入口 | `main.cpp` | 选模式 / 种子、主循环、空王座结局 |
| 引擎 | `FloorManager.h/.cpp` | 单层状态机：导航、清房、上楼、`drawMap` |
| 派发 | `FloorSource.h/.cpp` | 楼层来源决策（模式 + 楼层 → 来源） |
| 内容源 | `FloorBuilder` / `FloorGenerator` | 手写 1~7 层 / 程序化生成 |
| 数据 | `FloorData.h` · `MapTemplates.h` · `Room.h` · `GameMode.h` | 统一楼层数据契约、地图模板、房间、模式枚举 |
| 战斗桥接 | `EncounterBuilder.h/.cpp` | 楼层 + 房间类型 → 敌人编组 + 数值强化（平衡参数集中处） |
| 战斗 | `Combat` · `Card` · `Deck` · `Enemy` · `Player` · `Data` · `Item` | 卡牌战斗引擎与数据 |
| 商店 | `shop.h/.cpp` | 买卡 / 删牌 / 药水 / 遗物 |
| UI | `UI.h/.cpp` | 控制台输入输出封装（含 EOF 保护） |

工程共 **13 个 `.cpp` + 18 个头文件**（`Game.cpp/.h` 为早期测试入口，保留存档但**不参与编译**）。

## 🚧 进度

**已完成**
- [x] 地图系统：双模式（故事 / 无尽）+ 程序化生成 + ASCII 拓扑图
- [x] 架构优化：三套重复楼层结构统一为 `FloorData`；故事模式自动生成拓扑图；`drawMap` 与房间 ID 解耦
- [x] 卡牌战斗系统（回合 / 能量 / 意图 / 群怪 / 状态 / 登神共鸣）
- [x] 数据内容：27 敌人、13 卡、11 遗物、8 药水、商店
- [x] 地图显示优化：战争迷雾 + 去路房间描述

**进行中 / 待办**
- [ ] 数值平衡（敌人强度 / 金币产出 / 商店定价）
- [ ] 神骸系统（植入 / 熔炼 / 拒绝）+ 记忆残响技能
- [ ] 存档 / 读档、遗产生态（登塔者残影）
- [ ] 正式 UI 模块（当前为按调用点反推的兼容实现）

## 🛠 构建与运行

- **语言**：C++20　**工具链**：Visual Studio 18（`v145`）
- **打开方式**：用 VS 打开根目录 `youxi_demo.slnx`，选 `x64` / `Debug` 生成运行
- **命令行（MSVC 示例）**：
  ```bat
  cl /std:c++20 /EHsc youxi_demo\main.cpp youxi_demo\FloorBuilder.cpp youxi_demo\FloorGenerator.cpp ^
     youxi_demo\FloorSource.cpp youxi_demo\FloorManager.cpp youxi_demo\Combat.cpp youxi_demo\Data.cpp ^
     youxi_demo\Deck.cpp youxi_demo\Item.cpp youxi_demo\Player.cpp youxi_demo\shop.cpp ^
     youxi_demo\EncounterBuilder.cpp youxi_demo\UI.cpp
  ```
- ⚠️ **不要加 `/utf-8`**：源码为 GBK 与 UTF-8(BOM) 混排，靠 BOM 各自解码；加 `/utf-8` 会把 GBK 文件读坏。控制台保持默认代码页 936，中文即正常。

## 🎮 操作

- 启动：`1` 故事模式 / `2` 无尽模式 → 输入地图种子（`0` 随机）
- 移动：输入去路编号（命令大小写均可）
- `map` 查看当前楼层地图（`@` 为你，`?` 为未探索）｜ `up` 上楼（需击败 BOSS）｜ `quit` 退出
- 战斗：输入手牌编号出牌 / `0` 结束回合

## 👥 分工

- 地图 / 房间设计 + 基础框架：本项目主
- 卡牌战斗系统：队友 zhao（SimpleMonkey）
- 地图显示优化（战争迷雾等）：队友 SimpleMonkey

> 仓库以 `main` 为**唯一主干**。
