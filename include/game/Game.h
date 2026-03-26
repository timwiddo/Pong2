#pragma once

#include "entities/Ball.h"
#include "entities/Paddle.h"
#include "ui/ScoreBoard.h"

enum class GameState {
    Playing,
    GameOver
};

class Game {
public:
    Game(int screenWidth, int screenHeight);

    void Run();

private:
    void Update(float deltaTime);
    void UpdatePlaying(float deltaTime);
    void TryRestart();
    void Draw() const;

    int screenWidth_{};
    int screenHeight_{};

    Paddle player_;
    Paddle cpu_;
    Ball ball_;
    ScoreBoard scoreBoard_;
    GameState gameState_{GameState::Playing};
};

