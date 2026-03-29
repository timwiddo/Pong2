#include "ui/ScoreBoard.h"

#include <raylib.h>

namespace {
constexpr int scoreFontSize = 56;
constexpr int scoreY = 35;
constexpr int playerScoreXOffset = 110;
constexpr int cpuScoreXOffset = 80;
}  // namespace

ScoreBoard::ScoreBoard(int targetScore) : targetScore_(targetScore) {
}

void ScoreBoard::PlayerScored() {
    ++playerScore_;
}

void ScoreBoard::CpuScored() {
    ++cpuScore_;
}

void ScoreBoard::Reset() {
    playerScore_ = 0;
    cpuScore_ = 0;
}

void ScoreBoard::Draw(int screenWidth) const {
    const int centerX = screenWidth / 2;
    DrawText(TextFormat("%d", playerScore_), centerX - playerScoreXOffset, scoreY, scoreFontSize, Color{230, 230, 230, 255});
    DrawText(TextFormat("%d", cpuScore_), centerX + cpuScoreXOffset, scoreY, scoreFontSize, Color{230, 230, 230, 255});
}

bool ScoreBoard::HasWinner() const {
    return playerScore_ >= targetScore_ || cpuScore_ >= targetScore_;
}

bool ScoreBoard::PlayerWon() const {
    return playerScore_ > cpuScore_;
}

