#pragma once
#include "Component.h"
#include "PlayerBuff.h"

class Primitive;
class Collider;
class DrawManager;
struct ScreenBounds;

//コマンドのアイテム
//上からまっすぐ落ちてきて、プレイヤーが触れると拾われる。画面の下まで落ちたら消える
//強化の種類を1つ持ち、その色で描く。拾われたときに何が起きるかはプレイヤー側が決めるので、ここでは自分を消すだけ
class ItemController : public Component
{
public:
	std::unique_ptr<Component> Clone()const override
	{
		return std::make_unique<ItemController>(*this);
	}

	void Initialize(ServiceLocator& locator)override;
	void Update(float deltaTime)override;
	void Draw()override;
	void OnPickup(GameObject& picker)override;

	//拾うと、この強化のコマンドが出る。出す側（GameScene）が作った直後に決める
	void SetBuffType(BuffType type) { _buffType = type; }
	BuffType GetBuffType() const { return _buffType; }

private:
	void DrawBody() const;
private:
	BuffType _buffType = BuffType::Attack;

	Primitive* _shape = nullptr;
	Collider* _collider = nullptr;
	DrawManager* _drawManager = nullptr;
	const ScreenBounds* _bounds = nullptr;
};
