#pragma once
#include <vector>
#include "Datas.h"
#include "ServiceLocator.h"

class GameObjectFactory;
class BaseScene;

//読み込んだ出現データと、出現させた数をまとめたもの
struct SpawnEntry
{
	SpawnData data;
	int spawnedCount = 0;
	bool CanSpawn() const { return data.maxCount < 0 || spawnedCount < data.maxCount; }
};

//雑魚敵のランダム出現を担当する
//一定間隔ごとに、出現できるテンプレートから1つ選んでシーンに追加する
class EnemySpawner
{
public:
	void Initialize(const ServiceLocator& locator, std::vector<SpawnData> spawnData);
	void Update(float deltaTime, BaseScene& scene);

	//ボス出現時などに出現を止める
	void SetActive(bool isActive) { _isActive = isActive; }

	//ウェーブを切り替える。waveは1から数える
	//FirstWaveがwave以下の敵だけ出す
	void SetWave(int wave, const WaveData& data);

private:
	//次に出すまでの時間を、minInterval〜maxIntervalからランダムに決める
	void ResetTimer();

	GameObjectFactory* _factory = nullptr;
	std::vector<SpawnEntry> _entries;
	float _timer = 0.0f;
	float _minInterval = 2.0f;
	float _maxInterval = 3.5f;
	int _wave = 1;
	int _screenWidth = 640;
	bool _isActive = true;
};
