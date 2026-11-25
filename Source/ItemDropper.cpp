#include <DxLib.h>

#include "ItemDropper.h"
#include "GameObject.h"
#include "GameObjectfactory.h"
#include "ItemController.h"

namespace
{
	//確率を整数の乱数で判定するときの細かさ。0.001刻みで指定できる
	constexpr int ChanceResolution = 1000;
}

void ItemDropper::Initialize(const ServiceLocator& locator, ObjectData itemData, const ItemDropData& dropData)
{
	_factory = locator.factory;
	_itemData = std::move(itemData);
	_dropData = dropData;
	_missCount = 0;
}

std::unique_ptr<GameObject> ItemDropper::TryDrop(const Vector2& position, float hp, float maxHp)
{
	if (!RollDrop()) return nullptr;

	//倒した敵の位置から落ちてくるようにする
	ObjectData data = _itemData;
	data.transform.Position = position;

	std::unique_ptr<GameObject> item = _factory->CreateObject(data);
	//種類は作った直後に決める。テンプレートは1つにして、種類ごとにJSONを分けなくてすむようにする
	if (auto* itemController = item->GetComponent<ItemController>())
		itemController->SetBuffType(ChooseType(hp, maxHp));
	return item;
}

BuffType ItemDropper::ChooseType(float hp, float maxHp) const
{
	//ピンチのときに攻撃などが出ても助からないので、回復に決める
	if (hp <= maxHp * _dropData.pinchHpRatio) return BuffType::Heal;

	//Countの手前までの、全部の強化から1つ選ぶ
	const int typeCount = static_cast<int>(BuffType::Count);
	return static_cast<BuffType>(GetRand(typeCount - 1));
}

bool ItemDropper::RollDrop()
{
	//続けて落ちなかった回数が上限に達していたら、確率に関係なく落とす
	const bool isPity = _missCount >= _dropData.pityCount;
	const bool isDropped = isPity || GetRand(ChanceResolution - 1) < static_cast<int>(_dropData.dropChance * ChanceResolution);

	//落としたら数え直し、落ちなかったら1つ増やす
	_missCount = isDropped ? 0 : _missCount + 1;
	return isDropped;
}
