#include <DxLib.h>

#include "BackgroundScroller.h"

BackgroundScroller::~BackgroundScroller()
{
	if (_image != -1) DeleteGraph(_image);
}

void BackgroundScroller::Initialize(float speed)
{
	_image = LoadGraph("Library/image/Ground.png");
	_speed = speed;
	GetDrawScreenSize(&_screenWidth, &_screenHeight);
}

void BackgroundScroller::Update(float deltaTime)
{
	_offsetY += _speed * deltaTime;
	if (_offsetY >= _screenHeight) _offsetY -= _screenHeight;   //1枚分ずれたら元の位置に戻す
}

void BackgroundScroller::Draw() const
{
	int y = static_cast<int>(_offsetY);
	//画面サイズに引き伸ばした画像を2枚、縦につなげて描画する
	DrawExtendGraph(0, y - _screenHeight, _screenWidth, y, _image, FALSE);   //上の1枚
	DrawExtendGraph(0, y, _screenWidth, y + _screenHeight, _image, FALSE);   //下の1枚
}
