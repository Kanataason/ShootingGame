#pragma once
#include "SceneManager.h"
#include "InputManager.h"
#include "GameObjectfactory.h"
#include "BulletManager.h"
#include "GameWindow.h"
#include "PlayerProvider.h"
#include "ColliderRegistry.h"
#include "ColliderManager.h"
#include "DrawManager.h"
#include "SoundManager.h"
#include "GameSettings.h"
#include "ScreenShake.h"

#include "ServiceLocator.h"

//ゲーム全体で1つだけのサービスを所有し、毎フレームの処理順を決める
class GameManager
{
public:
	//画面の大きさをGameWindowに入れる。DxLib_Initの後に作ること
	explicit GameManager(HWND hwnd);
	//初期化関数
	void Initialize();

	//更新順はInput->SceneManager->BulletManager->Resolve->Collision->描画の登録->Flush
	void Update(float deltaTime);
private:
	ServiceLocator CreateLocator();

private:
	//メンバーは宣言と逆順に破棄される
	//シーン内のコライダーが破棄時にRegistryを使うので、Registry・Providerはシーンより先に宣言する
	GameWindow _gameWindow;
	//シーンをまたいで、タイトルで選んだ難易度を持つ
	GameSettings _settings;

	PlayerProvider _provider;
	ColliderRegistry _registry;
	ColliderManager _colliderManager;

	SceneManager _sceneManager;
	InputManager _inputManager;
	BulletManager _bulletManager;
	DrawManager _drawManager;
	ScreenShake _screenShake;
	SoundManager _soundManager;

	GameObjectFactory _gameObjectFactory;
};
