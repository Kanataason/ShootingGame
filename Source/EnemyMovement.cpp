#include "EnemyMovement.h"
#include "GameObject.h"


void EnemyMovement::SetDestination(const Vector2& targetPos, float duration)
{
	_startPos = GetOwner()->GetTransform().Position;
	_targetPos = targetPos;
	_duration = duration;
	_elapsedTime = 0.0f;
}

bool EnemyMovement::Translate(float deltaTime)
{
	//JSONのIsMoveが0の敵は動かさない
	if (!_isMove)return false;

	auto& position = GetOwner()->GetTransform().Position;

	// durationが0以下なら、即座に目標位置に移動して終了
	if (_duration <= 0.0f)
	{
		position = _targetPos;
		return true;
	}

	_elapsedTime += deltaTime;
	float progress = _elapsedTime / _duration;
	if (progress >= 1.0f)
	{
		position = _targetPos; //ピッタリ移動
		return true;
	}
	// Lerpで滑らかに移動
	position.x = std::lerp(_startPos.x, _targetPos.x, progress);
	position.y = std::lerp(_startPos.y, _targetPos.y, progress);
	return false;
}
