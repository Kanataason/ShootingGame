#pragma once
#include <array>

#include "Event.h"
#include "Component.h"

enum class BuffType
{
	//Healだけは時間で続く強化ではなく、成功したその場でHPを回復する
	Attack, Defense, Speed, Predict, Heal, Count
};

//強化の色。機体の光とHUDで共通で使う（企画書：攻撃＝赤、防御＝青、速度＝黄。予測は紫、回復は緑）
inline int GetBuffColor(BuffType type)
{
	switch (type)
	{
	case BuffType::Attack:  return 0xFF4444;
	case BuffType::Defense: return 0x4488FF;
	case BuffType::Speed:   return 0xFFDD00;
	case BuffType::Predict: return 0xBB66FF;
	case BuffType::Heal:    return 0x44FF88;
	default:                return 0xFFFFFF;
	}
}

//1タイプ分の強化の状態
struct BuffSlot
{
	int level = 0;          //0は未強化。1〜MaxLevel
	float remaining = 0.0f; //残り秒数
};

//コマンド成功で付く強化の、段階と残り時間を持つ
//倍率は段階から表で引くだけなので、ほかのクラスはGet〜Rateを読むだけでよい
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
	//Healは時間で続かないので何もしない。回復はコマンドのステートがPlayerStatusに頼む
	void Apply(BuffType type);

	float GetAttackRate() const { return RateOf(BuffType::Attack, AttackRates); }
	float GetDamageRate() const { return RateOf(BuffType::Defense, DamageRates); }//被ダメージに掛ける
	float GetSpeedRate() const { return RateOf(BuffType::Speed, SpeedRates); }

	//今の段階。0は未強化。コマンドの長さや、敵の攻撃予告の強さを決めるのに使う
	int GetLevel(BuffType type) const { return _slots[static_cast<size_t>(type)].level; }

	//攻撃と速度が両方かかっているとコンボになる
	bool IsComboActive() const { return GetLevel(BuffType::Attack) > 0 && GetLevel(BuffType::Speed) > 0; }
	//コンボ中に移動しながら撃つと、ダメージに追加で掛ける倍率
	//止まって撃つと付かないので、「攻撃＋速度が常に正解」にはならない
	float GetComboRate(bool isMoving) const { return (IsComboActive() && isMoving) ? ComboRate : 1.0f; }

	static constexpr int MaxLevel = 3;
	static constexpr float ComboRate = 1.15f;//HUDにも倍率を出すのでpublic

private:
	static constexpr float Duration = 15.0f;

	//段階ごとの倍率。添字が段階で、0は未強化
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
