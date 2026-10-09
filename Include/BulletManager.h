#pragma once
#include "Vector2.h"
#include "GunData.h"
#include "ObjectPool.h"
#include "Bullet.h"
#include "ServiceLocator.h"
#include "GameWindow.h"

//全ての弾の移動・寿命・描画をオブジェクトプールで管理する
//当たり判定はColliderManagerが行う
class BulletManager
{
public:
	BulletManager() = default;
	~BulletManager() = default;

	void Initialize(ServiceLocator& locator);

	void Update(float deltaTime);
	void Draw();

	//プールが満杯のときは撃たない
	void FireBullet(const Vector2& position, const Vector2& velocity, const ShotSetting& data, int color, Team team);


	BulletPool& GetPool() { return _bulletPool; }
private:
	void BulletMovement(Bullet& bullet,float deltaTime);
	//弾描写
	void DrawBullet(const Bullet& bullet);

	//左右の壁で跳ね返す。跳ね返れる回数が残っていなければ、今まで通り画面外で消す
	void Reflect(Bullet& bullet);
	//寿命が尽きたか、画面外に出たらtrueを返す
	bool ShouldRelease(Bullet& bullet,float deltaTime);
private:
	const ScreenBounds* _screenBounds{};

	BulletPool  _bulletPool;
};