#pragma once
#include <vector>

#include "BaseScene.h"
#include "BackgroundScroller.h"
#include "EnemySpawner.h"
#include "GameHUD.h"

//ゲーム本編
//背景スクロール・雑魚の出現・ウェーブの進行・ボス出現の判定・UIを持つ
//ウェーブは時間で進み、最後のウェーブが終わるとボスが出る
class GameScene : public BaseScene
{
public:
	void Initialize(ServiceLocator& locator)override;

	//更新順->バックグラウンド->バックグラウンドDraw->ランダムスポナー->Update
	void Update(float deltaTime)override;

	void DrawUI()override;
private:
	void SubScribeEvents();
	//ボスを生成して、HPバーとGameClearへの遷移をつなぐ
	void SpawnBoss();
	//ウェーブの時間を進め、終わったら次のウェーブかボスへ移る
	void UpdateWave(float deltaTime);
	//index番目（0から）のウェーブを始める
	void StartWave(size_t index);
private:
	ObjectData _bossData;   //Initializeで読み込んでおき、出現時に使う

	float _scrollSpeed = 120.0f;   //背景のスクロール速度

	//シーンが持つので、リトライで作り直されると自動で0に戻る
	GameProgress _progress;
	GameHUD _hud;
	bool _isBossSpawned = false;

	std::vector<WaveData> _waves; //Spawn.jsonのWaves。この順に進む
	size_t _waveIndex = 0;        //今のウェーブ（0から）
	float _waveElapsed = 0.0f;    //今のウェーブが始まってからの秒数

	EnemySpawner _spawner;
	BackgroundScroller _background;
};