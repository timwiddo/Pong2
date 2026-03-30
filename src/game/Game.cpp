#include "game/Game.h"

#include <algorithm>
#include <array>
#include <cmath>

namespace {
constexpr int targetScore = 3;
constexpr int winCoins = 10;
constexpr int lossCoins = 3;
constexpr float playerSpeed = 520.0F;
constexpr float cpuSpeed = 460.0F;
constexpr float ballBaseSpeed = 680.0F;
constexpr float defaultPaddleHeight = 150.0F;
constexpr float paddleWidth = 28.0F;
constexpr float defaultBallRadius = 20.0F;
constexpr float cpuDeadZone = 10.0F;
constexpr float paddleMargin = 40.0F;

constexpr float minBallRadius = 8.0F;
constexpr float maxBallRadius = 34.0F;
constexpr float ballRadiusStep = 2.0F;
constexpr float minPaddleHeight = 80.0F;
constexpr float maxPaddleHeight = 230.0F;
constexpr float paddleHeightStep = 10.0F;

constexpr int mainMenuOptionCount = 3;
constexpr int pauseOptionCount = 2;
constexpr int settingsOptionCount = 3;

Rectangle MainMenuButtonBounds(const int index, const int screenWidth, const int screenHeight) {
    constexpr float buttonWidth = 340.0F;
    constexpr float buttonHeight = 70.0F;
    constexpr float spacing = 18.0F;

    const float totalHeight = mainMenuOptionCount * buttonHeight + (mainMenuOptionCount - 1) * spacing;
    const float startY = screenHeight * 0.5F - totalHeight * 0.5F;
    const float x = screenWidth * 0.5F - buttonWidth * 0.5F;
    const float y = startY + index * (buttonHeight + spacing);
    return Rectangle{x, y, buttonWidth, buttonHeight};
}

Rectangle PauseButtonBounds(const int index, const int screenWidth, const int screenHeight) {
    constexpr float buttonWidth = 300.0F;
    constexpr float buttonHeight = 62.0F;
    constexpr float spacing = 14.0F;

    const float totalHeight = pauseOptionCount * buttonHeight + (pauseOptionCount - 1) * spacing;
    const float startY = screenHeight * 0.5F - totalHeight * 0.5F + 40.0F;
    const float x = screenWidth * 0.5F - buttonWidth * 0.5F;
    const float y = startY + index * (buttonHeight + spacing);
    return Rectangle{x, y, buttonWidth, buttonHeight};
}
}  // namespace

Game::Game(const int screenWidth, const int screenHeight)
    : screenWidth_(screenWidth),
      screenHeight_(screenHeight),
      ballRadius_(defaultBallRadius),
      paddleHeight_(defaultPaddleHeight),
      playerPaddle_(Rectangle{paddleMargin, screenHeight * 0.5F - paddleHeight_ * 0.5F, paddleWidth, paddleHeight_}, playerSpeed),
      cpuPaddle_(Rectangle{screenWidth - (paddleMargin + paddleWidth), screenHeight * 0.5F - paddleHeight_ * 0.5F, paddleWidth, paddleHeight_}, cpuSpeed),
      pongBall_(Vector2{screenWidth * 0.5F, screenHeight * 0.5F}, ballRadius_, ballBaseSpeed),
      scoreBoard_(targetScore) {
    CenterPaddles();
    ServeFromCenter((GetRandomValue(0, 1) == 0) ? -1.0F : 1.0F);
}

void Game::Run() {
    while (!WindowShouldClose()) {
        Update(GetFrameTime());

        BeginDrawing();
        Draw();
        EndDrawing();
    }
}

void Game::Update(float deltaTime) {
    switch (gameState_) {
        case GameState::MainMenu:
            UpdateMainMenu();
            break;
        case GameState::Playing:
            if (IsKeyPressed(KEY_P)) {
                pauseSelection_ = 0;
                gameState_ = GameState::Paused;
                break;
            }
            UpdatePlaying(deltaTime);
            break;
        case GameState::Paused:
            UpdatePaused();
            break;
        case GameState::Shop:
            UpdatePlaceholderScreen();
            break;
        case GameState::Settings:
            UpdateSettings();
            break;
        case GameState::GameOver:
            TryRestart();
            break;
    }
}

