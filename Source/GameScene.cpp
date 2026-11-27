#include <string>
#include <algorithm>
#include <cmath>

#include "GameScene.h"
#include "GameObject.h"
#include "Transform.h"

#include "SoundManager.h"
#include "JsonLoader.h"

#include "PlayerStatus.h"
#include "PlayerProvider.h"
#include "PlayerAttack.h"
#include "PlayerBuff.h"
#include "PlayerCommand.h"
#include "EnemyStatus.h"
#include "ItemController.h"
#include "SceneManager.h"
#include "InputManager.h"
#include "DrawManager.h"
#include "GameSettings.h"
#include "ScreenShake.h"

namespace
{
	//画面の揺れの強さ（ピクセル）と長さ（秒）
	struct ShakeSetting
	{
		float strength;
		float duration;
	};
	constexpr ShakeSetting EnemyDefeatShake = { 3.0f, 0.15f };
	constexpr ShakeSetting PlayerDamageShake = { 6.0f, 0.25f };
	constexpr ShakeSetting BossDefeatShake = { 12.0f, 0.8f };

	//出る間隔に倍率を掛けたウェーブ
	WaveData ScaleInterval(WaveData wave, float rate)
	{
		wave.minInterval *= rate;
		wave.maxInterval *= rate;
		return wave;
	}

	//アイテムの落とし方に、難易度の倍率を掛ける
	ItemDropData ApplyDifficulty(ItemDropData dropData, const DifficultyData& difficulty)
	{
		//確率は1を超えないように、天井は1体より少なくならないようにする
		dropData.dropChance = (std::min)(dropData.dropChance * difficulty.dropChanceRate, 1.0f);
		dropData.pityCount = (std::max)(static_cast<int>(std::lround(dropData.pityCount * difficulty.pityCountRate)), 1);
		//ボス戦で降る間隔は、落ちやすい難易度ほど短くする
		dropData.bossFall = ScaleInterval(dropData.bossFall, 1.0f / difficulty.dropChanceRate);
		return dropData;
	}
}

