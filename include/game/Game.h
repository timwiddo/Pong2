#pragma once

#include "entities/Ball.h"
#include "entities/Paddle.h"
#include "ui/ScoreBoard.h"

enum class GameState {
    MainMenu,
    Playing,
    Paused,
    Shop,
    Settings,
    GameOver
};

class Game {
public:
    Game(int screenWidth, int screenHeight);

    void Run();

private:
    void Update(float deltaTime);
    void UpdateMainMenu();
    void UpdatePlaying(float deltaTime);
    void UpdatePaused();
    void UpdateSettings();
    void UpdatePlaceholderScreen();
    void TryRestart();
    void StartNewRound();
    void ReturnToMainMenu();
    void Draw() const;
    void DrawMainMenu() const;
    void DrawGameplay() const;
    void DrawPausedOverlay() const;
    void DrawSettings() const;
    void DrawPlaceholderScreen(const char* title) const;
    [[nodiscard]] Vector2 ScreenCenter() const;
    void ServeFromCenter(float horizontalDirection);
    void CenterPaddles();
    void UpdatePlayerPaddle(float deltaTime);
    void UpdateCpuPaddle(float deltaTime);
    void HandleBallCollisions();
    void HandleScoring();

    int screenWidth_{};
    int screenHeight_{};

    float ballRadius_{20.0F};
    float paddleHeight_{150.0F};

    Paddle playerPaddle_;
    Paddle cpuPaddle_;
    Ball pongBall_;
    ScoreBoard scoreBoard_;
    GameState gameState_{GameState::MainMenu};
    int mainMenuSelection_{};
    int pauseSelection_{};
    int settingsSelection_{};
};

