#pragma once
#include <DxLib.h>
#include "Vector2.h"
#include "ServiceLocator.h"
#include "GameWindow.h"

struct PlayerInput
{
	Vector2 direction;
	bool isShotHeld = false;//押している間ずっと撃つ
	bool isChangePressed = false; //押した瞬間だけ
};

class InputManager
{
public:
public:
	InputManager(GameWindow& gameWindow):_gameWindow(&gameWindow)
	{
		GetDrawScreenSize(&screenWidth, &screenHeight);
		POINT center ={ screenWidth / 2,screenHeight / 2};

		SetCursorPos(center.x, center.y);
	};

	void Initialize(ServiceLocator& locator);
	void Update();

	bool GetKey(int key)const;
	bool GetKeyEnter(int key)const;
	bool GetKeyDown(int key)const;
	bool AnyKey()const;

	POINT GetMousePosition() const;
	POINT GetMouseDelta() const;

private:
	POINT GetScreenCenter();
	void UpdateMouseDelta(bool isForeground);

	bool IsMouseInsideWindow(const POINT& mousePos)const
	{
		return mousePos.x > screenWidth ||
			mousePos.x < 0 ||
			mousePos.y > screenHeight ||
			mousePos.y < 0;
	}
private:
	bool _current[256]{};
	bool _previous[256]{};


	int screenWidth = 0;
	int screenHeight = 0;

	GameWindow* _gameWindow = nullptr;
	POINT _mouseDelta{};
};