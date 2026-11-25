#pragma once
#include <memory>

#include "Datas.h"
#include "PlayerBuff.h"
#include "ServiceLocator.h"

class GameObject;
class GameObjectFactory;

//雑魚を倒したときに、アイテムを落とすかどうかと、その種類を決める
//運が悪くて長く落ちないことがないよう、続けて落ちなかった回数を数えて、上限で必ず落とす
class ItemDropper
{
public:
	//itemDataはItems.jsonのアイテムのテンプレート。落とすたびに、これを元に作る
	void Initialize(const ServiceLocator& locator, ObjectData itemData, const ItemDropData& dropData);

	//雑魚を倒した位置で呼ぶ。落とすと決まったら、その位置にアイテムを作って返す。落とさなければnullptr
	//hpとmaxHpはプレイヤーのもの。ピンチなら回復のアイテムにする
	std::unique_ptr<GameObject> TryDrop(const Vector2& position, float hp, float maxHp);

	//アイテムの種類を決める。ピンチなら回復、それ以外は5種類からランダム
	//ボス戦で上から降らせるアイテムにも使う
	BuffType ChooseType(float hp, float maxHp) const;

private:
	//確率で落とすかを決める。続けて落ちなかった回数が上限に達していたら、必ず落とす
	bool RollDrop();

	GameObjectFactory* _factory = nullptr;
	ObjectData _itemData;
	ItemDropData _dropData;
	int _missCount = 0;//続けて落ちなかった回数
};
