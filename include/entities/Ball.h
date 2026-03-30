#pragma once

#include <raylib.h>

class Paddle;

class Ball {
public:
    Ball(Vector2 startPosition, float radius, float baseSpeed);

    void Reset(Vector2 centerPosition, float horizontalDirection);
    void Update(float deltaTime);
    void BounceFromTop();
    void BounceFromBottom(float screenHeight);
    void BounceFromPaddle(const Paddle& paddle, bool fromLeftPaddle);
    void SetRadius(float radius);
    void Draw(Color color) const;

    [[nodiscard]] Rectangle Bounds() const;
    [[nodiscard]] float VelocityX() const;
    [[nodiscard]] float Y() const;
    [[nodiscard]] bool IsOutLeft() const;
    [[nodiscard]] bool IsOutRight(float screenWidth) const;

private:
    [[nodiscard]] Vector2 MakeServeVelocity(float horizontalDirection) const;

    Vector2 position_{};
    Vector2 velocity_{};
    float radius_{};
    float baseSpeed_{};
};

