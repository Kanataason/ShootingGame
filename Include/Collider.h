#pragma once
#include "Component.h"
#include "ColliderRegistry.h"
#include "Vector2.h"

#include "GameObject.h"

enum class ColliderType
{
    Circle,
    Box,
    Count
};

//当たり判定の基底クラス
//Initializeでレジストリに登録し、破棄されるときに自動で登録解除する
class Collider : public Component
{
public:
    explicit Collider(Team team) : _team(team) {}
    Collider(const Collider& source)
        : Component(source), _team(source._team) {
    }  //登録情報はコピーしない

    ~Collider() override;

    void Initialize(ServiceLocator& locator) override;

    void SetColliderType(ColliderType type) { _colliderType = type; }
    const ColliderType& GetColliderType()const { return _colliderType; }

    Team GetTeam() const { return _team; }
    GameObject* GetOwner() const { return _owner; }

    //無敵中や撃破演出中など、falseの間は弾との判定・押し戻しの両方から外れる
    //アイテムを拾う判定だけは、プレイヤーがfalseでも行う
    void SetCollisionEnabled(bool isEnabled) { _isCollisionEnabled = isEnabled; }
    bool IsCollisionEnabled()const { return _isCollisionEnabled; }

private:
    bool _isCollisionEnabled = true;
    Team _team;
    ColliderType _colliderType = ColliderType::Count;
    ColliderRegistry* _registry = nullptr;
};