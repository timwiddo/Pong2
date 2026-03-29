#include <raylib.h>

#include "game/Game.h"

namespace config {
constexpr int screenWidth = 1920;
constexpr int screenHeight = 1080;
constexpr int targetFps = 144;
}  // namespace config

int main() {
    InitWindow(config::screenWidth, config::screenHeight, "Pong - Raylib");
    SetTargetFPS(config::targetFps);
    SetRandomSeed(static_cast<unsigned int>(GetTime()));

    Game game(config::screenWidth, config::screenHeight);
    game.Run();

    CloseWindow();
    return 0;
}
