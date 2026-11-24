#pragma once
#include <DxLib.h>
#include "Vector2.h"
#include "ServiceLocator.h"
#include "GameWindow.h"
#include <vector>

enum class InputKeyName
{
	up =1,
	down = -1,
	left = -2,
	right = 2,
	none = 0
};

struct InputCommand
{
	std::vector<InputKeyName> commands;
};
struct PlayerInput
{
	Vector2 direction;
	bool isShotHeld = false;//押している間ずっと撃つ
	bool isChangePressed = false; //押した瞬間だけ

	InputCommand commandList;
};


class InputManager
{
public:
	explicit InputManager(GameWindow& gameWindow);

	void Update();

	bool GetKey(int key)const;
	bool GetKeyDown(int key)const;

private:
	POINT GetScreenCenter();
	//カーソルを画面の中央に戻す。クリックでほかのウィンドウに移らないようにするため
	void KeepCursorCenter(bool isForeground);
private:
	bool _current[256]{};
	bool _previous[256]{};

	GameWindow* _gameWindow = nullptr;
};