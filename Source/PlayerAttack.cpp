#include <DxLib.h>
#include <algorithm>

#include "PlayerAttack.h"
#include "PlayerBuff.h"
#include "BulletManager.h"
#include "GameObject.h"

void PlayerAttack::Initialize(ServiceLocator& locator)
{
	_bulletManager = locator.bulletManager;
	_buff = GetOwner()->GetComponent<PlayerBuff>();
}
void PlayerAttack::InitializeShotSetting(int type)
{
    if (const ShotSetting* setting = FindShotSetting(type))
        _currentShotData = *setting;
}

void PlayerAttack::LongShot(bool isMoving)
{
    //上方向固定なのでdirectionは固定
	auto direction = Vector2(0, -1);
	const ShotSetting shotData = GetBuffedShotData(isMoving);
	_bulletManager->FireBullet(GetOwner()->GetTransform().Position, direction * shotData.bulletSpeed,
        shotData,_bulletColor, Team::Player);
}
void PlayerAttack::SpreadShot(bool isMoving)
{

    Vector2 firePosition = GetOwner()->GetTransform().Position;
    const ShotSetting shotData = GetBuffedShotData(isMoving);

    for (int bulletNumber = 0; bulletNumber < shotData.bulletCount; ++bulletNumber)
    {
        Vector2 bulletDirection = GetRandomDirection();

        _bulletManager->FireBullet(firePosition, bulletDirection *shotData.bulletSpeed,
            shotData, _bulletColor,Team::Player);
    }
}
ShotSetting PlayerAttack::GetBuffedShotData(bool isMoving)const
{
    //元の性能表は書き換えず、コピーにだけ倍率を掛ける
    ShotSetting shotData = _currentShotData;
    shotData.bulletDamage *= _buff->GetAttackRate() * _buff->GetComboRate(isMoving);
    return shotData;
}
Vector2 PlayerAttack::GetRandomDirection()const
{
    //ばらつきを持たせてランダム性を持たせる
    const float randomDegrees = _minDegrees + GetRand(static_cast<int>((_maxDegrees - _minDegrees) * 100.0f)) / 100.0f;
    const float randomAngle = randomDegrees * DX_PI_F / 180.0f;

    return { -std::cos(randomAngle), -std::sin(randomAngle) };
}