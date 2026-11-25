#pragma once
#include "Datas.h"

//弾1種類分の性能
struct ShotSetting
{
	int bulletCount = 0;      //1回に撃つ弾の数
	float bulletDamage = 0.0f;
	float bulletSpeed = 0.0f;
	float bulletLife = 0.0f;  //消えるまでの秒数
	float length = 0.0f;      //描画する線の長さ
	float thickness = 0.0f;   //線の太さ。当たり判定の太さにも使う
	int bounceCount = 0;
};

//IDから弾の性能を探す。見つからなければnullptr
//性能表はGunData.cppにある
const ShotSetting* FindShotSetting(int id);
