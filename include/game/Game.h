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
    [[nodiscard]] Vector2 ScreenCenter() const;
    void ServeFromCenter(float horizontalDirection);
    void CenterPaddles();
    void UpdatePlayerPaddle(float deltaTime);
    void UpdateCpuPaddle(float deltaTime);
    void HandleBallCollisions();
    void HandleScoring();

    int screenWidth_{};
    int screenHeight_{};

    Paddle playerPaddle_;
    Paddle cpuPaddle_;
    Ball pongBall_;
    ScoreBoard scoreBoard_;
    GameState gameState_{GameState::Playing};
};

