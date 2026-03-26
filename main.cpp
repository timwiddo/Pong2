#include <raylib.h>

#include "game/Game.h"

namespace config {
constexpr int kScreenWidth = 1920;
constexpr int kScreenHeight = 1080;
constexpr int kTargetFps = 120;
}  // namespace config

int main() {
    InitWindow(config::kScreenWidth, config::kScreenHeight, "Pong - Raylib");
    SetTargetFPS(config::kTargetFps);
    SetRandomSeed(static_cast<unsigned int>(GetTime()));

    Game game(config::kScreenWidth, config::kScreenHeight);
    game.Run();

    CloseWindow();
    return 0;
}
