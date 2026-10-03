#pragma once
#include <functional>
#include <vector>
#include <array>

//描画の順番。上から順に奥 -> 手前
enum class DrawLayer { Background, Enemy, Player, Bullet, Effect, UI, Count };

class DrawManager
{
public:
	//このフレームに描くものを登録する
	void Register(DrawLayer layer, std::function<void()> draw)
	{
		_layers[static_cast<size_t>(layer)].push_back(std::move(draw));
	};
	//レイヤーの順に全部描いて、登録を空にする（フレームの最後に1回）
	void Flush()
	{
		for (auto& layer : _layers)
		{
			for (auto& draw : layer) draw();
			layer.clear();//clearしてもメモリは残るので、毎フレームの確保は起きない
		}
	}
private:
	std::array<std::vector<std::function<void()>>, static_cast<size_t>(DrawLayer::Count)> _layers;
};