void GameScene::Initialize(ServiceLocator& locator)
{
	BaseScene::Initialize(locator);
	//タイトルで選んだ難易度。シーンの間は変わらないので、コピーして持っておく
	_difficulty = locator.gameSettings->GetDifficulty();

	_background.Initialize(_scrollSpeed);
	JsonLoader loader;
	//ボスの情報はJsonから読み込んでおき、出現時に使う
	_bossData = loader.LoadBossData("JsonData/Boss.json");
	_spawner.Initialize(locator,loader.LoadSpawnData("JsonData/Spawn.json"));
	_waves = loader.LoadWaveData("JsonData/Spawn.json");
	//出した雑魚は、倒されたらアイテムを落とせるように見張る
	_spawner.SetOnSpawned([this](GameObject& enemy) { WatchEnemyDeath(enemy); });

	//アイテムも敵と同じ形のJSONなので、同じ読み込みを使う
	std::vector<SpawnData> itemTemplates = loader.LoadSpawnData("JsonData/Items.json");
	if (itemTemplates.empty())
		throw std::runtime_error("Items.json has no Templates.");
	const ItemDropData dropData = ApplyDifficulty(loader.LoadItemDropData("JsonData/Items.json"), _difficulty);
	_itemDropper.Initialize(locator, itemTemplates.front().objectData, dropData);

	//ボス戦で降らせる分。ボスが出るまでは止めておく
	_bossItemFall = dropData.bossFall;
	_itemSpawner.Initialize(locator, std::move(itemTemplates));
	_itemSpawner.SetOnSpawned([this](GameObject& item) { SetFallingItemType(item); });
	_itemSpawner.SetActive(false);

	_hud.Initialize(locator);
	_hud.SetDifficultyName(_difficulty.name);

	locator.soundManager->PlayBgm(SoundId::BgmGame);

	SubScribeEvents();
	StartWave(0);
}
void GameScene::StartWave(size_t index)
{
	_waveIndex = index;
	_waveElapsed = 0.0f;

	const int waveNumber = static_cast<int>(index) + 1;//画面に出すのは1から
	//難易度で、雑魚が出る間隔だけを変える。ウェーブの長さは変えない
	_spawner.SetWave(waveNumber, ScaleInterval(_waves[index], _difficulty.spawnIntervalRate));
	_hud.ShowBanner("WAVE " + std::to_string(waveNumber), GameHUD::WaveBannerColor);
}
void GameScene::UpdateWave(float deltaTime)
{
	if (_isBossSpawned) return;

	//ウェーブの経過時間を進める
	_waveElapsed += deltaTime;
	const WaveData& wave = _waves[_waveIndex];
	//ウェーブの残り時間をHUDに渡す。秒の表示はHUD側で切り上げる（0.3秒残りなら「あと1秒」）
	_hud.SetWave(static_cast<int>(_waveIndex) + 1, static_cast<int>(_waves.size()), wave.duration - _waveElapsed);
	if (_waveElapsed < wave.duration) return;

	//次のウェーブがあれば、そこへ進む
	if (_waveIndex + 1 < _waves.size())
	{
		StartWave(_waveIndex + 1);
		return;
	}

	//最後のウェーブが終わったら、雑魚を止めてボスを出す
	_isBossSpawned = true;
	_spawner.SetActive(false);
	//雑魚がいなくなるとアイテムが落ちないので、代わりに上から降らせ始める
	_itemSpawner.SetWave(1, _bossItemFall);
	_itemSpawner.SetActive(true);
	_hud.ShowBanner("WARNING", GameHUD::BossBannerColor);
	SpawnBoss();
}
void GameScene::WatchEnemyDeath(GameObject& enemy)
{
	auto* enemyStatus = enemy.GetComponent<EnemyStatus>();
	if (enemyStatus == nullptr) return;

	//HPが0以下になった通知で、倒されたと分かる
	//死亡演出中はダメージを受けないので、0以下の通知は1体につき1回だけ来る
	//画面の下へ通り抜けて消えた敵は、HPが減らないので落とさない
	enemyStatus->OnHealthChanged.Subscribe([this, &enemy](float hp, float)
		{
			if (hp > 0.0f) return;
			//倒した手ごたえとして小さく揺らす。数が多いので、揺れで画面が見にくくならない強さにする
			_locator.screenShake->Start(EnemyDefeatShake.strength, EnemyDefeatShake.duration);
			DropItem(enemy.GetTransform().Position);
		});
}
void GameScene::DropItem(const Vector2& position)
{
	float hp = 0.0f;
	float maxHp = 0.0f;
	GetPlayerHp(hp, maxHp);

	//当たり判定の途中で呼ばれるので、シーンにはSpawnで予約だけして、次のUpdateの最後に加える
	if (auto item = _itemDropper.TryDrop(position, hp, maxHp))
		Spawn(std::move(item));
}
void GameScene::SetFallingItemType(GameObject& item)
{
	//アイテム用のスポナーから呼ばれるので、アイテム以外は来ないはずだが、念のため確かめる
	auto* itemController = item.GetComponent<ItemController>();
	if (itemController == nullptr) return;

	//種類の決め方は、雑魚が落とすアイテムと同じにする（ピンチなら回復）
	//決め方をItemDropperの1か所にまとめて、ボス戦だけ違う種類が出ることがないようにする
	float hp = 0.0f;
	float maxHp = 0.0f;
	GetPlayerHp(hp, maxHp);
	itemController->SetBuffType(_itemDropper.ChooseType(hp, maxHp));
}
void GameScene::GetPlayerHp(float& hp, float& maxHp) const
{
	//プレイヤーが見つからないときは、ピンチ扱いにならない値にする
	hp = 1.0f;
	maxHp = 1.0f;
	//GameSceneはプレイヤーを直接持たないので、ほかのクラスと同じくPlayerProviderから探す
	GameObject* player = _locator.playerProvider->GetPlayer();
	if (player == nullptr) return;
	//HPはPlayerStatusが持つ。今のHPと最大HPの両方を返し、割合の計算は呼ぶ側にまかせる
	if (const auto* playerStatus = player->GetComponent<PlayerStatus>())
	{
		hp = playerStatus->GetHealth();
		maxHp = playerStatus->GetMaxHealth();
	}
}

