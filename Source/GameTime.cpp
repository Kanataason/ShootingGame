#include <DxLib.h>

#include "GameTime.h"

float Time::GetDeltaTime(int& prevCount)
{
	int nowCount = GetNowCount();
	float result = (float)(nowCount - prevCount) / 1000.0f;
	prevCount = nowCount;

	//ウィンドウ操作などで処理が止まった直後に、弾や敵がワープしないよう上限を付ける
	if (result > MaxDeltaTime) result = MaxDeltaTime;

	SetWorldTime(result);

	return result;
}
