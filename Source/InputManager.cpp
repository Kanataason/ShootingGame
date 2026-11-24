#include "InputManager.h"

InputManager::InputManager(GameWindow& gameWindow) :_gameWindow(&gameWindow)
{
    POINT center = GetScreenCenter();
    SetCursorPos(center.x, center.y);
}

POINT InputManager::GetScreenCenter()
{
    int screenWidth = 0;
    int screenHeight = 0;
    GetDrawScreenSize(&screenWidth, &screenHeight);

    return { screenWidth / 2, screenHeight / 2 };
}

void InputManager::Update()
{
    if (GetKey(VK_ESCAPE))
        PostQuitMessage(0);

    //前のフレームを保存して、現在のフレームを更新する
    memcpy(_previous, _current, sizeof(_current));

    const bool isForeground = (GetForegroundWindow() == _gameWindow->GetHandle());

    for (int keyCode = 0; keyCode < 256; ++keyCode)
    {
        //別ウィンドウを操作中のキー入力は拾わない
        _current[keyCode] = isForeground && (GetAsyncKeyState(keyCode) & 0x8000);
    }

    KeepCursorCenter(isForeground);
}
void InputManager::KeepCursorCenter(bool isForeground)
{
    if (!isForeground) { return; }

    POINT center = GetScreenCenter(); //クライアント座標
    ClientToScreen(_gameWindow->GetHandle(), &center);//スクリーン座標に揃える
    SetCursorPos(center.x, center.y);
}
bool InputManager::GetKey(int key)const
{
    return _current[key];
}
bool InputManager::GetKeyDown(int key)const
{
    return _current[key] && !_previous[key];
}