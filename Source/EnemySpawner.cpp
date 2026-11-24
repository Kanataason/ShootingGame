#include <DxLib.h>

#include "EnemySpawner.h"
#include "GameObjectfactory.h"
#include "GameWindow.h"
#include "BaseScene.h"

void EnemySpawner::Initialize(const ServiceLocator& locator, std::vector<SpawnData> spawnData)
{
	_factory = locator.factory;
	for (auto& data : spawnData)
		_entries.push_back({ std::move(data) });
	_screenWidth = static_cast<int>(locator.gameWindow->GetBounds().width);
	ResetTimer();
}

void EnemySpawner::Update(float deltaTime, BaseScene& scene)
{
	if (!_isActive || _entries.empty()) return;

	_timer -= deltaTime;
	if (_timer > 0.0f) return;
	ResetTimer();

	//出現数の上限に達したものと、このウェーブではまだ出ないものを、先に候補から外す
	//選んでから出せないと分かると、その回は何も出ず、出現間隔がばらつくため
	std::vector<SpawnEntry*> candidates;
	candidates.reserve(_entries.size());
	for (auto& entry : _entries)
		if (entry.CanSpawn() && entry.data.firstWave <= _wave) candidates.push_back(&entry);
	if (candidates.empty()) return;

	//候補の中からランダムに1つ選ぶ
	SpawnEntry& entry = *candidates[GetRand(static_cast<int>(candidates.size()) - 1)];
	++entry.spawnedCount;

	ObjectData objectData = entry.data.objectData;
	if (entry.data.isRandomX)
		objectData.transform.Position.x = static_cast<float>(40 + GetRand(_screenWidth - 80));

	scene.Spawn(_factory->CreateObject(objectData));
}

void EnemySpawner::SetWave(int wave, const WaveData& data)
{
	//ウェーブが変わったので、出現間隔をリセットする
	_wave = wave;
	_minInterval = data.minInterval;
	_maxInterval = data.maxInterval;
	ResetTimer();
}

void EnemySpawner::ResetTimer()
{
	_timer = _minInterval + GetRand(static_cast<int>((_maxInterval - _minInterval) * 100)) / 100.0f;
}
