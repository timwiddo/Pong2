#include "ui/ScoreBoard.h"

#include <raylib.h>

namespace {
constexpr int kScoreFontSize = 56;
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
    DrawText(TextFormat("%d", playerScore_), screenWidth / 2 - 110, 35, kScoreFontSize, Color{230, 230, 230, 255});
    DrawText(TextFormat("%d", cpuScore_), screenWidth / 2 + 80, 35, kScoreFontSize, Color{230, 230, 230, 255});
}

bool ScoreBoard::HasWinner() const {
    return playerScore_ >= targetScore_ || cpuScore_ >= targetScore_;
}

bool ScoreBoard::PlayerWon() const {
    return playerScore_ > cpuScore_;
}

