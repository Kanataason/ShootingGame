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

	//locatorを保存し、読み込み済みのオブジェクトを全部初期化する
	virtual void Initialize(ServiceLocator& locator);

	//全オブジェクトを更新し、削除予約されたものを消して、途中で生成されたものを追加する
	virtual void Update(float deltaTime);

	//UIをDrawManagerのUIレイヤーに登録する。GameManagerから毎フレーム呼ばれる
	virtual void DrawUI() {}

	//全オブジェクトの描画を登録する
	void Draw();

	//シーン読み込み時、Initializeの前にオブジェクトを追加する
	void AddObject(std::unique_ptr<GameObject> object) { _objects.push_back(std::move(object)); }

	//実行中にオブジェクトを追加する。次のUpdateの最後に追加される
	void Spawn(std::unique_ptr<GameObject> obj) { _pendingObjects.push_back(std::move(obj)); }
protected:
	ServiceLocator _locator{};

	std::vector<std::unique_ptr<GameObject>> _pendingObjects;
	std::vector<std::unique_ptr<GameObject>> _objects;
};
