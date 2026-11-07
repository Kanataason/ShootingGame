#pragma once
#include "Event.h"
#include "Component.h"

enum class BuffType 
{
	Attack, Defense, Speed, Count 
};


class PlayerBuff : public Component
{
public:
	Event<BuffType, int, float> OnBuffChanged;//HUDへ送るイベント

	std::unique_ptr<Component> Clone()const override
	{
		return std::make_unique<PlayerBuff>(*this);
	}
	void Apply(BuffType type);
	void Update(float deltaTime)override;
	void Initialize(ServiceLocator& locator)override;
private:

	float GetAttackRate() const;
	float GetDamageRate() const;
	float GetSpeedRate() const;

};