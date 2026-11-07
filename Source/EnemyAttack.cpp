#include "EnemyAttack.h"
#include "PlayerProvider.h"
#include "PlayerStatus.h"
#include "BulletManager.h"
#include "Primitive.h"

#include "GameWindow.h"
#include "GameObject.h"

#include <algorithm>
#include <array>

namespace
{
	//予測の段階ごとの、撃つ何秒前から予告を出すか。添字は段階で、0は予測なし
	constexpr std::array<float, 4> WarnTimes = { 0.3f, 0.5f, 0.7f, 1.0f };
	//予測の段階ごとの、撃つ方向の線の長さ（ピクセル）。0は線を出さない
	constexpr std::array<float, 4> WarnLineLengths = { 0.0f, 120.0f, 240.0f, 480.0f };

	constexpr float WarnRingSpread = 24.0f; //出し始めの輪が、本体からどれだけ外にあるか
	constexpr int WarnRingAlpha = 160;      //撃つ瞬間の輪の濃さ
	constexpr int WarnLineBaseAlpha = 60;   //撃つ瞬間の線の濃さ
	constexpr int WarnLineAlphaPerLevel = 50;//予測の段階が1つ上がるごとに線を濃くする量
	constexpr float DefaultBodyRadius = 16.0f;//形が取れないときの本体の半径
}

void EnemyAttack::Initialize(ServiceLocator& locator)
{
	_bulletManager = locator.bulletManager;
	_gameWindow = locator.gameWindow;
	_provider = locator.playerProvider;
}

void EnemyAttack::InitializeShotSetting(int attackType)
{
	_shape = GetOwner()->GetComponent<Primitive>();
	if (const ShotSetting* setting = FindShotSetting(attackType))
		_gunData = *setting;
}

void EnemyAttack::LongRangeShot()
{
	//弾を出す場所は形によって変える
	for (const Vector2& direction : GetLongRangeDirections())
	{
		_bulletManager->FireBullet(GetMuzzlePosition(direction), direction * _gunData.bulletSpeed,
			_gunData, _bulletColor, Team::Enemy);
	}
}
void EnemyAttack::ReflectShot()
{
	FireAll(GetReflectDirections());
}
void EnemyAttack::SpreadShot()
{
	FireAll(GetSpreadDirections());
}
void EnemyAttack::SwirlShot(float baseDegrees)
{
	FireAll(GetSwirlDirections(baseDegrees));
}

std::vector<Vector2> EnemyAttack::GetLongRangeDirections() const
{
	if (!CanShoot()) return {};

	const Vector2& shooterPosition = GetOwner()->GetTransform().Position;
	Vector2 toPlayer = _provider->GetPlayer()->GetTransform().Position - shooterPosition;
	float distance = toPlayer.Length();
	if (distance <= 0.0001f) return {}; //ほぼ同じ位置なら撃たない

	return { toPlayer.Normalized(distance) };
}
std::vector<Vector2> EnemyAttack::GetReflectDirections() const
{
	int bulletCount = _gunData.bulletCount;
	if (!CanShoot() || bulletCount <= 0) return {};

	Vector2 angleRange = GetNearerHorizontalAngle();

	//1発なら中央、2発以上なら両端を含めて均等に並べる
	float angleStep = (bulletCount > 1)
		? (angleRange.y - angleRange.x) / (bulletCount - 1)
		: 0.0f;
	float startDegrees = (bulletCount > 1)
		? angleRange.x
		: (angleRange.x + angleRange.y) / 2.0f;

	std::vector<Vector2> directions;
	for (int bulletIndex = 0; bulletIndex < bulletCount; ++bulletIndex)
		directions.push_back(DirectionFromAngle(startDegrees + angleStep * bulletIndex));
	return directions;
}
std::vector<Vector2> EnemyAttack::GetSpreadDirections() const
{
	//全方位は、渦巻きを0度から撃ったときと同じ並び
	return GetSwirlDirections(0.0f);
}
std::vector<Vector2> EnemyAttack::GetSwirlDirections(float baseDegrees) const
{
	int armCount = _gunData.bulletCount;//渦の腕の数
	if (!CanShoot() || armCount <= 0) return {};

	float angleStep = 360.0f / static_cast<float>(armCount);

	std::vector<Vector2> directions;
	for (int armIndex = 0; armIndex < armCount; ++armIndex)
	{
		float radians = (baseDegrees + angleStep * armIndex) * DX_PI_F / 180.0f;
		directions.push_back({ std::cos(radians), std::sin(radians) });
	}
	return directions;
}

