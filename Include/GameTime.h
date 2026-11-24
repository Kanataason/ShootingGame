#pragma once

//1フレームの経過時間と、ゲーム開始からの経過時間を持つ
class Time
{
public:

	//前のフレームからの経過秒数を返し、ゲーム開始からの経過時間に足す
	//prevCountは前のフレームのGetNowCountの値。呼ぶと今の値に書き換わる
	static float GetDeltaTime(int& prevCount);

	 static float GetWorldTime() { return WorldTime; }

private:
	static constexpr float MaxDeltaTime = 0.1f;

	//inlineでメモリ領域を確保
	inline static float WorldTime;

	static void SetWorldTime(float deltaTime) { WorldTime += deltaTime; }
};
