#pragma once
#undef min
#undef max
#define NOMINMAX

#include <algorithm>

#include <DxLib.h>
#include "Vector2.h"

class EffectUtillity
{
public:
	//被弾点滅：残り時間の間、一定間隔で表示と非表示を切り替える
    //残り時間が0以下なら常に表示する
	static bool IsBlinkVisible(float remainingTime, float interval = 0.08f)
	{
		if (remainingTime <= 0.0f) return true;
		return static_cast<int>(remainingTime / interval) % 2 == 0;
	}

	//渡された引数でCircleを作って死亡エフェクトとして描画する
    static void DrawEffect(float elapsed,float effectTime,int color,const Vector2& spownPos) 
    {
		float ratio = std::min(elapsed / effectTime, 1.0f);
		int alpha = static_cast<int>(255 * (1.0f - ratio));//だんだん消える

		SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
		for (int ring = 0; ring < 3; ++ring)
		{
			int radius = static_cast<int>(ratio * 60.0f) + ring * 12;//広がる輪を3重に
			DrawCircle((int)spownPos.x, (int)spownPos.y, radius,color, FALSE, 3);
		}
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    }
};