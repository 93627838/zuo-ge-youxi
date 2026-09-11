#pragma once

#include <string>

class ConsoleUI {
public:
    enum Foreground {
        RED,
        GREEN,
        BLUE,
        YELLOW,
        WHITE,
        GRAY
    };

    static void setColor(Foreground color);

    static void printHeader(const std::string& text);

    static void printDivider(char ch = '=');

    static void printCenter(const std::string& text);

    static void printStatus(const std::string& key, const std::string& value);
};
