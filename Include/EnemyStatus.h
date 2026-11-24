#pragma once
#include "Component.h"
#include "Vector2.h"
#include "Datas.h"
#include "CharacterStatus.h"
#include "Primitive.h"

//四角形の半分の大きさを持つ。弾を撃つ位置と攻撃予告の輪に使う
class BoxShape : public Primitive
{
public:
	void SetHalfSize(Vector2 half) {_halfSize = half;}
	Vector2 GetHalfSize() const override{ return _halfSize; }
private:
	Vector2 _halfSize;
};

//敵のHP・種類・動き方を持つ
class EnemyStatus : public Component,public CharacterStatus
{
public:
	std::unique_ptr<Component> Clone()const override
	{
		return std::make_unique<EnemyStatus>(*this);
	}
	void Initialize(ServiceLocator& locator)override {};
	void Update(float deltaTime)override {};

	void SetMoveFlag(bool isMove) { _isMove = isMove; }
	bool GetMoveFlag()const { return _isMove; }

	//弾に当たったらダメージを受ける。死亡演出中は受けない
	void OnColliderEnter(float damage)override;
	//攻撃のパターンを設定 パターンはGunDataを参照
	void SetMovePattern(MovePattern pattern) { _pattern = pattern; }
	MovePattern GetMovePattern() const { return _pattern; }
	//HPが0になったら非アクティブにするだけ。死亡時の処理はEnemyControllerが行う
	void OnDead()override;

private:
	MovePattern _pattern = MovePattern::EnterAndStay;
	bool _isMove = true;
};