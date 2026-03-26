#include "entities/Paddle.h"

#include <algorithm>

Paddle::Paddle(Rectangle bounds, float speed) : bounds_(bounds), speed_(speed) {
}

void Paddle::Move(float direction, float deltaTime, int screenHeight) {
    bounds_.y += direction * speed_ * deltaTime;
    bounds_.y = std::clamp(bounds_.y, 0.0F, static_cast<float>(screenHeight) - bounds_.height);
}

void Paddle::SetY(float y) {
    bounds_.y = y;
}

void Paddle::Draw(Color color) const {
    DrawRectangleRec(bounds_, color);
}

float Paddle::CenterY() const {
    return bounds_.y + bounds_.height * 0.5F;
}

const Rectangle& Paddle::Bounds() const {
    return bounds_;
}

