#include <string>

#include "GameScene.h"
#include "GameObject.h"
#include "Transform.h"

#include "SoundManager.h"
#include "JsonLoader.h"

#include "PlayerStatus.h"
#include "PlayerAttack.h"
#include "PlayerBuff.h"
#include "PlayerCommand.h"
#include "EnemyStatus.h"
#include "SceneManager.h"
#include "DrawManager.h"

void GameScene::Initialize(ServiceLocator& locator)
{
	//進行状況はこのシーンだけのものなので、コピーしたlocatorに追加して渡す
	//SceneManagerが持っている元のlocatorは書き換えない
	ServiceLocator sceneLocator = locator;
	sceneLocator.progress = &_progress;
	BaseScene::Initialize(sceneLocator);

	_background.Initialize(_scrollSpeed);
	JsonLoader loader;
	_bossData = loader.LoadBossData("JsonData/Boss.json");
	_spawner.Initialize(sceneLocator, loader.LoadSpawnData("JsonData/Spawn.json"));
	_waves = loader.LoadWaveData("JsonData/Spawn.json");
	_hud.Initialize(sceneLocator);

	locator.soundManager->PlayBgm(SoundId::BgmGame);

	SubScribeEvents();
	StartWave(0);
}
void GameScene::StartWave(size_t index)
{
	_waveIndex = index;
	_waveElapsed = 0.0f;

	const int waveNumber = static_cast<int>(index) + 1;//画面に出すのは1から
	_spawner.SetWave(waveNumber, _waves[index]);
	_hud.ShowBanner("WAVE " + std::to_string(waveNumber), GameHUD::WaveBannerColor);
}
void GameScene::UpdateWave(float deltaTime)
{
	if (_isBossSpawned) return;

	_waveElapsed += deltaTime;
	const WaveData& wave = _waves[_waveIndex];
	_hud.SetWave(static_cast<int>(_waveIndex) + 1, static_cast<int>(_waves.size()), wave.duration - _waveElapsed);
	if (_waveElapsed < wave.duration) return;

	if (_waveIndex + 1 < _waves.size())
	{
		StartWave(_waveIndex + 1);
		return;
	}

	//最後のウェーブが終わったら、雑魚を止めてボスを出す
	_isBossSpawned = true;
	_spawner.SetActive(false);
	_hud.ShowBanner("WARNING", GameHUD::BossBannerColor);
	SpawnBoss();
}
void GameScene::SubScribeEvents()
{
	//プレイヤー：シーンの初期化時に1回だけ
	GameObject* player = _locator.playerProvider->GetPlayer();
	if (!player) return;
	if (auto* playerStatus = player->GetComponent<PlayerStatus>())
	{
		_hud.SetPlayerHp(playerStatus->GetHealth(), playerStatus->GetMaxHealth());   //初期値
		playerStatus->OnHealthChanged.Subscribe([this](float hp, float maxHp) { _hud.SetPlayerHp(hp, maxHp); });

		playerStatus->OnDeathFinished.Subscribe([this]
			{
				_locator.sceneManager->RequestChange(SceneName::EGameOver);
			});
	}
	if (auto* playerAttack = player->GetComponent<PlayerAttack>())
	{
		playerAttack->OnHeatChanged.Subscribe([this](float heat, float max,bool isOver) 
			{ _hud.SetHeat(heat, max,isOver); });
	}
	if (auto* playerBuff = player->GetComponent<PlayerBuff>())
	{
		playerBuff->OnBuffChanged.Subscribe([this](BuffType type, int level, float remaining)
			{ _hud.SetBuff(type, level, remaining); });

		//予測の段階は、敵が攻撃予告の強さを決めるのに使う
		playerBuff->OnBuffChanged.Subscribe([this](BuffType type, int level, float)
			{
				if (type == BuffType::Predict) _locator.playerProvider->SetPredictLevel(level);
			});
	}
	if (auto* playerCommand = player->GetComponent<PlayerCommand>())
	{
		playerCommand->OnCommandChanged.Subscribe([this](const CommandView& command)
			{ _hud.SetCommand(command); });

		playerCommand->OnCommandResult.Subscribe([this](CommandResult result)
			{_hud.SetCommandResult(result); });
	}
}

void GameScene::SpawnBoss()
{
	auto boss = _locator.factory->CreateObject(_bossData);

	_locator.soundManager->PlayBgm(SoundId::BgmBoss);
	//ボス：生成したときに1回だけ
	if (auto* bossStatus = boss->GetComponent<EnemyStatus>())
	{
		_hud.SetBossHp(bossStatus->GetHealth(), bossStatus->GetMaxHealth());   //初期値
		bossStatus->OnHealthChanged.Subscribe([this](float hp, float maxHp) { _hud.SetBossHp(hp, maxHp); });

		bossStatus->OnDeathFinished.Subscribe([this]
			{
				_locator.sceneManager->RequestChange(SceneName::EGameClear);
			});
	}
	Spawn(std::move(boss));
}

void GameScene::DrawUI()
{
	_locator.drawManager->Register(DrawLayer::UI, [this] { _hud.Draw(); });
}

void GameScene::Update(float deltaTime)
{
	_background.Update(deltaTime);
	_locator.drawManager->Register(DrawLayer::Background, [this] { _background.Draw(); });
	_spawner.Update(deltaTime, *this);
	UpdateWave(deltaTime);

	BaseScene::Update(deltaTime);
}
