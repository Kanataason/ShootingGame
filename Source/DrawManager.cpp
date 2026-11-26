#include <DxLib.h>

#include "DrawManager.h"

DrawManager::~DrawManager()
{
	if (_worldScreen != -1) DeleteGraph(_worldScreen);
}

void DrawManager::Initialize(int screenWidth, int screenHeight)
{
	//背景も描くので透明は要らない。FALSEにすると描画が少し軽い
	_worldScreen = MakeScreen(screenWidth, screenHeight, FALSE);
}

void DrawManager::Flush(const Vector2& worldOffset)
{
	//UI以外は一度別の画面に描き、まとめてずらして貼る
	//1つずつの描画にずらし量を足すと、全部の描画処理を書き換えることになるため
	SetDrawScreen(_worldScreen);
	ClearDrawScreen();
	for (size_t index = 0; index < static_cast<size_t>(DrawLayer::UI); ++index)
		FlushLayer(static_cast<DrawLayer>(index));

	//本来の画面に戻して、ずらした位置に貼る。ずれてできた端のすき間は黒のまま
	SetDrawScreen(DX_SCREEN_BACK);
	DrawGraph(static_cast<int>(worldOffset.x), static_cast<int>(worldOffset.y), _worldScreen, FALSE);

	//HUDは読めなくならないように、揺らさずに上から描く
	FlushLayer(DrawLayer::UI);
}

void DrawManager::FlushLayer(DrawLayer layer)
{
	auto& draws = _layers[static_cast<size_t>(layer)];
	for (auto& draw : draws) draw();
	draws.clear();//clearしてもメモリは残るので、毎フレームの確保は起きない
}
