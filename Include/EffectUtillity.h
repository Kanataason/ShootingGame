#pragma once
#undef min
#undef max
#define NOMINMAX

#include <algorithm>
#include <cmath>

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

	//強化中のオーラ：段階が高いほど濃く太くなり、脈打つ
	//timeは経過時間。Time::GetWorldTime()を渡す
	static void DrawAura(const Vector2& center, float radius, int color, int level, float time)
	{
		//0から1を行ったり来たりする値
		float pulse = 0.5f + 0.5f * std::sin(time * AuraPulseSpeed);
		int alpha = AuraBaseAlpha + level * AuraAlphaPerLevel + static_cast<int>(pulse * AuraPulseAlpha);

		SetDrawBlendMode(DX_BLENDMODE_ADD, alpha);//加算で光って見せる
		DrawCircle((int)center.x, (int)center.y, (int)(radius + pulse * AuraPulseRadius), color, FALSE, level + 1);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}

private:
	static constexpr float AuraPulseSpeed = 10.0f; //脈打つ速さ。約1秒で1回
	static constexpr int AuraBaseAlpha = 60;      //一番薄いときの濃さ
	static constexpr int AuraAlphaPerLevel = 40;  //段階が1つ上がるごとに濃くする量
	static constexpr int AuraPulseAlpha = 40;     //脈打つときの濃さの幅
	static constexpr float AuraPulseRadius = 5.0f;//脈打つときの半径の幅（ピクセル）
};