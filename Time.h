#pragma once
#include "DxLib.h"

class Time
{
public:

	//デルタタイムを取得関数
	static float GetDeltaTime(int& prevCount)
	{
	    int nowCount = GetNowCount();
		float result = (float)(nowCount - prevCount) / 1000.0f;
		prevCount = nowCount;

		//ウィンドウ操作などで処理が止まった直後に、弾や敵がワープしないよう上限を付ける
		if (result > MaxDeltaTime) result = MaxDeltaTime;

		SetWorldTime(result);

		return result;
	}

	 static float GetWorldTime() { return WorldTime; }

private:
	static constexpr float MaxDeltaTime = 0.1f;

	//inlineでメモリ領域を確保
	inline static float WorldTime;

	static void SetWorldTime(float deltaTime) { WorldTime += deltaTime; }
};