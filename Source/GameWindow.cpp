#include <algorithm>

#include "GameWindow.h"

bool ScreenBounds::Contains(const Vector2& position, float margin) const
{
    return position.x >= -margin && position.x <= width + margin &&
        position.y >= -margin && position.y <= height + margin;
}

float ScreenBounds::ClampY(float wrappedPosition, float min, float max, float offset)const
{
    //プレイヤーの半径を追加してめり込まないようにする
    return std::clamp(wrappedPosition, min + offset, max - offset);
}

Vector2 ScreenBounds::Wrap(const Vector2& position) const
{
    Vector2 wrappedPosition = position;
    if (wrappedPosition.x < 0.0f) { wrappedPosition.x += width; }
    if (wrappedPosition.x > width) { wrappedPosition.x -= width; }

    return wrappedPosition;
}
