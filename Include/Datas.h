#pragma once
#include <iostream>
#include <string>
#include <functional>
#include <variant>
#include <vector>

#include "Transform.h"

enum class SceneName
{
	EGameTitle,
	EGameScene,
	EGameOver,
	EGameClear,
	None
};
enum class ObjectType
{
	Player,
	Enemy,
	Boss,
	Item,
	None
};
enum class CharacterType
{
	Player,
	MoveEnemy,
	ReflectShotEnemy,
	SpreadEnemy,
	Boss,
	Item,
	None
};
enum class MovePattern
{
	EnterAndStay, 
	PassThrough
};

class BaseScene;

enum class SoundId
{
	PlayerShot,
	PlayerShotGun,
	PlayerHit, 
	CharacterExplosion,
	BossExplosion,
	//コマンド成功時
	CommandAttack,
	CommandDefense,
	CommandSpeed,
	CommandPredict,
	CommandHeal,
	BgmGame,
	BgmBoss,
	BgmGameClear,
	Count
};
//すべてJsonからの読みだした値を保存するのに使う
struct SoundData
{
	SoundId id;
	std::string path;
	int volume = 255;
	int bufferNum = 1;
};

struct SceneData
{
	SceneName name = SceneName::None;
	std::string path = "";
	std::function<std::unique_ptr<BaseScene>()> scene{};
};

struct SphereData
{
	float radius = 0.0f;
};
struct BoxData
{
	Vector2 halfSize{};
};
using SizeData = std::variant<SphereData, BoxData>;

struct StatusData
{
	CharacterType characterType = CharacterType::None;
	MovePattern movePattern = MovePattern::EnterAndStay;

	std::string imagePath = "";
	bool isMove = true;
	float health = 0;
};

//オブジェクトのデータ構造体
struct ObjectData
{
	ObjectType name = ObjectType::None;
	std::vector<int> attackType;
	Transform transform{};
	SizeData sizeData{};

	StatusData statusData{};
};
struct SpawnData
{
	ObjectData objectData;   //敵そのものの情報
	bool isRandomX = true;   //出現のルール
	int maxCount = -1;       //-1 = 無制限
	int firstWave = 1;       //このウェーブから出始める
};
//1ウェーブ分の長さと、雑魚が出る間隔（秒）
struct WaveData
{
	float duration = 40.0f;
	float minInterval = 2.0f;
	float maxInterval = 3.5f;
};
//アイテムの出し方。Items.jsonのDropから読む
struct ItemDropData
{
	float dropChance = 0.3f;    //雑魚を倒したときに落とす確率（0〜1）
	int pityCount = 4;          //この数だけ続けて落ちなかったら、次は必ず落とす
	float pinchHpRatio = 0.3f;  //HPがこの割合以下なら、回復のアイテムを落とす
	WaveData bossFall{};        //ボス戦で上から降らせる間隔。durationは使わない
};
//難易度1つ分の倍率。Normalを基準（すべて1）にして、ほかの難易度は倍率だけで違いを出す
//倍率を掛ける側は、どの難易度かを知らずに、自分に必要な倍率だけを読む
struct DifficultyData
{
	std::string name = "NORMAL";   //タイトルとHUDに出す名前
	float damageTakenRate = 1.0f;  //プレイヤーが受けるダメージ
	float spawnIntervalRate = 1.0f;//雑魚が出る間隔。小さいほど多く出る
	float dropChanceRate = 1.0f;   //アイテムを落とす確率。ボス戦で降る間隔は、この逆数を掛ける
	float pityCountRate = 1.0f;    //続けて落ちなかったら必ず落とす数
	float commandTimeRate = 1.0f;  //コマンドの制限時間
	float warnTimeRate = 1.0f;     //敵の攻撃予告を出し始める時間
	float bossHealthRate = 1.0f;   //ボスのHP
};
//選べる難易度の一覧。Difficulty.jsonから読む
struct DifficultyTable
{
	std::vector<DifficultyData> difficulties;
	size_t defaultIndex = 0;//最初に選ばれている難易度
};