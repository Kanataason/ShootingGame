#include <algorithm>

#include "GunState.h"

void GunState::SetGunState(float coolDown, bool shotFlag, bool changeFlag)
{
	_maxCoolDown = coolDown;

	_coolDown = coolDown;
	_changeCoolDown = _maxChangeTime;
	_isChangeFlag = changeFlag;
	_isShotFlag = shotFlag;
}

void GunState::Clamp()
{
	_changeCoolDown = std::clamp(_changeCoolDown, 0.0f, _maxChangeTime);
	_coolDown = std::clamp(_coolDown, 0.0f, _maxCoolDown);
}

void GunState::ElapsedTimer(float deltaTime)
{
	//間隔が終わっていれば、次の1発を撃てる状態に戻す
	if (_coolDown <= 0.0f)
		SetShotInfo(false, 0.0f);

	//切り替えの待ち時間が終わっていれば、切り替えを許す
	if (_changeCoolDown <= 0.0f)
		_isChangeFlag = true;

	_changeCoolDown -= deltaTime;
	_coolDown -= deltaTime;

	Clamp();
}
