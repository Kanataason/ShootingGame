#include <unordered_map>

#include "GunData.h"

namespace
{
	//弾の性能表。IDはJSONのAttackTypeで指定する
	//1000番台はPlayer、2000番台はEnemy
	//弾を追加したら、使うキャラのJSONのAttackTypeにもIDを書く
	//撃ち方が新しい場合は、PlayerAttackかEnemyAttackに関数を足し、それを呼ぶステートも作る
	const std::unordered_map<int, ShotSetting> BulletSettingTable =
	{
		//Player
		{1001, {.bulletCount = 0,  .bulletDamage = 1,    .bulletSpeed = 600, .bulletLife = 3,     .length = 10, .thickness = 3}}, //ロングショット
		{1002, {.bulletCount = 10, .bulletDamage = 0.6f, .bulletSpeed = 800, .bulletLife = 0.15f, .length = 6,  .thickness = 3}}, //拡散

		//Enemy
		{2001, {.bulletCount = 0,  .bulletDamage = 1,    .bulletSpeed = 600, .bulletLife = 3,     .length = 15, .thickness = 3, .bounceCount = 1}}, //狙い撃ち
		{2002, {.bulletCount = 5,  .bulletDamage = 0.5f, .bulletSpeed = 400, .bulletLife = 5,     .length = 20, .thickness = 3, .bounceCount = 4}}, //反射弾
		{2003, {.bulletCount = 20, .bulletDamage = 1,    .bulletSpeed = 400, .bulletLife = 3,     .length = 10, .thickness = 3}}, //全方位
		{2004, {.bulletCount = 10, .bulletDamage = 1,    .bulletSpeed = 400, .bulletLife = 3,     .length = 10, .thickness = 3}}  //渦巻き
	};
}

const ShotSetting* FindShotSetting(int id)
{
	auto it = BulletSettingTable.find(id);
	return (it != BulletSettingTable.end()) ? &it->second : nullptr;
}
