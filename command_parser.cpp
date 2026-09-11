#include "command_parser.h"

#include <sstream>
#include <cctype>

namespace {

std::string toLower(const std::string& s) {
    std::string out = s;
    for (char& c : out) {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return out;
}

std::vector<std::string> split(const std::string& s) {
    std::vector<std::string> tokens;
    std::istringstream iss(s);
    std::string token;
    while (iss >> token) {
        tokens.push_back(token);
    }
    return tokens;
}

}  // namespace

CommandParser& CommandParser::getInstance() {
    static CommandParser instance;
    return instance;
}

CommandParser::CommandParser() {
    keywords_ = {
        {"攻击", "attack"},
        {"attack", "attack"},
        {"使用", "use"},
        {"use", "use"},
        {"查看状态", "status"},
        {"status", "status"},
        {"保存", "save"},
        {"save", "save"},
        {"读取", "load"},
        {"load", "load"},
    };

    directions_ = {
        {"向北", "north"}, {"北", "north"}, {"north", "north"}, {"n", "north"},
        {"向南", "south"}, {"南", "south"}, {"south", "south"}, {"s", "south"},
        {"向东", "east"},  {"东", "east"},  {"east", "east"},   {"e", "east"},
        {"向西", "west"},  {"西", "west"},  {"west", "west"},   {"w", "west"},
    };
}

CommandParser::Command CommandParser::parse(const std::string& input) const {
    Command cmd;

    std::string lower = toLower(input);
    std::vector<std::string> tokens = split(lower);

    if (tokens.empty()) {
        return cmd;
    }

    const std::string& keyword = tokens[0];

    auto it = keywords_.find(keyword);
    if (it != keywords_.end()) {
        cmd.type = it->second;
        if (cmd.type == "attack" && tokens.size() > 1) {
            cmd.args.push_back(tokens[1]);
        } else if (cmd.type == "use" && tokens.size() > 1) {
            cmd.args.push_back(tokens[1]);
        }
        return cmd;
    }

    auto dit = directions_.find(keyword);
    if (dit != directions_.end()) {
        cmd.type = "move";
        cmd.args.push_back(dit->second);
    }

    return cmd;
}
