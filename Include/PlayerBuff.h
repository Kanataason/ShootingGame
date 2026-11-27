#pragma once
#include <array>

#include "Event.h"
#include "Component.h"

enum class BuffType
{
	//Healだけは時間で続かず、その場で回復する
	Attack, Defense, Speed, Predict, Heal, Count
};

//強化の色。機体とHUDで使う
int GetBuffColor(BuffType type);
//強化の頭文字（A、D、S、P、H）。アイテムとHUDの出番待ちで使う
const char* GetBuffInitial(BuffType type);

//1タイプ分の強化の状態
struct BuffSlot
{
	int level = 0;          //0は未強化。1〜MaxLevel
	float remaining = 0.0f; //残り秒数
};

//コマンド成功で付く強化の、段階と残り時間を持つ
//倍率はGet〜Rateで取る
class PlayerBuff : public Component
{
public:
	//バフタイプ、段階、残り時間
	Event<BuffType, int, float> OnBuffChanged;//HUDへ送るイベント

	std::unique_ptr<Component> Clone()const override
	{
		return std::make_unique<PlayerBuff>(*this);
	}
	void Initialize(ServiceLocator& locator)override {}
	void Update(float deltaTime)override;

	//段階を1つ上げて、残り時間を戻す。最大段階では時間だけ戻す
	//Healは何もしない。回復はPlayerStatusでやる
	void Apply(BuffType type);

	float GetAttackRate() const { return RateOf(BuffType::Attack, AttackRates); }
	float GetDamageRate() const { return RateOf(BuffType::Defense, DamageRates); }//被ダメージに掛ける
	float GetSpeedRate() const { return RateOf(BuffType::Speed, SpeedRates); }

	//今の段階。0は未強化
	int GetLevel(BuffType type) const { return _slots[static_cast<size_t>(type)].level; }

	//攻撃と速度が両方かかっているとコンボになる
	bool IsComboActive() const { return GetLevel(BuffType::Attack) > 0 && GetLevel(BuffType::Speed) > 0; }
	//コンボ中に移動しながら撃つと付く倍率
	float GetComboRate(bool isMoving) const { return (IsComboActive() && isMoving) ? ComboRate : 1.0f; }

	static constexpr int MaxLevel = 3;
	static constexpr float ComboRate = 1.15f;//HUDでも使う

private:
	static constexpr float Duration = 15.0f;

	//段階ごとの倍率。[0]は未強化
	using RateTable = std::array<float, MaxLevel + 1>;
	static constexpr RateTable AttackRates = { 1.0f, 1.25f, 1.5f, 1.8f };
	static constexpr RateTable DamageRates = { 1.0f, 0.8f, 0.65f, 0.6f };
	static constexpr RateTable SpeedRates = { 1.0f, 1.2f, 1.4f, 1.6f };

	float RateOf(BuffType type, const RateTable& rates) const
	{
		return rates[_slots[static_cast<size_t>(type)].level];
	}

	std::array<BuffSlot, static_cast<size_t>(BuffType::Count)> _slots{};
};
