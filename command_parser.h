#pragma once

#include <string>
#include <vector>
#include <unordered_map>

class CommandParser {
public:
    struct Command {
        std::string type;
        std::vector<std::string> args;
    };

    static CommandParser& getInstance();

    Command parse(const std::string& input) const;

    CommandParser(const CommandParser&) = delete;
    CommandParser& operator=(const CommandParser&) = delete;

private:
    CommandParser();

    std::unordered_map<std::string, std::string> keywords_;
    std::unordered_map<std::string, std::string> directions_;
};
