#include "BaseScene.h"
#include "GameObject.h"

void BaseScene::Initialize(ServiceLocator& locator)
{
	_locator = locator;
	for (auto& obj : _objects)
		obj->Initialize(locator);
}

void BaseScene::Update(float deltaTime)
{
	for (auto& obj : _objects)
		obj->Update(deltaTime);

	//削除予約されたものをまとめて消す。ループ中に消すとイテレータが壊れるため
	std::erase_if(_objects, [](const std::unique_ptr<GameObject>& obj)
		{
			return obj->IsDestroyed();
		});

	//途中で生成されたものを、ここでまとめて追加する
	for (auto& obj : _pendingObjects)
	{
		obj->Initialize(_locator);
		_objects.push_back(std::move(obj));
	}
	_pendingObjects.clear();

	Draw();
}

void BaseScene::Draw()
{
	for (auto& obj : _objects)
		obj->Draw();
}