void Game::UpdateMainMenu() {
    if (IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP)) {
        mainMenuSelection_ = (mainMenuSelection_ - 1 + mainMenuOptionCount) % mainMenuOptionCount;
    }
    if (IsKeyPressed(KEY_S) || IsKeyPressed(KEY_DOWN)) {
        mainMenuSelection_ = (mainMenuSelection_ + 1) % mainMenuOptionCount;
    }

    const Vector2 mousePosition = GetMousePosition();
    for (int index = 0; index < mainMenuOptionCount; ++index) {
        if (CheckCollisionPointRec(mousePosition, MainMenuButtonBounds(index, screenWidth_, screenHeight_))) {
            mainMenuSelection_ = index;
        }
    }

    if (!IsKeyPressed(KEY_ENTER) && !IsKeyPressed(KEY_SPACE) && !IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        return;
    }

    if (mainMenuSelection_ == 0) {
        StartNewRound();
    } else if (mainMenuSelection_ == 1) {
        gameState_ = GameState::Shop;
    } else {
        settingsSelection_ = 0;
        gameState_ = GameState::Settings;
    }
}

void Game::UpdatePlaying(float deltaTime) {
    UpdatePlayerPaddle(deltaTime);
    UpdateCpuPaddle(deltaTime);

    pongBall_.Update(deltaTime);
    HandleBallCollisions();
    HandleScoring();

    if (scoreBoard_.HasWinner()) {
        coins_ += scoreBoard_.PlayerWon() ? winCoins : lossCoins;
        gameState_ = GameState::GameOver;
    }
}

void Game::UpdatePaused() {
    if (IsKeyPressed(KEY_P)) {
        gameState_ = GameState::Playing;
        return;
    }

    if (IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP)) {
        pauseSelection_ = (pauseSelection_ - 1 + pauseOptionCount) % pauseOptionCount;
    }
    if (IsKeyPressed(KEY_S) || IsKeyPressed(KEY_DOWN)) {
        pauseSelection_ = (pauseSelection_ + 1) % pauseOptionCount;
    }

    const Vector2 mousePosition = GetMousePosition();
    for (int index = 0; index < pauseOptionCount; ++index) {
        if (CheckCollisionPointRec(mousePosition, PauseButtonBounds(index, screenWidth_, screenHeight_))) {
            pauseSelection_ = index;
        }
    }

    if (!IsKeyPressed(KEY_ENTER) && !IsKeyPressed(KEY_SPACE) && !IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        return;
    }

    if (pauseSelection_ == 0) {
        gameState_ = GameState::Playing;
    } else {
        ReturnToMainMenu();
    }
}

void Game::UpdateSettings() {
    if (IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP)) {
        settingsSelection_ = (settingsSelection_ - 1 + settingsOptionCount) % settingsOptionCount;
    }
    if (IsKeyPressed(KEY_S) || IsKeyPressed(KEY_DOWN)) {
        settingsSelection_ = (settingsSelection_ + 1) % settingsOptionCount;
    }

    float delta = 0.0F;
    if (IsKeyPressed(KEY_A) || IsKeyPressed(KEY_LEFT)) {
        delta = -1.0F;
    } else if (IsKeyPressed(KEY_D) || IsKeyPressed(KEY_RIGHT)) {
        delta = 1.0F;
    }

    if (delta != 0.0F) {
        if (settingsSelection_ == 0) {
            ballRadius_ = std::clamp(ballRadius_ + delta * ballRadiusStep, minBallRadius, maxBallRadius);
            pongBall_.SetRadius(ballRadius_);
        } else if (settingsSelection_ == 1) {
            paddleHeight_ = std::clamp(paddleHeight_ + delta * paddleHeightStep, minPaddleHeight, maxPaddleHeight);
            playerPaddle_.SetHeight(paddleHeight_, screenHeight_);
            cpuPaddle_.SetHeight(paddleHeight_, screenHeight_);
            CenterPaddles();
        } else {
            ballRadius_ = defaultBallRadius;
            paddleHeight_ = defaultPaddleHeight;
            pongBall_.SetRadius(ballRadius_);
            playerPaddle_.SetHeight(paddleHeight_, screenHeight_);
            cpuPaddle_.SetHeight(paddleHeight_, screenHeight_);
            CenterPaddles();
        }
    }

    if ((settingsSelection_ == 2) && (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE))) {
        ballRadius_ = defaultBallRadius;
        paddleHeight_ = defaultPaddleHeight;
        pongBall_.SetRadius(ballRadius_);
        playerPaddle_.SetHeight(paddleHeight_, screenHeight_);
        cpuPaddle_.SetHeight(paddleHeight_, screenHeight_);
        CenterPaddles();
    }

    if (IsKeyPressed(KEY_M)) {
        ReturnToMainMenu();
    }
}

