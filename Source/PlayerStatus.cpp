#include "PlayerStatus.h"
#include "PlayerProvider.h"
#include "PlayerBuff.h"
#include "GameObject.h"

void PlayerStatus::Initialize(ServiceLocator& locator)
{
	_provider = locator.playerProvider;
	_buff = GetOwner()->GetComponent<PlayerBuff>();
}

void PlayerStatus::OnColliderEnter(float damage)
{
	if (!GetIsActive()) return;//死亡演出中はダメージを受けない。無敵中は当たり判定自体が無効になる
	ApplyDamage(damage * _buff->GetDamageRate());//防御の強化中は被ダメージを減らす
	OnHitSound.Invoke(SoundId::PlayerHit);
}

void PlayerStatus::OnDead()
{
	SetIsActive(false);
	//敵が撃つのを止めるために、生存状態を共有する
	_provider->SetPlayerActive(false);
}
