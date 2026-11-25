#pragma once
#include <vector>
#include <functional>
#include "Datas.h"
#include "ServiceLocator.h"

class GameObject;
class GameObjectFactory;
class BaseScene;

//読み込んだ出現データと、出現させた数をまとめたもの
struct SpawnEntry
{
	SpawnData data;
	int spawnedCount = 0;
	bool CanSpawn() const { return data.maxCount < 0 || spawnedCount < data.maxCount; }
};

//雑魚敵とアイテムのランダム出現を担当する。敵用とアイテム用で1つずつ使う
//一定間隔ごとに、出現できるテンプレートから1つ選んでシーンに追加する
class EnemySpawner
{
public:
	void Initialize(const ServiceLocator& locator, std::vector<SpawnData> spawnData);
	void Update(float deltaTime, BaseScene& scene);

	//出したものを、シーンに加える前に渡す
	//スポナーは出すだけにして、撃破の監視やアイテムの種類決めは呼ぶ側にまかせるため
	void SetOnSpawned(std::function<void(GameObject&)> onSpawned) { _onSpawned = std::move(onSpawned); }

	//ボス出現時などに出現を止める
	void SetActive(bool isActive) { _isActive = isActive; }

	//ウェーブを切り替える。waveは1から数える
	//FirstWaveがwave以下の敵だけ出す
	void SetWave(int wave, const WaveData& data);

private:
	//次に出すまでの時間を、minInterval〜maxIntervalからランダムに決める
	void ResetTimer();

	GameObjectFactory* _factory = nullptr;
	std::function<void(GameObject&)> _onSpawned;
	std::vector<SpawnEntry> _entries;
	float _timer = 0.0f;
	float _minInterval = 2.0f;
	float _maxInterval = 3.5f;
	int _wave = 1;
	int _screenWidth = 640;
	bool _isActive = true;
};
