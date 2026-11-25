#include "GameObjectfactory.h"

#include "PlayerController.h"
#include "PlayerMovement.h"
#include "PlayerAttack.h"
#include "PlayerStatus.h"
#include "PlayerBuff.h"
#include "PlayerCommand.h"
#include "PlayerProvider.h"

#include "EnemyController.h"
#include "EnemyMovement.h"
#include "EnemyStatus.h"
#include "EnemyAttack.h"

#include "BossController.h"
#include "ItemController.h"

#include "CircleCollider.h"
#include "BoxCollider.h"

std::unique_ptr<GameObject> GameObjectFactory::CreateObject(ObjectData data)
{
	//本来ならすでにロードされているかを確認してから生成するが
	//今回使うモデルは、primitiveなのでコストがないのでそのまま処理

	std::unique_ptr<GameObject> obj = std::make_unique<GameObject>();

	//関数テーブルで設定する
	auto foundBuilder = _objectBuilderTable.find(data.name);
	if (foundBuilder != _objectBuilderTable.end())
	{
		//ObjectTypeに対応する組み立て関数が見つかったので呼ぶ
		ObjectBuilder builder = foundBuilder->second;
		(this->*builder)(*obj.get(), data); //メンバー関数として呼ぶ
	}


	return obj;
}
void GameObjectFactory::SetShapeData(GameObject& gameObject, const ObjectData& objectData,Team team)
{
	//キャラクターの種類によって、円形か四角形の当たり判定を作る
	switch (objectData.statusData.characterType)
	{
	case CharacterType::Player:
	case CharacterType::MoveEnemy:
	case CharacterType::Item:
	{
		auto sizeStatus = gameObject.AddComponent<CircleShape>();
		if (const SphereData* shapeData = std::get_if<SphereData>(&objectData.sizeData))
		{
			//円形の当たり判定を作る
			gameObject.AddComponent<CircleCollider>(shapeData->radius,team);
			sizeStatus->SetRadius(shapeData->radius);
		}
		break;
	}
	case CharacterType::Boss:
	case CharacterType::SpreadEnemy:
	case CharacterType::ReflectShotEnemy:
	{
		auto sizeStatus = gameObject.AddComponent<BoxShape>();
		if (const BoxData* shapeData = std::get_if<BoxData>(&objectData.sizeData))
		{
			//四角形の当たり判定を作る
			gameObject.AddComponent<BoxCollider>(shapeData->halfSize,team);
			sizeStatus->SetHalfSize(shapeData->halfSize);
		}
		break;
	}
	}
}

void GameObjectFactory::BuildEnemy(GameObject& gameObject, const ObjectData& objectData)
{
	gameObject.SetTransform(objectData.transform);
	//司令塔と、移動・攻撃の部品。どう動くか、いつ撃つかはEnemyControllerのステートが決める
	gameObject.AddComponent<EnemyController>();
	gameObject.AddComponent<EnemyMovement>();

	//撃てる弾の種類。JSONのAttackTypeの並び順で、ステートから番号で選ぶ
	auto attack = gameObject.AddComponent<EnemyAttack>();
	attack->SetAttackTypes(objectData.attackType);

	//各敵に対しての初期値を設定
	//種類と動き方は、登場後にどのステートへ進むかの判定に使う
	auto status = gameObject.AddComponent<EnemyStatus>();
	status->SetHealth(objectData.statusData.health);
	status->SetMoveFlag(objectData.statusData.isMove);
	status->SetCharacterType(objectData.statusData.characterType);
	status->SetMovePattern(objectData.statusData.movePattern);
	status->SetImagePath(LoadGraph(objectData.statusData.imagePath.c_str()));

	//Enemyチームにして、敵の弾が味方に当たらないようにする
	SetShapeData(gameObject, objectData, Team::Enemy);
}

void GameObjectFactory::BuildPlayer(GameObject& gameObject, const ObjectData& objectData)
{
	gameObject.SetTransform(objectData.transform);
	//入力から動かす部品と司令塔。ステートマシンはPlayerControllerが持つ
	gameObject.AddComponent<PlayerMovement>();
	gameObject.AddComponent<PlayerController>();

	//武器の種類。JSONのAttackTypeの並び順が、切り替えの順番になる
	auto attack = gameObject.AddComponent<PlayerAttack>();
	attack->SetAttackTypes(objectData.attackType);

	auto status = gameObject.AddComponent<PlayerStatus>();
	status->SetHealth(objectData.statusData.health);
	status->SetCharacterType(objectData.statusData.characterType);
	status->SetImagePath(LoadGraph(objectData.statusData.imagePath.c_str()));

	//コマンドと強化はプレイヤーだけが持つ
	gameObject.AddComponent<PlayerBuff>();
	gameObject.AddComponent<PlayerCommand>();

	//Playerチームにして、自分の弾が自分に当たらないようにする
	SetShapeData(gameObject, objectData, Team::Player);
	//PlayerProviderにプレイヤーを登録する
	_locator.playerProvider->SetPlayer(&gameObject);
}
void GameObjectFactory::BuildBoss(GameObject& gameObject, const ObjectData& objectData)
{
	gameObject.SetTransform(objectData.transform);
	//雑魚と違うのは司令塔だけ。移動・攻撃・HPの部品は雑魚と同じものを使い回す
	gameObject.AddComponent<BossController>();
	gameObject.AddComponent<EnemyMovement>();

	//ボスは攻撃を切り替えるので、AttackTypeに全部の攻撃を並べておく
	auto attack = gameObject.AddComponent<EnemyAttack>();
	attack->SetAttackTypes(objectData.attackType);

	//各敵に対しての初期値を設定
	//動き方はBossControllerのステートが決めるので、MovePatternは使わない
	auto status = gameObject.AddComponent<EnemyStatus>();
	status->SetHealth(objectData.statusData.health);
	status->SetCharacterType(objectData.statusData.characterType);
	status->SetImagePath(LoadGraph(objectData.statusData.imagePath.c_str()));

	SetShapeData(gameObject, objectData,Team::Enemy);
}
void GameObjectFactory::BuildItem(GameObject& gameObject, const ObjectData& objectData)
{
	gameObject.SetTransform(objectData.transform);
	gameObject.AddComponent<ItemController>();

	//Itemチームにして、弾に当たらず、プレイヤーに触れたら拾われるようにする
	SetShapeData(gameObject, objectData, Team::Item);
}