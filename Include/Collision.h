#pragma once
#include "Vector2.h"

#include "CircleCollider.h"
#include "BoxCollider.h"

//当たり判定と押し戻しの計算だけをまとめたもの
//弾は太さを持った線分として扱う
class Collision
{
public:
	//円と、太さを持った線分（弾）が当たっているか
	static bool CheckCollision(const Vector2& center, float radius, float halfThickness, const Vector2& segmentStart, const Vector2& segmentEnd);
	//四角形と、太さを持った線分（弾）が当たっているか
	static bool CheckCollision(const Vector2& center, const Vector2& halfSize, float halfThickness, const Vector2& segmentStart,
		const Vector2& segmentEnd);

	//重なっていたら押し戻してtrueを返す。重なっていなければ何もせずfalse
	//円と円：半分ずつ押し戻す
	static bool Resolve(CircleCollider* colliderA, CircleCollider* colliderB);
	//四角形と円：円だけを押し戻す
	static bool Resolve(BoxCollider* boxCollider, CircleCollider* circleCollider);
	//四角形と四角形：めり込みが浅い軸の方向に、半分ずつ押し戻す
	static bool Resolve(BoxCollider* boxColliderA, BoxCollider* boxColliderB);
};
