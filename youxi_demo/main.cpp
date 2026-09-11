#include "Game.h"
#include <cstdlib>
#include <ctime>

#ifdef _WIN32
#include <windows.h>
#endif

int main() {
#ifdef _WIN32
    // 让 Windows 控制台按 UTF-8 输出,中文不乱码
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    Game game;
    game.Run();
    return 0;
}