void Game::UpdatePlaceholderScreen() {
    if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_ENTER) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        ReturnToMainMenu();
    }
}

void Game::TryRestart() {
    if (IsKeyPressed(KEY_R)) {
        StartNewRound();
        return;
    }

    if (IsKeyPressed(KEY_M) || IsKeyPressed(KEY_ESCAPE)) {
        ReturnToMainMenu();
    }
}

void Game::StartNewRound() {
    scoreBoard_.Reset();
    gameState_ = GameState::Playing;
    pongBall_.SetRadius(ballRadius_);
    playerPaddle_.SetHeight(paddleHeight_, screenHeight_);
    cpuPaddle_.SetHeight(paddleHeight_, screenHeight_);
    CenterPaddles();
    ServeFromCenter((GetRandomValue(0, 1) == 0) ? -1.0F : 1.0F);
}

void Game::ReturnToMainMenu() {
    gameState_ = GameState::MainMenu;
    mainMenuSelection_ = 0;
    pauseSelection_ = 0;
}

void Game::Draw() const {
    ClearBackground(Color{18, 18, 24, 255});

    switch (gameState_) {
        case GameState::MainMenu:
            DrawMainMenu();
            break;
        case GameState::Shop:
            DrawPlaceholderScreen("Shop");
            break;
        case GameState::Settings:
            DrawSettings();
            break;
        case GameState::Playing:
            DrawGameplay();
            break;
        case GameState::Paused:
            DrawGameplay();
            DrawPausedOverlay();
            break;
        case GameState::GameOver:
            DrawGameplay();
            DrawRectangle(0, 0, screenWidth_, screenHeight_, Fade(BLACK, 0.45F));
            DrawText(scoreBoard_.PlayerWon() ? "You Win!" : "Computer Wins!", screenWidth_ / 2 - 150, screenHeight_ / 2 - 50, 48, RAYWHITE);
            DrawText("Press R to play again", screenWidth_ / 2 - 135, screenHeight_ / 2 + 14, 24, Color{220, 220, 220, 255});
            DrawText("Press M or Esc for main menu", screenWidth_ / 2 - 168, screenHeight_ / 2 + 48, 22, Color{200, 200, 210, 255});
            break;
    }

    DrawCoinsHud();
}

void Game::DrawMainMenu() const {
    DrawText("PONG", screenWidth_ / 2 - 100, 130, 84, RAYWHITE);
    DrawText("Choose an option", screenWidth_ / 2 - 120, 230, 30, Color{185, 185, 200, 255});

    constexpr std::array<const char*, mainMenuOptionCount> options = {"Play", "Shop", "Settings"};

    for (int index = 0; index < mainMenuOptionCount; ++index) {
        const Rectangle bounds = MainMenuButtonBounds(index, screenWidth_, screenHeight_);
        const bool selected = index == mainMenuSelection_;

        DrawRectangleRec(bounds, selected ? Color{80, 120, 220, 255} : Color{45, 45, 60, 255});
        DrawRectangleLinesEx(bounds, 2.0F, selected ? RAYWHITE : Color{90, 90, 110, 255});

        DrawText(options[index], static_cast<int>(bounds.x) + 18, static_cast<int>(bounds.y) + 18, 34,
                 selected ? RAYWHITE : Color{220, 220, 230, 255});
    }

    DrawText("Use mouse or W/S (Up/Down), Enter to choose", screenWidth_ / 2 - 255, screenHeight_ - 90, 22, Color{165, 165, 180, 255});
}

