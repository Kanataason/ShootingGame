#include <map>
#include "ColliderManager.h"
#include "Collider.h"
#include "ColliderRegistry.h"
#include "Collision.h"

namespace
{
    //コライダーの種類の組み合わせごとに、押し戻し処理を呼び分けるための表
    using CollisionCheck = bool (*)(Collider& colliderA, Collider& colliderB);

    template<typename ColliderA, typename ColliderB>
    CollisionCheck MakeCollisionResolve()
    {
        return [](Collider& colliderA, Collider& colliderB) -> bool
            {
                return Collision::Resolve(static_cast<ColliderA*>(&colliderA),
                    static_cast<ColliderB*>(&colliderB));
            };
    }

    const std::map<std::pair<ColliderType, ColliderType>, CollisionCheck> collisionTable =
    {
        { { ColliderType::Circle, ColliderType::Circle }, MakeCollisionResolve<CircleCollider, CircleCollider>() },
        { { ColliderType::Box,    ColliderType::Circle }, MakeCollisionResolve<BoxCollider,    CircleCollider>() },
        { { ColliderType::Box, ColliderType::Box }, MakeCollisionResolve<BoxCollider, BoxCollider>() }
    };

    //逆向きも吸収して判定する
    bool ResolveCollision(Collider& colliderA, Collider& colliderB)
    {
        if (auto it = collisionTable.find({ colliderA.GetColliderType(), colliderB.GetColliderType() }); it != collisionTable.end())
            return it->second(colliderA, colliderB);

        if (auto it = collisionTable.find({ colliderB.GetColliderType(), colliderA.GetColliderType() }); it != collisionTable.end())
            return it->second(colliderB, colliderA);

        return false;  //未登録の組み合わせ
    }

    //アイテムとプレイヤーが重なっているか。押し戻しはしない
    //アイテムもプレイヤーも円なので、円どうしだけを見る
    bool IsTouching(const Collider& item, const Collider& player)
    {
        if (item.GetColliderType() != ColliderType::Circle || player.GetColliderType() != ColliderType::Circle)
            return false;

        const Vector2& itemCenter = item.GetOwner()->GetTransform().Position;
        const Vector2& playerCenter = player.GetOwner()->GetTransform().Position;
        const float itemRadius = static_cast<const CircleCollider&>(item).GetRadius();
        const float playerRadius = static_cast<const CircleCollider&>(player).GetRadius();

        //長さ0の線分と円の判定にして、半径の和で重なりを見る
        return Collision::CheckCollision(itemCenter, itemRadius, playerRadius, playerCenter, playerCenter);
    }

    //片方がアイテムなら、拾う処理だけをする。押し戻しとダメージはしない
    //アイテムどうし、アイテムと敵は何もしない
    void ResolvePickup(Collider& colliderA, Collider& colliderB)
    {
        Collider& item = (colliderA.GetTeam() == Team::Item) ? colliderA : colliderB;
        Collider& other = (&item == &colliderA) ? colliderB : colliderA;
        if (!item.IsCollisionEnabled()) return;   //拾われた直後のアイテム
        if (other.GetTeam() != Team::Player) return;
        if (!IsTouching(item, other)) return;

        //プレイヤーは拾ったアイテムの種類を読むので、お互いを相手として渡す
        item.GetOwner()->OnPickup(*other.GetOwner());
        other.GetOwner()->OnPickup(*item.GetOwner());
    }
}

void ColliderManager::Initialize(ServiceLocator& locator)
{
	_colliderRegistry = locator.colliderRegistry;
}

void ColliderManager::CheckCollisions(BulletPool& pool)
{
    //登録済みのコライダーを取得
    const auto& colliders = _colliderRegistry->GetColliders();
    if (colliders.empty()) { return; }

    //弾ごとに当たり判定をチェック
    pool.ForEachActive([&](Bullet& bullet)
        {
            //弾の先端と後端、太さの半分を計算
        const Vector2& bulletHead = bullet.position;
        const Vector2 bulletTail = bullet.GetTail();
        float halfThickness = bullet.thickness * 0.5f;

        for (Collider* targetCollider : colliders)
        {
            //当たり判定が無効なもの、同じチームには当たらないようにする
            if (!targetCollider->IsCollisionEnabled()) { continue; }
            if (targetCollider->GetTeam() == bullet.ownerTeam) { continue; }
            //アイテムは撃って消せないようにする
            if (targetCollider->GetTeam() == Team::Item) { continue; }

            const Vector2& targetPosition = targetCollider->GetOwner()->GetTransform().Position;
            bool isHit = false;

            //これ以上当たり判定の形は増える予定がないのでswitchで設定
            switch (targetCollider->GetColliderType())
            {
            case ColliderType::Circle:
            {
                auto* circleCollider = static_cast<CircleCollider*>(targetCollider);
                isHit = Collision::CheckCollision(targetPosition, circleCollider->GetRadius(),
                    halfThickness, bulletTail, bulletHead);
                break;
            }
            case ColliderType::Box:
            {
                auto* boxCollider = static_cast<BoxCollider*>(targetCollider);
                isHit = Collision::CheckCollision(targetPosition, boxCollider->GetHalfSize(),
                    halfThickness, bulletTail, bulletHead);
                break;
            }
            default:break;
            }

			//当たったら、当たった側のOnColliderEnterを呼んで弾を消す
            if (isHit)
            {
                targetCollider->GetOwner()->OnColliderEnter(bullet.damage);
                pool.Release(&bullet);
                break;
            }
        }
        });
}

void ColliderManager::ResolveCollider()
{
    const auto& colliders = _colliderRegistry->GetColliders();

    //全ペアを1回ずつ調べる
    for (std::size_t indexA = 0; indexA < colliders.size(); ++indexA)
    {
        for (std::size_t indexB = indexA + 1; indexB < colliders.size(); ++indexB)
        {
            Collider* colliderA = colliders[indexA];
            Collider* colliderB = colliders[indexB];

            //同じチームには当たらないようにする
            if (colliderA->GetTeam() == colliderB->GetTeam()) { continue; }

            //アイテムは、被弾後の無敵中でも拾えるようにしたいので、当たり判定の有効・無効を見る前に処理する
            if (colliderA->GetTeam() == Team::Item || colliderB->GetTeam() == Team::Item)
            {
                ResolvePickup(*colliderA, *colliderB);
                continue;
            }

            //当たり判定がないものには当たらないようにする
            if (!colliderA->IsCollisionEnabled() || !colliderB->IsCollisionEnabled()) { continue; }

            if (ResolveCollision(*colliderA, *colliderB))
            {
                float damage = 1.0f;
                //Playerなら1ダメージを与える
                Collider* damaged = (colliderA->GetTeam() == Team::Player) ? colliderA : colliderB;
                damaged->GetOwner()->OnColliderEnter(damage);
            }
        }
    }
}