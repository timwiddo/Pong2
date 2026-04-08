#pragma once

#include <array>

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
    void UpdateShop();
    void UpdateSettings();
    void UpdatePlaceholderScreen();
    void TryRestart();
    void StartNewRound();
    void ReturnToMainMenu();
    void Draw() const;
    void DrawMainMenu() const;
    void DrawGameplay() const;
    void DrawPausedOverlay() const;
    void DrawShop() const;
    void DrawSettings() const;
    void DrawPlaceholderScreen(const char* title) const;
    void DrawCoinsHud() const;
    void LoadCoinsFromFile();
    void SaveCoinsToFile() const;
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
    int coins_{};

    // Shop state
    int shopCategory_{};
    int shopItemSel_{};

    // Active skin indices
    int activeFieldSkin_{};
    int activePaddleSkin_{};
    int activeBallSkin_{};

    // Ownership: first skin of each category is free and always owned
    std::array<bool, 4> fieldOwned_{true, false, false, false};
    std::array<bool, 4> paddleOwned_{true, false, false, false};
    std::array<bool, 4> ballOwned_{true, false, false, false};
};

