#include "UI.h"
#include <iostream>
#include <string>

// 输入统一走 getline:每次交互恰好消耗一整行,
// 这样战斗系统与爬塔主循环共用同一个 cin 时不会互相残留换行。

void UI::Clear() {
    // ANSI 清屏指令,与爬塔主循环里的清屏方式保持一致
    std::cout << "\033[2J\033[2;1H";
}

void UI::Print(const std::string& text) {
    std::cout << text << "\n";
}

void UI::Pause() {
    std::cout << "\n(按 Enter 继续)";
    std::string dummy;
    std::getline(std::cin, dummy);
}

int UI::GetInt() {
    std::string line;
    std::getline(std::cin, line);
    try {
        return std::stoi(line);
    }
    catch (...) {
        return 0;   // 非数字输入一律当作 0(等价于取消 / 返回上一级)
    }
}

std::string UI::GetLine() {
    std::string line;
    if (!std::getline(std::cin, line)) {
        // 输入流已结束(EOF:重定向输入读完 / 管道中断等)。
        // 这里返回 "0" —— 对战斗而言等价于"结束回合",
        // 让 while(true) 能继续推进而不是拿到空串后无限空转。
        return "0";
    }
    return line;
}
