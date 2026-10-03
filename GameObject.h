#pragma once
#include <iostream>
#include <vector>     
#include "Transform.h"
#include "Component.h"

//所持しているすべてのコンポーネントに処理を通知しているクラス
class GameObject
{
public:
    GameObject() = default;
    virtual ~GameObject() = default;

    //コピーコンストラクタ：コンポーネントを1つずつ複製する
    GameObject(const GameObject& source)
        : _transform(source._transform)
    {
        _components.reserve(source._components.size());
        for (const auto& sourceComponent : source._components)
        {
            AttachComponent(sourceComponent->Clone());
        }
    }

    //Moveは許可する。コピーは禁止
    GameObject(GameObject&&) = default;
    GameObject& operator=(GameObject&&) = default;
    GameObject& operator=(const GameObject&) = delete;

    virtual void Initialize(ServiceLocator& locator);
    virtual void Update(float deltaTime);
    virtual void Draw();

    template<typename T, typename... Args>
    T* AddComponent(Args&&... args)
    {
        static_assert(std::is_base_of_v<Component, T>, "T must derive from Component");

        auto component = std::make_unique<T>(std::forward<Args>(args)...);
        T* result = component.get();
        AttachComponent(std::move(component));
        return result;
    }

    template<typename T>
    T* GetComponent()
    {
        for (const auto& component : _components)
        {
            T* foundComponent = dynamic_cast<T*>(component.get());
            if (foundComponent != nullptr)
            {
                return foundComponent;
            }
        }
        return nullptr;
    }

    void SetTransform(const Transform& transform) { _transform = transform; }
    const Transform& GetTransform() const { return _transform; }
    Transform& GetTransform() { return _transform; }

    //何かに当たったら流れる. 今回は最初に弾が当たった時だけ通知を送りたいためほかの処理はしない
    void OnColliderEnter(float damage);

    //削除の予約だけ行う。実際の削除はシーンがフレームの最後にまとめて行う
    void Destroy() { _isDestroyed = true; }
    bool IsDestroyed() const { return _isDestroyed; }
protected:
    std::vector<std::unique_ptr<Component>> _components;
    Transform _transform{};

    bool _isDestroyed = false;
private:
    //AddComponentとコピーの両方から使う共通処理
    void AttachComponent(std::unique_ptr<Component> component)
    {
        component->SetOwner(this);
        _components.push_back(std::move(component));
    }
};