void GameScene::SubScribeEvents()
{
	//シーンに最初から置いてある雑魚も、倒されたらアイテムを落とすようにする
	for (auto& object : _objects)
		WatchEnemyDeath(*object);

	//プレイヤー：シーンの初期化時に1回だけ
	GameObject* player = _locator.playerProvider->GetPlayer();
	if (!player) return;
	if (auto* playerStatus = player->GetComponent<PlayerStatus>())
	{
		_hud.SetPlayerHp(playerStatus->GetHealth(), playerStatus->GetMaxHealth());   //初期値
		playerStatus->OnHealthChanged.Subscribe([this](float hp, float maxHp) { _hud.SetPlayerHp(hp, maxHp); });
		//被弾したら揺らして、点滅に気づく前でも「くらった」と分かるようにする
		//回復では揺らさないように、HPの変化ではなくダメージの通知を使う
		playerStatus->OnDamaged.Subscribe([this]
			{
				_locator.screenShake->Start(PlayerDamageShake.strength, PlayerDamageShake.duration);
			});

		//死亡時にGameOverへ遷移する
		playerStatus->OnDeathFinished.Subscribe([this]
			{
				_locator.sceneManager->RequestChange(SceneName::EGameOver);
			});
	}
	if (auto* playerAttack = player->GetComponent<PlayerAttack>())
	{
		//ヒート値の変化をHUDに渡す
		playerAttack->OnHeatChanged.Subscribe([this](float heat, float max,bool isOver) 
			{ _hud.SetHeat(heat, max,isOver); });
	}

	if (auto* playerBuff = player->GetComponent<PlayerBuff>())
	{
		//強化の段階と残り時間をHUDに渡す
		playerBuff->OnBuffChanged.Subscribe([this](BuffType type, int level, float remaining)
			{ _hud.SetBuff(type, level, remaining); });

		//予測の段階を敵に渡す
		playerBuff->OnBuffChanged.Subscribe([this](BuffType type, int level, float)
			{
				if (type == BuffType::Predict) _locator.playerProvider->SetPredictLevel(level);
			});
	}
	if (auto* playerCommand = player->GetComponent<PlayerCommand>())
	{
		//コマンドの候補や入力中の方向列をHUDに渡す
		playerCommand->OnCommandChanged.Subscribe([this](const CommandView& command)
			{ _hud.SetCommand(command); });

		//コマンドの結果をHUDに渡す
		playerCommand->OnCommandResult.Subscribe([this](CommandResult result)
			{_hud.SetCommandResult(result); });
	}
}

void GameScene::SpawnBoss()
{
	//読み込んだデータは変えずに、出すときのコピーにだけ難易度の倍率を掛ける
	ObjectData bossData = _bossData;
	bossData.statusData.health *= _difficulty.bossHealthRate;
	auto boss = _locator.factory->CreateObject(bossData);

	_locator.soundManager->PlayBgm(SoundId::BgmBoss);
	//ボス：生成したときに1回だけ
	if (auto* bossStatus = boss->GetComponent<EnemyStatus>())
	{
		_hud.SetBossHp(bossStatus->GetHealth(), bossStatus->GetMaxHealth());   //初期値
		bossStatus->OnHealthChanged.Subscribe([this](float hp, float maxHp) { _hud.SetBossHp(hp, maxHp); });
		//ボスを倒した瞬間は、雑魚より大きく長く揺らして、最後の山を越えたことを伝える
		bossStatus->OnHealthChanged.Subscribe([this](float hp, float)
			{
				if (hp <= 0.0f) _locator.screenShake->Start(BossDefeatShake.strength, BossDefeatShake.duration);
			});
		//死亡時にGameClearへ遷移する
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
	//背景スクロールはUpdateの最初にやる。DrawはDrawManagerに登録する
	_background.Update(deltaTime);
	_locator.drawManager->Register(DrawLayer::Background, [this] { _background.Draw(); });
	//出したものはSpawnで予約され、下のBaseScene::Updateの最後にシーンへ加わる
	_spawner.Update(deltaTime, *this);
	_itemSpawner.Update(deltaTime, *this);
	UpdateWave(deltaTime);

	//シーン上のオブジェクトを全部動かし、消すものを消して、予約したものを加える
	BaseScene::Update(deltaTime);

	//Escでタイトルへ戻る。いきなりゲームを終わらせず、もう一度Escを押したときだけ終わるようにする
	if (_locator.input->GetKeyDown(VK_ESCAPE))
		_locator.sceneManager->RequestChange(SceneName::EGameTitle);
}
