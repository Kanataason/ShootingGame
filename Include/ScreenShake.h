#pragma once
#include "Vector2.h"

//画面の揺れ。揺らす強さ（ピクセル）と時間を受け取り、毎フレームのずらし量を決める
//揺れは時間とともに弱くなる。描画をずらすのはDrawManagerで、ここはずらし量を計算するだけ
class ScreenShake
{
public:
	//揺らし始める。揺れている途中なら、強い方を残す
	//弱い揺れで、大きな揺れが途中で打ち消されないようにするため
	void Start(float strength, float duration);
	//残り時間を減らし、このフレームのずらし量を決める。描画の前に1回だけ呼ぶ
	void Update(float deltaTime);
	//このフレームのずらし量。揺れていなければ(0, 0)
	const Vector2& GetOffset() const { return _offset; }

private:
	//今の強さ。残り時間の割合で弱くしていく
	float GetCurrentStrength() const;

	float _strength = 0.0f;
	float _duration = 0.0f;
	float _remaining = 0.0f;
	Vector2 _offset{};
};
