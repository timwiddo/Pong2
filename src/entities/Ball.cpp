#include "entities/Ball.h"

#include <algorithm>
#include <cmath>

#include "entities/Paddle.h"

namespace {
constexpr float kServeVerticalRatio = 0.58F;
constexpr float kBounceSpeedMultiplier = 1.05F;
}  // namespace

Ball::Ball(Vector2 startPosition, float radius, float baseSpeed)
    : position_(startPosition), velocity_(MakeServeVelocity(1.0F)), radius_(radius), baseSpeed_(baseSpeed) {
}

void Ball::Reset(Vector2 centerPosition, float horizontalDirection) {
    position_ = centerPosition;
    velocity_ = MakeServeVelocity(horizontalDirection);
}

void Ball::Update(float deltaTime) {
    position_.x += velocity_.x * deltaTime;
    position_.y += velocity_.y * deltaTime;
}

void Ball::BounceFromTop() {
    position_.y = radius_;
    velocity_.y *= -1.0F;
}

void Ball::BounceFromBottom(float screenHeight) {
    position_.y = screenHeight - radius_;
    velocity_.y *= -1.0F;
}

void Ball::BounceFromPaddle(const Paddle& paddle, bool fromLeftPaddle) {
    const Rectangle paddleBounds = paddle.Bounds();

    if (fromLeftPaddle) {
        position_.x = paddleBounds.x + paddleBounds.width + radius_;
        velocity_.x = std::fabs(velocity_.x) * kBounceSpeedMultiplier;
    } else {
        position_.x = paddleBounds.x - radius_;
        velocity_.x = -std::fabs(velocity_.x) * kBounceSpeedMultiplier;
    }

    const float halfHeight = paddleBounds.height * 0.5F;
    const float impact = std::clamp((position_.y - paddle.CenterY()) / halfHeight, -1.0F, 1.0F);
    velocity_.y = baseSpeed_ * impact;
}

void Ball::Draw(Color color) const {
    DrawCircleV(position_, radius_, color);
}

Rectangle Ball::Bounds() const {
    return Rectangle{position_.x - radius_, position_.y - radius_, radius_ * 2.0F, radius_ * 2.0F};
}

float Ball::VelocityX() const {
    return velocity_.x;
}

float Ball::Y() const {
    return position_.y;
}

bool Ball::IsOutLeft() const {
    return position_.x + radius_ < 0.0F;
}

bool Ball::IsOutRight(float screenWidth) const {
    return position_.x - radius_ > screenWidth;
}

Vector2 Ball::MakeServeVelocity(float horizontalDirection) const {
    const float verticalDirection = (GetRandomValue(0, 1) == 0) ? -1.0F : 1.0F;
    return Vector2{
        baseSpeed_ * horizontalDirection,
        baseSpeed_ * kServeVerticalRatio * verticalDirection
    };
}

