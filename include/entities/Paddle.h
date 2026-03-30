#pragma once

#include <raylib.h>

class Paddle {
public:
    Paddle(Rectangle bounds, float speed);

    void Move(float direction, float deltaTime, int screenHeight);
    void SetY(float y);
    void SetHeight(float height, int screenHeight);
    void Draw(Color color) const;

    [[nodiscard]] float CenterY() const;
    [[nodiscard]] const Rectangle& Bounds() const;

private:
    Rectangle bounds_{};
    float speed_{};
};

