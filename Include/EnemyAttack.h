#pragma once
#include <vector>
#include "Vector2.h"
#include "Component.h"
#include "GunData.h"


class BulletManager;
class PlayerProvider;
class GameWindow;
class Primitive;

//敵の弾の撃ち方を担当する
//いつ撃つかはEnemyStatesが決め、ここは「どう撃つか」だけを持つ
class EnemyAttack : public Component
{
public:
	std::unique_ptr<Component> Clone()const override
	{
		return std::make_unique<EnemyAttack>(*this);
	}

	void Initialize(ServiceLocator& locator)override;
	void Update(float deltaTime)override {};

	//プレイヤーを狙って1発撃つ
	void LongRangeShot();
	//遠い側の壁に向かって、斜め下に扇状に撃つ。壁で跳ね返る回数は弾の設定で決まる
	void ReflectShot();
	//全方位に均等に撃つ
	void SpreadShot();
	//baseDegreesを起点に、bulletCount本の腕の方向へ1発ずつ撃つ。角度をずらしながら呼ぶと渦巻きになる
	void SwirlShot(float baseDegrees);

	//それぞれの撃ち方で、弾が飛ぶ方向の一覧。撃つときと攻撃予告で同じものを使い、予告と実際の弾がずれないようにする
	//撃てないときは空
	std::vector<Vector2> GetLongRangeDirections() const;
	std::vector<Vector2> GetReflectDirections() const;
	std::vector<Vector2> GetSpreadDirections() const;
	std::vector<Vector2> GetSwirlDirections(float baseDegrees) const;

	//攻撃予告を描く。ratioは0で出し始め、1で撃つ瞬間
	//本体に縮んでいく輪は常に出し、予測の強化中だけ撃つ方向の線も出す
	void DrawWarning(const std::vector<Vector2>& directions, float ratio) const;
	//撃つ何秒前から予告を出すか。予測の段階が高いほど早い
	float GetWarnTime() const;

	void InitializeShotSetting(int attackType);

	void SetCurrentAttackIndex(size_t index) { _currentAttackIndex = index; }
	void SetAttackTypes(std::vector<int> type) { _attackTypes = std::move(type); }

	//JSONにAttackTypeが無い場合でも落ちないように0を返す
	int GetAttackType()const
	{
		return (_currentAttackIndex < _attackTypes.size()) ? _attackTypes[_currentAttackIndex] : 0;
	}

private:
	//撃つ場所を図形によって求めている
	Vector2 GetMuzzlePosition(const Vector2& direction) const;
	//プレイヤーが生きている間だけ撃つ
	bool CanShoot() const;
	//directionsの方向へ1発ずつ、本体の中心から撃つ
	void FireAll(const std::vector<Vector2>& directions);
	Vector2 GetNearerHorizontalAngle() const;
	//角度から、上向きを基準にした単位ベクトルを返す
	Vector2 DirectionFromAngle(float degrees)const;
	//プレイヤーにかかっている予測の段階
	int GetPredictLevel() const;
	//本体のおおよその半径。予告の輪の大きさに使う
	float GetBodyRadius() const;
private:
	std::vector<int> _attackTypes;
	size_t _currentAttackIndex = 0;

	BulletManager* _bulletManager = nullptr;
	PlayerProvider* _provider = nullptr;
	GameWindow* _gameWindow = nullptr;
	Primitive* _shape = nullptr;
	
	int _bulletColor = 0xFF4466;
	ShotSetting _gunData{};
};