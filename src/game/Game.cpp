#include "game/Game.h"

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
}  // namespace

Game::Game(const int screenWidth, const int screenHeight)
    : screenWidth_(screenWidth),
      screenHeight_(screenHeight),
      playerPaddle_(Rectangle{paddleMargin, screenHeight * 0.5F - paddleHeight * 0.5F, paddleWidth, paddleHeight}, playerSpeed),
      cpuPaddle_(Rectangle{screenWidth - (paddleMargin + paddleWidth), screenHeight * 0.5F - paddleHeight * 0.5F, paddleWidth, paddleHeight}, cpuSpeed),
      pongBall_(Vector2{screenWidth * 0.5F, screenHeight * 0.5F}, ballRadius, ballBaseSpeed),
      scoreBoard_(targetScore) {
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
    if (gameState_ == GameState::Playing) {
        UpdatePlaying(deltaTime);
    } else {
        TryRestart();
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

void Game::TryRestart() {
    if (!IsKeyPressed(KEY_R)) {
        return;
    }

    scoreBoard_.Reset();
    gameState_ = GameState::Playing;

    CenterPaddles();
    ServeFromCenter((GetRandomValue(0, 1) == 0) ? -1.0F : 1.0F);
}

void Game::Draw() const {
    ClearBackground(Color{18, 18, 24, 255});

    DrawLine(screenWidth_ / 2, 0, screenWidth_ / 2, screenHeight_, Color{50, 50, 70, 255});
    playerPaddle_.Draw(Color{230, 230, 230, 255});
    cpuPaddle_.Draw(Color{230, 230, 230, 255});
    pongBall_.Draw(Color{235, 180, 80, 255});
    scoreBoard_.Draw(screenWidth_);

    DrawText("W/S or UP/DOWN to move", 28, screenHeight_ - 40, 20, Color{160, 160, 170, 255});

    if (gameState_ == GameState::GameOver) {
        DrawRectangle(0, 0, screenWidth_, screenHeight_, Fade(BLACK, 0.4F));
        DrawText(scoreBoard_.PlayerWon() ? "You Win!" : "Computer Wins!", screenWidth_ / 2 - 120, screenHeight_ / 2 - 30, 40, RAYWHITE);
        DrawText("Press R to restart", screenWidth_ / 2 - 110, screenHeight_ / 2 + 22, 24, Color{220, 220, 220, 255});
    }
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

