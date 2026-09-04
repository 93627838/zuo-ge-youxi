#include "FloorSource.h"
#include "FloorBuilder.h"
#include "FloorGenerator.h"
#include <stdexcept>

namespace FloorSource {

namespace {

LoadedFloor loadFixed(int floor) {
    LoadedFloor result;
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

    return result;
}

LoadedFloor loadGenerated(int floor, unsigned int seed) {
    // 楼层号继续增长，但文本主题循环前面 1~6 层
    int theme_floor = ((floor - 8) % 6) + 1;

    // 固定种子下每层也不完全相同
    unsigned int effective_seed =
        (seed == 0) ? 0 : seed + static_cast<unsigned int>(floor);

    FloorGenerator generator;
    auto generated = generator.generateFloor(floor, effective_seed, theme_floor);

    LoadedFloor result;
    result.rooms = std::move(generated.rooms);
    result.tmpl = std::move(generated.tmpl);
    return result;
}

} // namespace

LoadedFloor load(int floor, GameMode mode, unsigned int seed) {
    if (mode == GameMode::Story || floor <= 7)
        return loadFixed(floor);
    return loadGenerated(floor, seed);
}

} // namespace FloorSource
