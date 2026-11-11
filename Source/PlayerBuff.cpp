#include "PlayerBuff.h"
#include <algorithm>

void PlayerBuff::Apply(BuffType type)
{
	//回復は段階を持たない
	if (type == BuffType::Count || type == BuffType::Heal) return;

	auto& slot = _slots[static_cast<size_t>(type)];
	//minマクロ対策
	slot.level = (std::min)(slot.level + 1, MaxLevel);
	slot.remaining = Duration;
	OnBuffChanged.Invoke(type, slot.level, slot.remaining);
}

void PlayerBuff::Update(float deltaTime)
{
	//残り時間を減らす。HUDに出すので毎フレーム通知する
	for (size_t index = 0; index < _slots.size(); ++index)
	{
		auto& slot = _slots[index];
		if (slot.level == 0) continue;

		slot.remaining -= deltaTime;
		if (slot.remaining <= 0.0f)
		{
			slot = {};//時間切れで未強化に戻す
		}
		OnBuffChanged.Invoke(static_cast<BuffType>(index), slot.level, slot.remaining);
	}
}
