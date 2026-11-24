#include "EnemyStatus.h"

void EnemyStatus::OnColliderEnter(float damage)
{
	if (!GetIsActive()) return;//死亡演出中はダメージを受けない
	ApplyDamage(damage);
}

void EnemyStatus::OnDead()
{
	SetIsActive(false);
}
