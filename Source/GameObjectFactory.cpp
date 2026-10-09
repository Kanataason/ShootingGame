#include "GameObjectfactory.h"

#include "PlayerController.h"
#include "PlayerMovement.h"
#include "PlayerAttack.h"
#include "PlayerStatus.h"
#include "PlayerProvider.h"

#include "EnemyController.h"
#include "EnemyMovement.h"
#include "EnemyStatus.h"
#include "EnemyAttack.h"

#include "BossController.h"

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
		ObjectBuilder builder = foundBuilder->second;
		(this->*builder)(*obj.get(), data); //メンバー関数として呼ぶ
	}


	return obj;
}
void GameObjectFactory::SetShapeData(GameObject& gameObject, const ObjectData& objectData,Team team)
{
	switch (objectData.statusData.characterType)
	{
	case CharacterType::Player:
	case CharacterType::MoveEnemy:
	{
		auto sizeStatus = gameObject.AddComponent<CircleShape>();
		if (const SphereData* shapeData = std::get_if<SphereData>(&objectData.sizeData))
		{
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
		sizeStatus->SetCenter(objectData.transform.Position);

		if (const BoxData* shapeData = std::get_if<BoxData>(&objectData.sizeData))
		{
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
	gameObject.AddComponent<EnemyController>();
	gameObject.AddComponent<EnemyMovement>();

	auto attack = gameObject.AddComponent<EnemyAttack>();
	attack->SetAttackTypes(objectData.attackType);

	//各敵に対しての初期値を設定
	auto status = gameObject.AddComponent<EnemyStatus>();
	status->SetHealth(objectData.statusData.health);
	status->SetMoveFlag(objectData.statusData.isMove);
	status->SetCharacterType(objectData.statusData.characterType);
	status->SetMovePattern(objectData.statusData.movePattern);
	status->SetImagePath(LoadGraph(objectData.statusData.imagePath.c_str()));

	SetShapeData(gameObject, objectData, Team::Enemy);
}

void GameObjectFactory::BuildPlayer(GameObject& gameObject, const ObjectData& objectData)
{
	gameObject.SetTransform(objectData.transform);
	gameObject.AddComponent<PlayerMovement>();
	gameObject.AddComponent<PlayerController>();

	auto attack = gameObject.AddComponent<PlayerAttack>();
	attack->SetAttackTypes(objectData.attackType);

	auto status = gameObject.AddComponent<PlayerStatus>();
	status->SetHealth(objectData.statusData.health);
	status->SetCharacterType(objectData.statusData.characterType);
	status->SetImagePath(LoadGraph(objectData.statusData.imagePath.c_str()));

	SetShapeData(gameObject, objectData, Team::Player);
	_locator.playerProvider->SetPlayer(&gameObject);
}
void GameObjectFactory::BuildBoss(GameObject& gameObject, const ObjectData& objectData)
{
	gameObject.SetTransform(objectData.transform);
	gameObject.AddComponent<BossController>();
	gameObject.AddComponent<EnemyMovement>();

	auto attack = gameObject.AddComponent<EnemyAttack>();
	attack->SetAttackTypes(objectData.attackType);

	auto status = gameObject.AddComponent<EnemyStatus>();
	status->SetHealth(objectData.statusData.health);
	status->SetCharacterType(objectData.statusData.characterType);
	status->SetImagePath(LoadGraph(objectData.statusData.imagePath.c_str()));

	SetShapeData(gameObject, objectData,Team::Enemy);
}