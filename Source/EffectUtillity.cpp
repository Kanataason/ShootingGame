#include <algorithm>
#include <cmath>
#include <DxLib.h>

#include "EffectUtillity.h"

//DxLibが読み込むWindowsのmin・maxマクロが、std::minとぶつかるので消す
#undef min
#undef max

namespace
{
	//オーラ
	constexpr float AuraPulseSpeed = 10.0f; //脈打つ速さ（約0.6秒で1回）
	constexpr int AuraBaseAlpha = 60;      //基本の濃さ
	constexpr int AuraAlphaPerLevel = 40;  //段階ごとに足す濃さ
	constexpr int AuraPulseAlpha = 40;     //脈の濃さの幅
	constexpr float AuraPulseRadius = 5.0f;//脈の大きさの幅
}

bool EffectUtillity::IsBlinkVisible(float remainingTime, float interval)
{
	if (remainingTime <= 0.0f) return true;
	return static_cast<int>(remainingTime / interval) % 2 == 0;
}

void EffectUtillity::DrawEffect(float elapsed, float effectTime, int color, const Vector2& spownPos)
{
	float ratio = std::min(elapsed / effectTime, 1.0f);
	int alpha = static_cast<int>(255 * (1.0f - ratio));//だんだん消える

	SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
	for (int ring = 0; ring < 3; ++ring)
	{
		int radius = static_cast<int>(ratio * 60.0f) + ring * 12;//広がる輪を3重に
		DrawCircle((int)spownPos.x, (int)spownPos.y, radius, color, FALSE, 3);
	}
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

void EffectUtillity::DrawAura(const Vector2& center, float radius, int color, int level, float time)
{
	//0〜1で脈打つ
	float pulse = 0.5f + 0.5f * std::sin(time * AuraPulseSpeed);
	int alpha = AuraBaseAlpha + level * AuraAlphaPerLevel + static_cast<int>(pulse * AuraPulseAlpha);

	SetDrawBlendMode(DX_BLENDMODE_ADD, alpha);//加算で光って見せる
	DrawCircle((int)center.x, (int)center.y, (int)(radius + pulse * AuraPulseRadius), color, FALSE, level + 1);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}
