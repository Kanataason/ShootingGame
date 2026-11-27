#include "PlayerBuff.h"
#include <algorithm>

int GetBuffColor(BuffType type)
{
	//機体の光・オーラ・HUDの文字で同じ色を使い、どの強化かを色だけで分かるようにする
	switch (type)
	{
	case BuffType::Attack:  return 0xFF4444;//赤
	case BuffType::Defense: return 0x4488FF;//青
	case BuffType::Speed:   return 0xFFDD00;//黄
	case BuffType::Predict: return 0xBB66FF;//紫
	case BuffType::Heal:    return 0x44FF88;//緑
	default:                return 0xFFFFFF;//Countなど、強化ではないもの
	}
}

const char* GetBuffInitial(BuffType type)
{
	//色が見分けにくい人でも種類が分かるように、色と一緒に出す
	switch (type)
	{
	case BuffType::Attack:  return "A";
	case BuffType::Defense: return "D";
	case BuffType::Speed:   return "S";
	case BuffType::Predict: return "P";
	case BuffType::Heal:    return "H";
	default:                return "";
	}
}

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
