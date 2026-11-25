#include "Vector2.h"

Vector2 Vector2::Normalized() const
{
    return Normalized(Length());
}

Vector2 Vector2::Normalized(float length) const
{
    //長さ0で割らないように、向きのないベクトルはそのまま0にする
    if (length <= 0.0f)
        return { 0.0f, 0.0f };
    return { x / length, y / length };
}
