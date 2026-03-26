#pragma once

class ScoreBoard {
public:
    explicit ScoreBoard(int targetScore);

    void PlayerScored();
    void CpuScored();
    void Reset();
    void Draw(int screenWidth) const;

    [[nodiscard]] bool HasWinner() const;
    [[nodiscard]] bool PlayerWon() const;

private:
    int playerScore_{};
    int cpuScore_{};
    int targetScore_{};
};

