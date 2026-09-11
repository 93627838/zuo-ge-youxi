#include "console_ui.h"

#include <iostream>

#ifdef _WIN32
#include <windows.h>
#endif

namespace {
constexpr int kWidth = 80;
}

void ConsoleUI::setColor(Foreground color) {
#ifdef _WIN32
    WORD attr = 0;
    switch (color) {
        case RED:    attr = FOREGROUND_RED; break;
        case GREEN:  attr = FOREGROUND_GREEN; break;
        case BLUE:   attr = FOREGROUND_BLUE; break;
        case YELLOW: attr = FOREGROUND_RED | FOREGROUND_GREEN; break;
        case WHITE:  attr = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE; break;
        case GRAY:   attr = FOREGROUND_INTENSITY; break;
    }
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), attr);
#else
    switch (color) {
        case RED:    std::cout << "\033[1;31m"; break;
        case GREEN:  std::cout << "\033[1;32m"; break;
        case BLUE:   std::cout << "\033[1;34m"; break;
        case YELLOW: std::cout << "\033[1;33m"; break;
        case WHITE:  std::cout << "\033[1;37m"; break;
        case GRAY:   std::cout << "\033[1;30m"; break;
    }
#endif
}

void ConsoleUI::printHeader(const std::string& text) {
    printDivider();
    printCenter(text);
    printDivider();
}

void ConsoleUI::printDivider(char ch) {
    std::cout << std::string(kWidth, ch) << std::endl;
}

void ConsoleUI::printCenter(const std::string& text) {
    int pad = (kWidth - static_cast<int>(text.size())) / 2;
    if (pad < 0) {
        pad = 0;
    }
    std::cout << std::string(pad, ' ') << text << std::endl;
}

void ConsoleUI::printStatus(const std::string& key, const std::string& value) {
    std::cout << key << ": " << value << std::endl;
}