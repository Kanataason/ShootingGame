#pragma once
#include "ServiceLocator.h"
#include "ObjectPool.h"
#include "Bullet.h"


class ColliderRegistry;

//当たり判定を担当する
//登録済みのコライダーに対して、弾との判定とコライダー同士の押し戻しを行う
class ColliderManager
{
public :
	void Initialize(ServiceLocator& locator);
	//弾とコライダーの当たり判定。当たった弾は消し、当たった側のOnColliderEnterを呼ぶ
	void CheckCollisions(BulletPool& objectPool);
	//コライダー同士が重なっていたら押し戻し、プレイヤー側に体当たりのダメージを与える
	void ResolveCollider();

private:
	ColliderRegistry* _colliderRegistry = nullptr;
};