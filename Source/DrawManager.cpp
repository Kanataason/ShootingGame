#include "DrawManager.h"

void DrawManager::Flush()
{
	for (auto& layer : _layers)
	{
		for (auto& draw : layer) draw();
		layer.clear();//clearしてもメモリは残るので、毎フレームの確保は起きない
	}
}
