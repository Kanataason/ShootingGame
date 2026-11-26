#pragma once
#include <functional>
#include <vector>
#include <array>

#include "Vector2.h"

//描画の順番。上から順に奥 -> 手前
enum class DrawLayer { Background, Enemy, Player, Bullet, Effect, UI, Count };

class DrawManager
{
public:
	~DrawManager();

	//UI以外を描くための、画面と同じ大きさの描画先を作る。DxLib_Initの後に呼ぶ
	void Initialize(int screenWidth, int screenHeight);

	//このフレームに描くものを登録する
	void Register(DrawLayer layer, std::function<void()> draw)
	{
		_layers[static_cast<size_t>(layer)].push_back(std::move(draw));
	};
	//レイヤーの順に全部描いて、登録を空にする（フレームの最後に1回）
	//UI以外はworldOffsetだけずらして描く。画面を揺らすときに使い、HUDは揺らさない
	void Flush(const Vector2& worldOffset);
private:
	//1つのレイヤーに登録されたものを全部描いて、登録を空にする
	void FlushLayer(DrawLayer layer);

	std::array<std::vector<std::function<void()>>, static_cast<size_t>(DrawLayer::Count)> _layers;
	int _worldScreen = -1;//UI以外を一度ここに描いてから、ずらして画面に貼る
};
