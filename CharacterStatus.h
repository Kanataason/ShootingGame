#pragma once
#include "Datas.h"
#include "GameObject.h"
#include "Event.h"
//プレイヤーと敵で共通のステータス。HP・種類・生存状態を持つ
class CharacterStatus
{
public:
	Event<float, float> OnHealthChanged;
	Event<> OnDeathFinished;
	//初期化用。最大HPも同時に記録する。ダメージはTakeDamageで与える
	void SetHealth(float hp) { _health = hp; _maxHealth = hp;}
	float GetHealth() const { return _health; }

	float GetMaxHealth() const { return _maxHealth; }

	void SetCharacterType(CharacterType characterState) { _characterState = characterState; }
	const CharacterType& GetCharacterType()const { return _characterState; }

	void TakeDamage(float damage) 
	{
		_health -= damage; 
		OnHealthChanged.Invoke(_health, _maxHealth);
	}

	void SetIsActive(bool isActive) { _isActive = isActive; }
	bool GetIsActive()const { return _isActive; }

	void SetImagePath(int path) { _imagePath = path; }
	int GetImagePath() const { return _imagePath; }

	virtual void OnDead() {};
protected:
	void ApplyDamage(float damage) 
	{
		TakeDamage(damage);
		if (_health <= 0.0f)
		{
			OnDead();
		}
	}
protected:
	//死亡フラグ
	bool _isActive = true;

	float _maxHealth = 0;
	float _health = 0;
	
	int _imagePath = 0;

	CharacterType _characterState = CharacterType::None;
};