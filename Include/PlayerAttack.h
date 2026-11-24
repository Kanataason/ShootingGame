#pragma once
#undef min
#undef max

#include <vector>
#include <unordered_map>

#include "Component.h"
#include "Vector2.h"
#include "GunData.h"
#include "Event.h"

class BulletManager;
class PlayerStatus;
class PlayerBuff;

//プレイヤーの弾の撃ち方を担当する
//撃ち方だけを持つ。撃つタイミングはステート側
class PlayerAttack : public Component
{
public :
	Event<float, float,bool> OnHeatChanged;//ヒート値が変わったときにUIへ送るイベント

	std::unique_ptr<Component> Clone()const override
	{
		return std::make_unique<PlayerAttack>(*this);
	}
	void Initialize(ServiceLocator& locator)override;
	void Update(float deltaTime)override {};

	void InitializeShotSetting(int type);

	//isMovingはコンボ用
	//真上に1発撃つ
	void LongShot(bool isMoving);
	//真上から左右20度の範囲に、ランダムな向きで複数撃つ
	void SpreadShot(bool isMoving);

	//Jsonで宣言した配列のindexを設定する。範囲外なら何もしない
	void SetCurrentAttackIndex(size_t index);
	void SetAttackTypes(std::vector<int> type) { _attackTypes = std::move(type); }
	//JSONにAttackTypeが無い場合でも落ちないように0を返す
	int GetAttackType()const
	{
		return (_currentAttackIndex < _attackTypes.size()) ? _attackTypes[_currentAttackIndex] : 0;
	}

	//ヒート値に関する処理
	bool CanFire()const { return !_isOverheated; }
	//撃った分のヒートを溜める。最大まで溜まるとオーバーヒート
	void AddHeat(float amount);
	//時間で冷やす。0まで冷えるとオーバーヒートが解ける
	void Cool(float deltaTime);
private:
	//メンバ変数の角度内でランダムに撃つ
	Vector2 GetRandomDirection()const;
	//強化をかけた弾の性能
	ShotSetting GetBuffedShotData(bool isMoving)const;
private:
	float _heat = 0.0f;
	float _maxHeat = 100.0f;//最大ヒート
	float _coolRate = 30.0f;//1秒あたりに冷える量
	bool _isOverheated = false;

	BulletManager* _bulletManager = nullptr;
	PlayerBuff* _buff = nullptr;

	ShotSetting _currentShotData{};

	std::vector<int> _attackTypes;
	size_t _currentAttackIndex = 0;

	int _bulletColor = 0xFFFF00;
	float _minDegrees = 70.0f;//真上から左右に20度ずつ
	float _maxDegrees = 110.0f;
};