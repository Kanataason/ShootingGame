#pragma once
#include "Component.h"

//各サイズデータの関数を共通に使えるように定義
class Primitive :public Component
{
public:
	std::unique_ptr<Component> Clone()const override
	{
		return std::make_unique<Primitive>(*this);
	}
	void Initialize(ServiceLocator& locator)override {};
	void Update(float deltaTime)override {};

	virtual float GetRadius()const { return 0; }
	virtual Vector2 GetHalfSize() const { return{}; }
};