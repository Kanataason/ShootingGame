#pragma once
#include <vector>
#include "SceneLoader.h"
#include "GameObjectfactory.h"
#include "Datas.h"

class GameObject;

//シーンの基底クラス
//シーン上のオブジェクトの追加・更新・削除・描画を管理する
class BaseScene
{
public:
	virtual ~BaseScene() = default;
	BaseScene() = default;

	virtual void Initialize(ServiceLocator& locator)
	{
		_locator = locator;
		for (auto& obj : _objects)
			obj->Initialize(locator);
	}

	virtual void Update(float deltaTime)
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

	//UIをDrawManagerのUIレイヤーに登録する。GameManagerから毎フレーム呼ばれる
	virtual void DrawUI() {}

	void Draw()
	{
		for (auto& obj : _objects)
			obj->Draw();
	}

	//シーン読み込み時、Initializeの前にオブジェクトを追加する
	void AddObject(std::unique_ptr<GameObject> object) { _objects.push_back(std::move(object)); }

	//実行中にオブジェクトを追加する。次のUpdateの最後に追加される
	void Spawn(std::unique_ptr<GameObject> obj) { _pendingObjects.push_back(std::move(obj)); }
protected:
	ServiceLocator _locator{};

	std::vector<std::unique_ptr<GameObject>> _pendingObjects;
	std::vector<std::unique_ptr<GameObject>> _objects;
};
