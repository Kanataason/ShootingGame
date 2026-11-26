#include <DxLib.h>
#include <cmath>

#include "ScreenShake.h"

namespace
{
	//-strength〜strengthのランダムな整数。DrawGraphは整数の座標で描くので、整数にそろえる
	float RandomOffset(float strength)
	{
		const int range = static_cast<int>(std::round(strength));
		if (range <= 0) return 0.0f;
		return static_cast<float>(GetRand(range * 2) - range);
	}
}

void ScreenShake::Start(float strength, float duration)
{
	if (duration <= 0.0f) return;
	//今の揺れの方が強ければ、そのまま続ける
	if (strength < GetCurrentStrength()) return;

	_strength = strength;
	_duration = duration;
	_remaining = duration;
}

void ScreenShake::Update(float deltaTime)
{
	_remaining -= deltaTime;
	if (_remaining <= 0.0f)
	{
		_remaining = 0.0f;
		_offset = {};
		return;
	}

	//縦横ばらばらに揺らす。毎フレーム向きが変わるので、細かく震えて見える
	const float strength = GetCurrentStrength();
	_offset = { RandomOffset(strength), RandomOffset(strength) };
}

float ScreenShake::GetCurrentStrength() const
{
	if (_remaining <= 0.0f || _duration <= 0.0f) return 0.0f;
	//始めが一番強く、終わりに向かって0になる。急に止まって見えないようにするため
	return _strength * (_remaining / _duration);
}
