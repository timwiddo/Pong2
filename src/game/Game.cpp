#include "game/Game.h"

#include <cmath>

namespace {
constexpr int kTargetScore = 7;
constexpr float kPlayerSpeed = 520.0F;
constexpr float kCpuSpeed = 460.0F;
constexpr float kBallBaseSpeed = 520.0F;
constexpr float kPaddleHeight = 150.0F;
constexpr float kPaddleWidth = 28.0F;
constexpr float kBallRadius = 16.0F;
constexpr float kCpuDeadZone = 10.0F;
constexpr float kPaddleMargin = 40.0F;
}  // namespace

Game::Game(const int screenWidth, const int screenHeight)
    : screenWidth_(screenWidth),
      screenHeight_(screenHeight),
      player_(Rectangle{kPaddleMargin, screenHeight * 0.5F - kPaddleHeight * 0.5F, kPaddleWidth, kPaddleHeight}, kPlayerSpeed),
      cpu_(Rectangle{screenWidth - (kPaddleMargin + kPaddleWidth), screenHeight * 0.5F - kPaddleHeight * 0.5F, kPaddleWidth, kPaddleHeight}, kCpuSpeed),
      ball_(Vector2{screenWidth * 0.5F, screenHeight * 0.5F}, kBallRadius, kBallBaseSpeed),
      scoreBoard_(kTargetScore) {
    ball_.Reset(Vector2{screenWidth_ * 0.5F, screenHeight_ * 0.5F}, (GetRandomValue(0, 1) == 0) ? -1.0F : 1.0F);
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
    if (gameState_ == GameState::Playing) {
        UpdatePlaying(deltaTime);
    } else {
        TryRestart();
    }
}

void Game::UpdatePlaying(float deltaTime) {
    float playerDirection = 0.0F;
    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) {
        playerDirection -= 1.0F;
    }
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) {
        playerDirection += 1.0F;
    }
    player_.Move(playerDirection, deltaTime, screenHeight_);

    const float cpuOffset = ball_.Y() - cpu_.CenterY();
    if (std::fabs(cpuOffset) > kCpuDeadZone) {
        cpu_.Move((cpuOffset > 0.0F) ? 1.0F : -1.0F, deltaTime, screenHeight_);
    }

    ball_.Update(deltaTime);

    if (ball_.Bounds().y <= 0.0F) {
        ball_.BounceFromTop();
    } else if (ball_.Bounds().y + ball_.Bounds().height >= screenHeight_) {
        ball_.BounceFromBottom(static_cast<float>(screenHeight_));
    }

    if (CheckCollisionRecs(ball_.Bounds(), player_.Bounds()) && ball_.VelocityX() < 0.0F) {
        ball_.BounceFromPaddle(player_, true);
    } else if (CheckCollisionRecs(ball_.Bounds(), cpu_.Bounds()) && ball_.VelocityX() > 0.0F) {
        ball_.BounceFromPaddle(cpu_, false);
    }

    if (ball_.IsOutLeft()) {
        scoreBoard_.CpuScored();
        ball_.Reset(Vector2{screenWidth_ * 0.5F, screenHeight_ * 0.5F}, 1.0F);
    } else if (ball_.IsOutRight(static_cast<float>(screenWidth_))) {
        scoreBoard_.PlayerScored();
        ball_.Reset(Vector2{screenWidth_ * 0.5F, screenHeight_ * 0.5F}, -1.0F);
    }

    if (scoreBoard_.HasWinner()) {
        gameState_ = GameState::GameOver;
    }
}

void Game::TryRestart() {
    if (!IsKeyPressed(KEY_R)) {
        return;
    }

    scoreBoard_.Reset();
    gameState_ = GameState::Playing;

    const float centeredY = screenHeight_ * 0.5F - kPaddleHeight * 0.5F;
    player_.SetY(centeredY);
    cpu_.SetY(centeredY);

    ball_.Reset(Vector2{screenWidth_ * 0.5F, screenHeight_ * 0.5F}, (GetRandomValue(0, 1) == 0) ? -1.0F : 1.0F);
}

void Game::Draw() const {
    ClearBackground(Color{18, 18, 24, 255});

    DrawLine(screenWidth_ / 2, 0, screenWidth_ / 2, screenHeight_, Color{50, 50, 70, 255});
    player_.Draw(Color{230, 230, 230, 255});
    cpu_.Draw(Color{230, 230, 230, 255});
    ball_.Draw(Color{235, 180, 80, 255});
    scoreBoard_.Draw(screenWidth_);

    DrawText("W/S or UP/DOWN to move", 28, screenHeight_ - 40, 20, Color{160, 160, 170, 255});

    if (gameState_ == GameState::GameOver) {
        DrawRectangle(0, 0, screenWidth_, screenHeight_, Fade(BLACK, 0.4F));
        DrawText(scoreBoard_.PlayerWon() ? "You Win!" : "Computer Wins!", screenWidth_ / 2 - 120, screenHeight_ / 2 - 30, 40, RAYWHITE);
        DrawText("Press R to restart", screenWidth_ / 2 - 110, screenHeight_ / 2 + 22, 24, Color{220, 220, 220, 255});
    }
}

