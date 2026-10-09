#pragma once
#include <string>
#include "Vector2.h"
#include "Datas.h"
#include "ColliderRegistry.h"
#include "ObjectPool.h"


//弾1発分のデータ。太さを持った線分として描画・判定する
struct  Bullet
{
	Vector2 position{};//先端の位置
	Vector2 velocity{};
	Vector2 direction{};//進行方向の単位ベクトル。速度は変わらないので生成時に1回だけ計算する
	float lifeTime = 0.0f;
	float damage = 0.0f;
	float length = 10.0f;
	float thickness = 3;
	int bounceCount = 0;

	int color=0;
	Team ownerTeam = Team::None;

	Bullet() = default;
	Bullet(const Vector2& startPosition, const Vector2& startVelocity,
		float lifetime,float attackDamage,float length,int color,Team team,float thickness,int bounceCount = 0)
		: position(startPosition), velocity(startVelocity), direction(startVelocity.Normalized()),
		lifeTime(lifetime), damage(attackDamage), length(length), thickness(thickness),bounceCount(bounceCount),
		color(color), ownerTeam(team) {
	}

	//線分の後ろ側の端。描画と当たり判定の両方で使う
	Vector2 GetTail() const { return position - direction * length; }
};

constexpr std::size_t BulletPoolCapacity = 512;
using BulletPool = ObjectPool<Bullet, BulletPoolCapacity>;