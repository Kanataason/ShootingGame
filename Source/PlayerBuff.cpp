#include "PlayerBuff.h"
#include <algorithm>

void PlayerBuff::Apply(BuffType type)
{
	//回復はその場で終わるので、段階も残り時間も持たない
	if (type == BuffType::Count || type == BuffType::Heal) return;

	auto& slot = _slots[static_cast<size_t>(type)];
	//DxLib経由で入るWindowsのminマクロと衝突しないように、かっこで囲む
	slot.level = (std::min)(slot.level + 1, MaxLevel);
	slot.remaining = Duration;
	OnBuffChanged.Invoke(type, slot.level, slot.remaining);
}

void PlayerBuff::Update(float deltaTime)
{
	//バフの時間を減らす。HUDで残り秒数を出すので、かかっている間は毎フレーム通知する
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
