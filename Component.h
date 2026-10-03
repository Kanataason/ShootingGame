#pragma once
#include <memory> 

#include "DxLib.h"
#include "ServiceLocator.h"

class GameObject;

class Component
{
public:
    virtual ~Component() {}
    //全員に自分のコピーを作れと約束させる
    virtual std::unique_ptr<Component> Clone() const = 0;
    virtual void Initialize(ServiceLocator& locator) =0;
    virtual void Update(float deltaTime) =0;
    virtual void Draw() {};
    GameObject* GetOwner() const
    {
        return _owner;
    }
    void SetOwner(GameObject* owner)
    {
        _owner = owner;
    }
    virtual void OnColliderEnter(float damage) {};
protected:
    GameObject* _owner = nullptr;
};