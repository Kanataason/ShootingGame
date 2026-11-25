#pragma once
#include "Component.h"
#include "Datas.h"
#include "CharacterStatus.h"
#include "Primitive.h"

class PlayerProvider;
class PlayerBuff;

//円の半径を持つ。描画と移動制限に使う
class CircleShape : public Primitive
{
public:
	void SetRadius(float radius) { _radius = radius; }
	float GetRadius() const override { return _radius; }
private:
	float _radius = 0.0f;
};

//プレイヤーのHPと生存状態を持つ
class PlayerStatus : public Component, public CharacterStatus
{
public:
	Event<SoundId> OnHitSound;
	std::unique_ptr<Component> Clone()const override
	{
		return std::make_unique<PlayerStatus>(*this);
	}
	void Initialize(ServiceLocator& locator)override;
	void Update(float deltaTime)override {};

	//弾に当たったらダメージを受ける。防御の強化中は減らし、難易度の倍率も掛ける
	void OnColliderEnter(float damage)override;
	//HPが0になったら非アクティブにし、敵にも知らせる
	void OnDead()override;

private:
	PlayerProvider* _provider = nullptr;
	PlayerBuff* _buff = nullptr;
	float _damageTakenRate = 1.0f;//難易度による、受けるダメージの倍率
};
