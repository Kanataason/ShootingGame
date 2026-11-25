#include <DxLib.h>

#include "GameTime.h"

float Time::GetDeltaTime(int& prevCount)
{
	//GetNowCountはミリ秒なので秒に直す。次のフレームの計算のために、今の時刻を返しておく
	int nowCount = GetNowCount();
	float result = (float)(nowCount - prevCount) / 1000.0f;
	prevCount = nowCount;

	//ウィンドウ操作などで処理が止まった直後に、弾や敵がワープしないよう上限を付ける
	if (result > MaxDeltaTime) result = MaxDeltaTime;

	//ゲーム開始からの時間も進めておく。点滅や表示時間の計算に使う
	SetWorldTime(result);

	return result;
}
