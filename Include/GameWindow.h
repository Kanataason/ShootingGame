#pragma once
#include <DxLib.h>
#include <algorithm>
#include "Vector2.h"

struct ScreenBounds
{
    float width;
    float height;

    //画面内かどうか確認関数。marginの分だけ外側まで画面内とみなす
    bool Contains(const Vector2& position, float margin) const;

    //Y軸上で画面外に出ないようにする。offsetは機体の半径
    float ClampY(float wrappedPosition, float min, float max,float offset)const;

    //左から出たら右、右から出たら左へ
    Vector2 Wrap(const Vector2& position) const;
};

class GameWindow
{
public:
    GameWindow(HWND hwnd) :_handle(hwnd) {};
    HWND GetHandle() const { return _handle; }
    const ScreenBounds& GetBounds() const { return _bounds; }
   
    void OnResize(float newWidth, float newHeight) { _bounds = { newWidth, newHeight }; } 

private:
    HWND _handle = nullptr;
    ScreenBounds _bounds{};
};
