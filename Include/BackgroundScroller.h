#pragma once

//縦スクロールする背景。同じ画像を2枚縦につなげてループさせる
class BackgroundScroller
{
public:
	~BackgroundScroller();

	void Initialize(float speed);
	void Update(float deltaTime);
	void Draw() const;

private:
	int _image = -1;
	float _speed = 0.0f;
	float _offsetY = 0.0f;
	int _screenWidth = 0;
	int _screenHeight = 0;
};
