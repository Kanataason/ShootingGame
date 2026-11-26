#pragma once
#include "Vector2.h"

//点滅や撃破エフェクト、オーラなど、いろいろな所で使う演出をまとめたもの
class EffectUtillity
{
public:
	//被弾点滅：残り時間の間、一定間隔で表示と非表示を切り替える
	//残り時間が0以下なら常に表示する
	static bool IsBlinkVisible(float remainingTime, float interval = 0.08f);

	//渡された引数でCircleを作って死亡エフェクトとして描画する
	static void DrawEffect(float elapsed, float effectTime, int color, const Vector2& spownPos);

	//強化中のオーラ。段階が高いほど濃く太い
	//timeは経過時間。Time::GetWorldTime()を渡す
	static void DrawAura(const Vector2& center, float radius, int color, int level, float time);

	//弾が当たったときに一瞬白く光らせる時間（秒）
	static constexpr float HitFlashTime = 0.06f;
	//hitTimeは当たったときのTime::GetWorldTime()。まだ光らせる時間内ならtrue
	static bool IsHitFlashing(float hitTime);
	//画像を加算で上から重ねて、白っぽく光って見せる。普通に描いた後に呼ぶ
	static void DrawHitFlash(const Vector2& center, int image);
};
