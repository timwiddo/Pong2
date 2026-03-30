#include "game/Game.h"

#include <array>
#include <cmath>

namespace {
constexpr int targetScore = 7;
constexpr float playerSpeed = 520.0F;
constexpr float cpuSpeed = 460.0F;
constexpr float ballBaseSpeed = 680.0F;
constexpr float paddleHeight = 150.0F;
constexpr float paddleWidth = 28.0F;
constexpr float ballRadius = 20.0F;
constexpr float cpuDeadZone = 10.0F;
constexpr float paddleMargin = 40.0F;

constexpr int mainMenuOptionCount = 3;
constexpr int pauseOptionCount = 2;

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
      playerPaddle_(Rectangle{paddleMargin, screenHeight * 0.5F - paddleHeight * 0.5F, paddleWidth, paddleHeight}, playerSpeed),
      cpuPaddle_(Rectangle{screenWidth - (paddleMargin + paddleWidth), screenHeight * 0.5F - paddleHeight * 0.5F, paddleWidth, paddleHeight}, cpuSpeed),
      pongBall_(Vector2{screenWidth * 0.5F, screenHeight * 0.5F}, ballRadius, ballBaseSpeed),
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
            if (IsKeyPressed(KEY_ESCAPE)) {
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
        case GameState::Settings:
            UpdatePlaceholderScreen();
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
        gameState_ = GameState::GameOver;
    }
}

void Game::UpdatePaused() {
    if (IsKeyPressed(KEY_ESCAPE)) {
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
            DrawPlaceholderScreen("Settings");
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
    DrawText("Esc to pause", 28, screenHeight_ - 42, 20, Color{160, 160, 170, 255});
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

    DrawText("Esc resumes", screenWidth_ / 2 - 70, screenHeight_ / 2 + 192, 20, Color{180, 180, 190, 255});
}

void Game::DrawPlaceholderScreen(const char* title) const {
    DrawText(title, screenWidth_ / 2 - 120, 220, 74, RAYWHITE);
    DrawText("Coming soon", screenWidth_ / 2 - 110, screenHeight_ / 2 - 12, 44, Color{190, 190, 210, 255});
    DrawText("Press Esc, Enter, or click to return", screenWidth_ / 2 - 210, screenHeight_ - 100, 26,
             Color{155, 155, 172, 255});
}

Vector2 Game::ScreenCenter() const {
    return Vector2{screenWidth_ * 0.5F, screenHeight_ * 0.5F};
}

void Game::ServeFromCenter(float horizontalDirection) {
    pongBall_.Reset(ScreenCenter(), horizontalDirection);
}

void Game::CenterPaddles() {
    const float centeredPaddleY = screenHeight_ * 0.5F - paddleHeight * 0.5F;
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
