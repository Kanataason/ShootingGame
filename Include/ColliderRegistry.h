#pragma once
#include <vector>

class Collider;

//Itemは弾に当たらず、押し戻しもしない。プレイヤーが触れたら拾われる
enum class Team { Player, Enemy, Item, None };

//シーン中のコライダーのポインタを所持してほかで使えるようにする
class ColliderRegistry
{
public:
    void Register(Collider* collider) { _colliders.push_back(collider); }
    void Unregister(Collider* collider) { std::erase(_colliders, collider); } 
    const std::vector<Collider*>& GetColliders() const { return _colliders; }
private:
    std::vector<Collider*> _colliders;
};