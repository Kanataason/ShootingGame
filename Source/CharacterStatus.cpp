#include "CharacterStatus.h"

void CharacterStatus::TakeDamage(float damage)
{
	_health -= damage;
	OnHealthChanged.Invoke(_health, _maxHealth);
	OnDamaged.Invoke();
}

void CharacterStatus::Heal(float amount)
{
	if (!_isActive) return;
	_health = (_health + amount < _maxHealth) ? _health + amount : _maxHealth;
	OnHealthChanged.Invoke(_health, _maxHealth);
	OnHealed.Invoke();
}

void CharacterStatus::ApplyDamage(float damage)
{
	TakeDamage(damage);
	if (_health <= 0.0f)
	{
		OnDead();
	}
}