void Game::DrawGameplay() const {
    DrawLine(screenWidth_ / 2, 0, screenWidth_ / 2, screenHeight_, Color{50, 50, 70, 255});
    playerPaddle_.Draw(Color{230, 230, 230, 255});
    cpuPaddle_.Draw(Color{230, 230, 230, 255});
    pongBall_.Draw(Color{235, 180, 80, 255});
    scoreBoard_.Draw(screenWidth_);

    DrawText("W/S or UP/DOWN to move", 28, screenHeight_ - 72, 20, Color{160, 160, 170, 255});
    DrawText("P to pause", 28, screenHeight_ - 42, 20, Color{160, 160, 170, 255});
}

void Game::DrawPausedOverlay() const {
    DrawRectangle(0, 0, screenWidth_, screenHeight_, Fade(BLACK, 0.55F));
    DrawText("Paused", screenWidth_ / 2 - 90, screenHeight_ / 2 - 110, 58, RAYWHITE);

    constexpr std::array<const char*, pauseOptionCount> options = {"Resume", "Main Menu"};
    for (int index = 0; index < pauseOptionCount; ++index) {
        const Rectangle bounds = PauseButtonBounds(index, screenWidth_, screenHeight_);
        const bool selected = index == pauseSelection_;

        DrawRectangleRec(bounds, selected ? Color{70, 110, 200, 255} : Color{48, 48, 62, 255});
        DrawRectangleLinesEx(bounds, 2.0F, selected ? RAYWHITE : Color{95, 95, 115, 255});
        DrawText(options[index], static_cast<int>(bounds.x) + 18, static_cast<int>(bounds.y) + 16, 32,
                 selected ? RAYWHITE : Color{220, 220, 230, 255});
    }

    DrawText("P resumes", screenWidth_ / 2 - 62, screenHeight_ / 2 + 192, 20, Color{180, 180, 190, 255});
}

void Game::DrawSettings() const {
    DrawText("Settings", screenWidth_ / 2 - 130, 105, 72, RAYWHITE);
    DrawText("Use W/S to select and A/D to change", screenWidth_ / 2 - 215, 200, 28, Color{185, 185, 205, 255});

    const bool ballSelected = settingsSelection_ == 0;
    const bool paddleSelected = settingsSelection_ == 1;
    const bool resetSelected = settingsSelection_ == 2;

    const Rectangle ballRow{screenWidth_ * 0.5F - 250.0F, 260.0F, 500.0F, 76.0F};
    DrawRectangleRec(ballRow, ballSelected ? Color{80, 120, 220, 255} : Color{45, 45, 60, 255});
    DrawRectangleLinesEx(ballRow, 2.0F, ballSelected ? RAYWHITE : Color{90, 90, 110, 255});
    DrawText(TextFormat("Ball Size: %.0f", ballRadius_), static_cast<int>(ballRow.x) + 20, static_cast<int>(ballRow.y) + 22, 34,
             ballSelected ? RAYWHITE : Color{220, 220, 230, 255});

    const Rectangle paddleRow{screenWidth_ * 0.5F - 250.0F, 358.0F, 500.0F, 76.0F};
    DrawRectangleRec(paddleRow, paddleSelected ? Color{80, 120, 220, 255} : Color{45, 45, 60, 255});
    DrawRectangleLinesEx(paddleRow, 2.0F, paddleSelected ? RAYWHITE : Color{90, 90, 110, 255});
    DrawText(TextFormat("Paddle Size: %.0f", paddleHeight_), static_cast<int>(paddleRow.x) + 20, static_cast<int>(paddleRow.y) + 22, 34,
             paddleSelected ? RAYWHITE : Color{220, 220, 230, 255});

    const Rectangle resetRow{screenWidth_ * 0.5F - 250.0F, 456.0F, 500.0F, 76.0F};
    DrawRectangleRec(resetRow, resetSelected ? Color{80, 120, 220, 255} : Color{45, 45, 60, 255});
    DrawRectangleLinesEx(resetRow, 2.0F, resetSelected ? RAYWHITE : Color{90, 90, 110, 255});
    DrawText("Reset to Default", static_cast<int>(resetRow.x) + 20, static_cast<int>(resetRow.y) + 22, 34,
             resetSelected ? RAYWHITE : Color{220, 220, 230, 255});

    DrawText(TextFormat("Ball range: %.0f - %.0f", minBallRadius, maxBallRadius), screenWidth_ / 2 - 145, 552, 24,
             Color{170, 170, 188, 255});
    DrawText(TextFormat("Paddle range: %.0f - %.0f", minPaddleHeight, maxPaddleHeight), screenWidth_ / 2 - 165, 585, 24,
             Color{170, 170, 188, 255});
    DrawText("Enter/Space on reset; M to go back", screenWidth_ / 2 - 205, screenHeight_ - 95, 24, Color{155, 155, 172, 255});
}

