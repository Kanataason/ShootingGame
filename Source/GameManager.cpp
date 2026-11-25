#include "GameManager.h"
#include "JsonLoader.h"

GameManager::GameManager(HWND hwnd) :_gameWindow(hwnd), _inputManager(_gameWindow)
{
	int width = 0;
	int height = 0;
	GetDrawScreenSize(&width, &height);
	_gameWindow.OnResize((float)width, (float)height);
}

void GameManager::Initialize()
{
	//音はシーンをまたいで使うので、最初に1回だけ読み込む
	JsonLoader loader;
	const auto& soundJson = loader.LoadJsonData("JsonData/Sound.json");
	_soundManager.Initialize(loader.LoadSoundData(soundJson));
	//難易度の一覧も、タイトルで選ぶ前に読んでおく
	_settings.Initialize(loader.LoadDifficultyTable("JsonData/Difficulty.json"));

	//シーンマネージャーは最初のシーンを作るときにlocatorを渡すので、先にlocatorを作っておく
	auto context = CreateLocator();
	_sceneManager.Initialize(_gameObjectFactory,context);
	_bulletManager.Initialize(context);
	_colliderManager.Initialize(context);
}
void GameManager::Update(float deltaTime)
{
	//描画の順番は呼ぶ順番ではなく、DrawLayerで決まる
	_inputManager.Update();
	_sceneManager.Update(deltaTime);
	_bulletManager.Update(deltaTime);

	//移動が終わった後に当たり判定を行う
	_colliderManager.ResolveCollider();
	_colliderManager.CheckCollisions(_bulletManager.GetPool());

	//描画は登録だけ行い、最後にレイヤーの順でまとめて描く
	_drawManager.Register(DrawLayer::Bullet, [this] { _bulletManager.Draw(); });
	_sceneManager.DrawUI();
	_drawManager.Flush();
}

ServiceLocator GameManager::CreateLocator()
{
	//実体はすべてGameManagerのメンバー。locatorはポインタを配るだけなので、コピーして渡してよい
	//GameManagerより長く生きるものは無いので、ポインタが無効になることもない
	ServiceLocator locator{};
	locator.input = &_inputManager;
	locator.bulletManager = &_bulletManager;
	locator.drawManager = &_drawManager;
	locator.gameWindow = &_gameWindow;
	locator.playerProvider = &_provider;
	locator.colliderRegistry = &_registry;
	locator.sceneManager = &_sceneManager;
	locator.factory = &_gameObjectFactory;
	locator.soundManager = &_soundManager;
	locator.gameSettings = &_settings;

	return locator;
}