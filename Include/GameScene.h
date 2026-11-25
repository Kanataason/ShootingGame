#pragma once
#include <vector>

#include "BaseScene.h"
#include "BackgroundScroller.h"
#include "EnemySpawner.h"
#include "ItemDropper.h"
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

	//雑魚が倒されたら、その位置でアイテムを落とすかを決められるようにする
	void WatchEnemyDeath(GameObject& enemy);
	//落とすと決まったら、その位置にアイテムを出す
	void DropItem(const Vector2& position);
	//ボス戦で上から降らせるアイテムの種類を決める
	void SetFallingItemType(GameObject& item);
	//プレイヤーのHPと最大HP。アイテムの種類（ピンチなら回復）を決めるのに使う
	void GetPlayerHp(float& hp, float& maxHp) const;
private:
	ObjectData _bossData;   //Initializeで読み込んでおき、出現時に使う
	DifficultyData _difficulty;//タイトルで選んだ難易度の倍率

	float _scrollSpeed = 120.0f;   //背景のスクロール速度

	GameHUD _hud;
	bool _isBossSpawned = false;

	std::vector<WaveData> _waves; //Spawn.jsonのWaves。この順に進む
	size_t _waveIndex = 0;        //今のウェーブ（0から）
	float _waveElapsed = 0.0f;    //今のウェーブが始まってからの秒数

	EnemySpawner _spawner;
	//雑魚を倒したときにアイテムを落とす
	ItemDropper _itemDropper;
	//ボス戦では雑魚がいなくてアイテムが落ちないので、上から降らせる。それまでは止めておく
	EnemySpawner _itemSpawner;
	WaveData _bossItemFall;   //Items.jsonのDropにある、ボス戦で降らせる間隔
	BackgroundScroller _background;
};