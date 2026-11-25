#include "PlayerStatus.h"
#include "PlayerProvider.h"
#include "PlayerBuff.h"
#include "GameObject.h"
#include "GameSettings.h"

void PlayerStatus::Initialize(ServiceLocator& locator)
{
	_provider = locator.playerProvider;
	_buff = GetOwner()->GetComponent<PlayerBuff>();
	_damageTakenRate = locator.gameSettings->GetDifficulty().damageTakenRate;
}

void PlayerStatus::OnColliderEnter(float damage)
{
	if (!GetIsActive()) return;//死亡演出中はダメージを受けない。無敵中は当たり判定自体が無効になる
	//防御の強化中は被ダメージを減らす。難易度の倍率も、弾と体当たりの両方にここでまとめて掛ける
	ApplyDamage(damage * _buff->GetDamageRate() * _damageTakenRate);
	OnHitSound.Invoke(SoundId::PlayerHit);
}

void PlayerStatus::OnDead()
{
	SetIsActive(false);
	//敵が撃つのを止めるために、生存状態を共有する
	_provider->SetPlayerActive(false);
}