void EnemyAttack::DrawWarning(const std::vector<Vector2>& directions, float ratio) const
{
	if (!CanShoot()) return;

	const Vector2& center = GetOwner()->GetTransform().Position;
	const int level = GetPredictLevel();

	//加算で光らせる。撃つ瞬間に近づくほど濃くする
	//本体に向かって縮んでいく輪。撃つ瞬間に本体の大きさになる
	const float ringRadius = GetBodyRadius() + (1.0f - ratio) * WarnRingSpread;
	SetDrawBlendMode(DX_BLENDMODE_ADD, static_cast<int>(ratio * WarnRingAlpha));
	DrawCircle((int)center.x, (int)center.y, (int)ringRadius, _bulletColor, FALSE, 2);

	//予測中だけ、撃つ方向の線を出す。段階が高いほど長く、濃く、太い
	const float length = WarnLineLengths[level];
	if (length > 0.0f)
	{
		SetDrawBlendMode(DX_BLENDMODE_ADD, static_cast<int>(ratio * (WarnLineBaseAlpha + level * WarnLineAlphaPerLevel)));
		for (const Vector2& direction : directions)
		{
			const Vector2 end = center + direction * length;
			DrawLine((int)center.x, (int)center.y, (int)end.x, (int)end.y, _bulletColor, level);
		}
	}
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}
float EnemyAttack::GetWarnTime() const
{
	return WarnTimes[GetPredictLevel()];
}

bool EnemyAttack::CanShoot() const
{
	return _provider->GetPlayer() != nullptr && _provider->GetActive();
}
void EnemyAttack::FireAll(const std::vector<Vector2>& directions)
{
	const Vector2& shooterPosition = GetOwner()->GetTransform().Position;
	for (const Vector2& direction : directions)
	{
		_bulletManager->FireBullet(shooterPosition, direction * _gunData.bulletSpeed,
			_gunData, _bulletColor, Team::Enemy);
	}
}
int EnemyAttack::GetPredictLevel() const
{
	//表の範囲外を読まないように収める
	return std::clamp(_provider->GetPredictLevel(), 0, static_cast<int>(WarnTimes.size()) - 1);
}
float EnemyAttack::GetBodyRadius() const
{
	if (_shape == nullptr) return DefaultBodyRadius;

	//円なら半径、四角形なら長いほうの辺の半分
	const Vector2 half = _shape->GetHalfSize();
	return (std::max)({ _shape->GetRadius(), half.x, half.y });
}
Vector2 EnemyAttack::GetMuzzlePosition(const Vector2& direction) const
{
	const Vector2& center = GetOwner()->GetTransform().Position;
	if (_shape == nullptr) return center;

	//円なら半径、四角形なら向いている方向で先に当たる辺までの距離
	float offset = _shape->GetRadius();
	Vector2 half = _shape->GetHalfSize();
	if (half.x > 0.0f || half.y > 0.0f)
	{
		float toX = (direction.x != 0.0f) ? half.x / std::abs(direction.x) : FLT_MAX;
		float toY = (direction.y != 0.0f) ? half.y / std::abs(direction.y) : FLT_MAX;
		offset = (std::min)(toX, toY);
	}
	return center + direction * offset;
}

Vector2 EnemyAttack::DirectionFromAngle(float degrees)const
{
	const float radians = degrees * DX_PI_F / 180.0f;

	return { -std::cos(radians), -std::sin(radians) };
}
Vector2 EnemyAttack::GetNearerHorizontalAngle() const
{
    float positionX = GetOwner()->GetTransform().Position.x;
	float width = static_cast<float>(_gameWindow->GetBounds().width);

	float distanceToLeft  = positionX;
	float distanceToRight = width - positionX;

	//遠い側の壁に向かって、斜め下に撃つ
	return (distanceToLeft <= distanceToRight)
		? Vector2{ 200.0f, 260.0f }  //右下寄り
	: Vector2{ 280.0f, 340.0f }; //左下寄り
}