void Game::DrawPlaceholderScreen(const char* title) const {
    DrawText(title, screenWidth_ / 2 - 120, 220, 74, RAYWHITE);
    DrawText("Coming soon", screenWidth_ / 2 - 110, screenHeight_ / 2 - 12, 44, Color{190, 190, 210, 255});
    DrawText("Press Esc, Enter, or click to return", screenWidth_ / 2 - 210, screenHeight_ - 100, 26,
             Color{155, 155, 172, 255});
}

void Game::DrawCoinsHud() const {
    const int circleX = screenWidth_ - 135;
    const int circleY = 42;
    DrawCircle(circleX, circleY, 16.0F, Color{246, 212, 66, 255});
    DrawCircleLines(circleX, circleY, 16.0F, Color{255, 236, 120, 255});
    DrawText(TextFormat("%d", coins_), screenWidth_ - 105, 25, 34, Color{246, 212, 66, 255});
}

Vector2 Game::ScreenCenter() const {
    return Vector2{screenWidth_ * 0.5F, screenHeight_ * 0.5F};
}

void Game::ServeFromCenter(float horizontalDirection) {
    pongBall_.Reset(ScreenCenter(), horizontalDirection);
}

void Game::CenterPaddles() {
    const float centeredPaddleY = screenHeight_ * 0.5F - paddleHeight_ * 0.5F;
    playerPaddle_.SetY(centeredPaddleY);
    cpuPaddle_.SetY(centeredPaddleY);
}

void Game::UpdatePlayerPaddle(float deltaTime) {
    float playerMoveDirection = 0.0F;
    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) {
        playerMoveDirection -= 1.0F;
    }
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) {
        playerMoveDirection += 1.0F;
    }
    playerPaddle_.Move(playerMoveDirection, deltaTime, screenHeight_);
}

void Game::UpdateCpuPaddle(float deltaTime) {
    const float cpuTrackingOffset = pongBall_.Y() - cpuPaddle_.CenterY();
    if (std::fabs(cpuTrackingOffset) > cpuDeadZone) {
        cpuPaddle_.Move((cpuTrackingOffset > 0.0F) ? 1.0F : -1.0F, deltaTime, screenHeight_);
    }
}

void Game::HandleBallCollisions() {
    const Rectangle ballBounds = pongBall_.Bounds();

    if (ballBounds.y <= 0.0F) {
        pongBall_.BounceFromTop();
    } else if (ballBounds.y + ballBounds.height >= screenHeight_) {
        pongBall_.BounceFromBottom(static_cast<float>(screenHeight_));
    }

    if (CheckCollisionRecs(ballBounds, playerPaddle_.Bounds()) && pongBall_.VelocityX() < 0.0F) {
        pongBall_.BounceFromPaddle(playerPaddle_, true);
    } else if (CheckCollisionRecs(ballBounds, cpuPaddle_.Bounds()) && pongBall_.VelocityX() > 0.0F) {
        pongBall_.BounceFromPaddle(cpuPaddle_, false);
    }
}

void Game::HandleScoring() {
    if (pongBall_.IsOutLeft()) {
        scoreBoard_.CpuScored();
        ServeFromCenter(1.0F);
    } else if (pongBall_.IsOutRight(static_cast<float>(screenWidth_))) {
        scoreBoard_.PlayerScored();
        ServeFromCenter(-1.0F);
    }
}
