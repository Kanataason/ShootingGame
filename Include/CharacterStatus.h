#pragma once
#include "Datas.h"
#include "GameObject.h"
#include "Event.h"
//プレイヤーと敵で共通のステータス。HP・種類・生存状態を持つ
class CharacterStatus
{
public:
	//今のHP、最大HP。ダメージでも回復でも呼ぶので、HPバーなど見た目の更新に使う
	Event<float, float> OnHealthChanged;
	//ダメージを受けたときだけ呼ぶ。被弾後の無敵など、回復では起きてほしくない処理に使う
	Event<> OnDamaged;
	//回復したときだけ呼ぶ。回復のエフェクトを出すのに使う
	Event<> OnHealed;
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
		OnDamaged.Invoke();
	}
	//最大HPを超えないように回復する。死んでいるときは何もしない
	void Heal(float amount)
	{
		if (!_isActive) return;
		_health = (_health + amount < _maxHealth) ? _health + amount : _maxHealth;
		OnHealthChanged.Invoke(_health, _maxHealth);
		OnHealed.Invoke();
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