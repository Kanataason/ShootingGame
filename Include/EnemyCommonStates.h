#pragma once
#include <algorithm>
#include <vector>
#include <DxLib.h>
#include "StateMachine.h"
#include "GunState.h"
#include "Vector2.h"
#include "GameObject.h"
#include "GameWindow.h"
#include "EnemyMovement.h"
#include "EnemyAttack.h"

//雑魚とボスで共通して使うステートの基底クラス
//TOwnerはステートの持ち主（EnemyController / BossController）

//同じ高さのまま、左右のランダムな位置へ移動し続ける
//TOwnerに必要なもの：GetMovement()、GetBounds()
template<class TOwner>
class RandomMoveXState : public StateMachine<TOwner>::State
{
protected:
	using StateBase = typename StateMachine<TOwner>::State;

	RandomMoveXState(float moveDuration, float screenMargin)
		: _moveDuration(moveDuration), _screenMargin(screenMargin) {
	}

	void OnEnter(StateBase* previousState) override
	{
		SetRandomTargetX();
	}
	void OnUpdate(float deltaTime) override
	{
		//目的地に着いたら次の目的地を決める
		if (this->owner().GetMovement()->Translate(deltaTime))
			SetRandomTargetX();
	}
private:
	void SetRandomTargetX()
	{
		auto& controller = this->owner();
		//左右の端から_screenMarginずつ内側の範囲で、横の位置だけを決める。高さは今のまま
		float range = controller.GetBounds().width - _screenMargin * 2.0f;
		Vector2 target = controller.GetOwner()->GetTransform().Position;
		target.x = _screenMargin + static_cast<float>(GetRand(static_cast<int>(range)));
		controller.GetMovement()->SetDestination(target, _moveDuration);
	}
private:
	float _moveDuration;
	float _screenMargin;   //画面の端から、この距離より内側を移動先にする
};

//JSONのAttackTypeのattackIndex番目の弾を、一定間隔で撃ち続ける
//撃つ少し前には攻撃予告を出す
//継承先はShootとGetWarnDirectionsを書く
//TOwnerに必要なもの：GetAttack()
template<class TOwner>
class ShotState : public StateMachine<TOwner>::State, public GunState
{
protected:
	using StateBase = typename StateMachine<TOwner>::State;

	ShotState(size_t attackIndex, float shotCoolDown)
		: _attackIndex(attackIndex), _shotCoolDown(shotCoolDown) {
	}

	void OnEnter(StateBase* previousState) override
	{
		EnemyAttack* attack = this->owner().GetAttack();
		attack->SetCurrentAttackIndex(_attackIndex);
		attack->InitializeShotSetting(attack->GetAttackType());

		//最初の1発は遅らせて予告を見せる
		_interval = (std::max)(_shotCoolDown, FirstShotDelay);
		SetGunState(_interval);
	}
	void OnUpdate(float deltaTime) override
	{
		//間隔が空いたら1回撃ち、次の間隔を数え始める
		if (_coolDown <= 0.0f)
		{
			SetShotInfo(false, _shotCoolDown);
			//最初の1発だけ長かった間隔を、2発目からは通常の間隔に戻す
			_interval = _shotCoolDown;
			Shoot(*this->owner().GetAttack());
		}
		ElapsedTimer(deltaTime);
	}
	void OnDraw() override
	{
		//連射が速いと点滅して見づらいので出さない（最初の1発には出る）
		if (_interval < MinWarnInterval) return;

		const EnemyAttack& attack = *this->owner().GetAttack();
		//予告は間隔の一部まで
		const float warnTime = (std::min)(attack.GetWarnTime(), _interval * MaxWarnRatio);
		if (_coolDown > warnTime) return;

		attack.DrawWarning(GetWarnDirections(attack), 1.0f - _coolDown / warnTime);
	}

	//1回分の撃ち方
	virtual void Shoot(EnemyAttack& attack) = 0;
	//次に撃つ方向。Shootと同じものを返す
	virtual std::vector<Vector2> GetWarnDirections(const EnemyAttack& attack) const = 0;
private:
	static constexpr float FirstShotDelay = 0.8f; //最初の1発までの秒数
	static constexpr float MinWarnInterval = 0.25f;//撃つ間隔がこれより短いと予告を出さない
	static constexpr float MaxWarnRatio = 0.8f;    //予告に使う割合の上限

	size_t _attackIndex;
	float _shotCoolDown;
	float _interval = 0.0f;//次に撃つまでの間隔
};
