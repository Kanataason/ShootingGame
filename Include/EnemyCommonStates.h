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
//どう撃つかと、どの方向へ撃つかだけを継承先で決める
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

		//攻撃が切り替わった最初の1発は遅らせて、予告を見る時間を作る
		_interval = (std::max)(_shotCoolDown, FirstShotDelay);
		SetGunState(_interval);
	}
	void OnUpdate(float deltaTime) override
	{
		if (_coolDown <= 0.0f)
		{
			SetShotInfo(false, _shotCoolDown);
			_interval = _shotCoolDown;
			Shoot(*this->owner().GetAttack());
		}
		ElapsedTimer(deltaTime);
	}
	void OnDraw() override
	{
		//連射が速すぎるときは、予告が点滅して見づらいので出さない
		//その場合も、攻撃が切り替わった最初の1発には出る
		if (_interval < MinWarnInterval) return;

		const EnemyAttack& attack = *this->owner().GetAttack();
		//撃つ間隔のほとんどが予告にならないように、間隔の一部までにする
		const float warnTime = (std::min)(attack.GetWarnTime(), _interval * MaxWarnRatio);
		if (_coolDown > warnTime) return;

		attack.DrawWarning(GetWarnDirections(attack), 1.0f - _coolDown / warnTime);
	}

	//1回分の撃ち方
	virtual void Shoot(EnemyAttack& attack) = 0;
	//次に撃つ弾の方向。攻撃予告に使うので、Shootと同じ方向を返す
	virtual std::vector<Vector2> GetWarnDirections(const EnemyAttack& attack) const = 0;
private:
	static constexpr float FirstShotDelay = 0.8f; //攻撃が切り替わってから最初に撃つまでの秒数
	static constexpr float MinWarnInterval = 0.25f;//撃つ間隔がこれより短いと予告を出さない
	static constexpr float MaxWarnRatio = 0.8f;    //撃つ間隔のうち、予告に使ってよい割合

	size_t _attackIndex;
	float _shotCoolDown;
	float _interval = 0.0f;//前に撃ってから（入った直後は入ってから）次に撃つまでの秒数
};
