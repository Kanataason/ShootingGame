#include <DxLib.h>

#include "BossController.h"
#include "BossStates.h"
#include "EnemyMovement.h"
#include "EnemyAttack.h"
#include "EnemyStatus.h"
#include "Collider.h"
#include "GameWindow.h"
#include "GameObject.h"
#include "DrawManager.h"
#include "EffectUtillity.h"
#include "GameTime.h"

using namespace BossStates;

void BossController::Initialize(ServiceLocator& locator)
{
	_movement = GetOwner()->GetComponent<EnemyMovement>();
	_attack = GetOwner()->GetComponent<EnemyAttack>();
	_status = GetOwner()->GetComponent<EnemyStatus>();
	_collider = GetOwner()->GetComponent<Collider>();
	_bounds = &locator.gameWindow->GetBounds();
	_drawManager = locator.drawManager;
	_soundManager = locator.soundManager;

	//第一形態の攻撃の順番
	_attackOrder = { EventLongShot, EventReflectShot, EventSpreadShot };

	SubScribeEvents();

	//ステートのOnEnterがここで呼ばれるため、ステートが使うものは先に設定しておく
	InitializeStateMachine();
}
void BossController::SubScribeEvents()
{
	_status->OnHealthChanged.Subscribe([this](float hp, float maxHp) { OnHealthChanged(hp, maxHp); });
	//弾が当たった時刻を覚えておき、描くときに一瞬白く光らせる。HPが多いので、当たっている手ごたえを見せるため
	_status->OnDamaged.Subscribe([this] { _hitTime = Time::GetWorldTime(); });

}

void BossController::InitializeStateMachine()
{
	//移動：登場 → 第一形態 → 無敵（形態移行） → 第二形態。撃破はどこからでも
	_normalState.AddTransition<EnterState, AliveState>(LifeEvent::EventEntered);
	_normalState.AddTransition<AliveState, InvincibleState>(LifeEvent::EventInvincible);
	_normalState.AddTransition<InvincibleState,SecondForm>(LifeEvent::EventSecondForm);
	_normalState.AnyAddTransition<DeadState>(LifeEvent::EventDied);

	//攻撃の順番を自由に入れ替えられるよう、どのステートからでも遷移できるようにする
	_attackState.AnyAddTransition<LongRangeShotState>(AttackEvent::EventLongShot);
	_attackState.AnyAddTransition<ReflectShotState>(AttackEvent::EventReflectShot);
	_attackState.AnyAddTransition<SpreadShotState>(AttackEvent::EventSpreadShot);
	_attackState.AnyAddTransition<AttackDisabledState>(AttackEvent::EventAttackStop);
	_attackState.AnyAddTransition<SwirlShotState>(AttackEvent::EventSwirlShot);

	//登場中は撃たない。登場が終わるとEnterStateがStartAttackを呼ぶ
	_attackState.Start<AttackDisabledState>();
	_normalState.Start<EnterState>();
}

void BossController::StartAttack()
{
	_isAttacking = true;
	_attackOrderIndex = 0;
	_attackTimer = 0.0f;
	_attackState.Dispatch(_attackOrder[_attackOrderIndex]);
}

void BossController::Update(float deltaTime)
{
	//HPが0になったフレームで1回だけ死亡用のステートに切り替える
	if (!_status->GetIsActive() && !_isDead)
	{
		_isDead = true;
		_isAttacking = false;
		_collider->SetCollisionEnabled(false);
		_normalState.Dispatch(LifeEvent::EventDied);
		_attackState.Dispatch(AttackEvent::EventAttackStop);
	}

	//攻撃の種類を切り替えてから、移動と攻撃のステートを進める
	UpdateAttackRotation(deltaTime);
	_normalState.Update(deltaTime);
	_attackState.Update(deltaTime);
}

void BossController::UpdateAttackRotation(float deltaTime)
{
	//攻撃の切り替えは、攻撃中でかつ攻撃の順番が1種類以上あるときだけ行う
	if (!_isAttacking || _attackOrder.empty()) return;

	_attackTimer += deltaTime;
	if (_attackTimer < _attackDuration) return;

	//次の攻撃へ切り替える
	_attackTimer = 0.0f;
	if (_attackOrder.size() > 1)
	{
		//進める数は1〜(種類の数-1)。1周しないので、今と同じ攻撃は選ばれない
		//例：3種類なら1か2進むので、今の攻撃以外の2つから選ばれる
		int step = 1 + GetRand(static_cast<int>(_attackOrder.size()) - 2);
		_attackOrderIndex = (_attackOrderIndex + step) % _attackOrder.size();
	}
	_attackState.Dispatch(_attackOrder[_attackOrderIndex]);
}


void BossController::Draw()
{
	//本体・点滅・撃破エフェクトの描き分けはステートが行う
	_drawManager->Register(DrawLayer::Enemy, [this] { _normalState.Draw(); });
	_drawManager->Register(DrawLayer::Enemy, [this] { _attackState.Draw(); });//攻撃予告
}

void BossController::DrawBody() const
{
	//座標が画像の中心になる。画像は表示する大きさと向きに加工済みなので、等倍・回転なしで描く
	const auto& pos = GetOwner()->GetTransform().Position;
	DrawRotaGraph((int)pos.x, (int)pos.y, 1.0, 0.0, _status->GetImagePath(), TRUE);
	if (EffectUtillity::IsHitFlashing(_hitTime))
		EffectUtillity::DrawHitFlash(pos, _status->GetImagePath());
}


void BossController::OnHealthChanged(float hp, float maxHp)
{
	if (_isSecondPhase || hp <= 0.0f) return;
	if (hp > maxHp * 0.4f) return;

	_isSecondPhase = true;
	_attackOrder.push_back(EventSwirlShot);
	_normalState.Dispatch(LifeEvent::EventInvincible);   //形態移行ステートへ
}