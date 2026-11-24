#include <algorithm>
#include <array>

#include "PlayerStates.h"
#include "PlayerController.h"

#include "SoundManager.h"
#include "PlayerAttack.h"
#include "PlayerMovement.h"
#include "PlayerStatus.h"
#include "PlayerBuff.h"
#include "PlayerCommand.h"
#include "Collider.h"

#include "EffectUtillity.h"
#include "GameObject.h"

namespace
{
	//移動速度（ピクセル/秒）。通常時と無敵中で共通
	constexpr float MoveSpeed = 350.0f;

	//段階ごとの矢印の数と制限時間。[0]が段階1
	constexpr std::array<int, PlayerBuff::MaxLevel> CommandLengths = { 5, 6,7 };
	constexpr std::array<float, PlayerBuff::MaxLevel> CommandTimeLimits = { 3.0f, 3.5f, 4.0f };

	constexpr std::array<InputKeyName, 4> CommandDirections =
	{
		InputKeyName::up, InputKeyName::down, InputKeyName::left, InputKeyName::right
	};

	//候補が消えるまでの秒数
	constexpr float OfferTime = 4.0f;

	constexpr float HealAmount = 5.0f;   //回復量
	constexpr float PinchHpRatio = 0.3f; //HPがこの割合以下なら回復を必ず出す

	//1回に出す候補の数
	constexpr size_t OfferCount = 3;

	//1文字目が被らないように、候補は方向の数まで
	static_assert(OfferCount <= CommandDirections.size());
	static_assert(OfferCount <= static_cast<size_t>(BuffType::Count));

	//並びをシャッフルする
	template<class T, size_t Size>
	void Shuffle(std::array<T, Size>& values)
	{
		for (int index = static_cast<int>(Size) - 1; index > 0; --index)
			std::swap(values[index], values[GetRand(index)]);
	}

	//お手本の方向列をランダムに作る
	std::vector<InputKeyName> MakeRandomSequence(int length)
	{
		std::vector<InputKeyName> sequence;
		sequence.reserve(length);
		for (int count = 0; count < length; ++count)
			sequence.push_back(CommandDirections[GetRand(static_cast<int>(CommandDirections.size()) - 1)]);
		return sequence;
	}

	//強化をOfferCount個選んで候補を作る
	//mustHealなら回復を必ず入れる
	//1文字目は候補ごとに別の方向にする
	std::vector<CommandCandidate> MakeCandidates(const PlayerBuff& buff, bool mustHeal)
	{
		//タイプをシャッフルして先頭から選ぶ
		std::array<BuffType, static_cast<size_t>(BuffType::Count)> types{};
		for (size_t index = 0; index < types.size(); ++index)
			types[index] = static_cast<BuffType>(index);
		Shuffle(types);

		//回復が選ばれていなければ先頭と入れ替える
		if (mustHeal)
		{
			auto heal = std::find(types.begin(), types.end(), BuffType::Heal);
			if (heal >= types.begin() + OfferCount) std::iter_swap(types.begin(), heal);
		}

		//HUDの並びをそろえるためにタイプ順に戻す
		std::sort(types.begin(), types.begin() + OfferCount);

		//1文字目に使う方向をシャッフル
		auto firstKeys = CommandDirections;
		Shuffle(firstKeys);

		std::vector<CommandCandidate> candidates;
		for (size_t index = 0; index < OfferCount; ++index)
		{
			const BuffType type = types[index];
			//次の段階で長さと時間を決める。最大段階でもかけ直せる
			//回復はいつも段階1の長さ
			const int nextLevel = (std::min)(buff.GetLevel(type) + 1, PlayerBuff::MaxLevel);

			CommandCandidate candidate;
			candidate.type = type;
			candidate.sequence = MakeRandomSequence(CommandLengths[nextLevel - 1]);
			candidate.sequence.front() = firstKeys[index];
			candidate.timeLimit = CommandTimeLimits[nextLevel - 1];
			candidates.push_back(std::move(candidate));
		}
		return candidates;
	}

	//ピンチかどうか
	bool IsPinch(const PlayerStatus& status)
	{
		return status.GetHealth() <= status.GetMaxHealth() * PinchHpRatio;
	}

	//強化ごとの成功時の音
	SoundId GetCommandSound(BuffType type)
	{
		switch (type)
		{
		case BuffType::Attack:  return SoundId::CommandAttack;
		case BuffType::Defense: return SoundId::CommandDefense;
		case BuffType::Speed:   return SoundId::CommandSpeed;
		case BuffType::Predict: return SoundId::CommandPredict;
		default:                return SoundId::CommandHeal;
		}
	}

	//コマンド成功時の効果。回復だけはその場でHPを戻す
	void ApplyCommand(PlayerController& controller, BuffType type)
	{
		controller.PlaySe(GetCommandSound(type));

		if (type == BuffType::Heal)
			controller.GetStatus()->Heal(HealAmount);
		else
			controller.GetBuff()->Apply(type);
	}
}

namespace PlayerStates
{
	void AliveState::OnUpdate(float deltaTime)
	{
		owner().MoveByInput(deltaTime, MoveSpeed);
	}
	void AliveState::OnDraw()
	{
		owner().DrawBody();
	}

	void InvincibleState::OnEnter(State* prevState)
	{
		TimedState::OnEnter(prevState);
		owner().GetCollider()->SetCollisionEnabled(false);//無敵中は当たり判定をなくす
	}
	void InvincibleState::OnUpdate(float deltaTime)
	{
		owner().MoveByInput(deltaTime, MoveSpeed);//無敵中も動ける
		TimedState::OnUpdate(deltaTime);
	}
	void InvincibleState::OnExit(State* nextState)
	{
		owner().GetCollider()->SetCollisionEnabled(true);
	}
	void InvincibleState::OnTimeUp()
	{
		stateMachine().Dispatch(LifeEvent::EventInvincibleEnd);
	}
	void InvincibleState::OnDraw()
	{
		//残り時間から、点滅で表示するフレームかどうかを判定する
		if (EffectUtillity::IsBlinkVisible(GetRemaining()))
			owner().DrawBody();
	}

	void DeadState::OnEnter(State* prevState)
	{
		TimedState::OnEnter(prevState);
		owner().GetSoundManager()->PlaySe(SoundId::CharacterExplosion);
		owner().GetCollider()->SetCollisionEnabled(false);
	}
	void DeadState::OnTimeUp()
	{
		owner().GetStatus()->OnDeathFinished.Invoke();
	}
	void DeadState::OnDraw()
	{
		//死亡エフェクトを描写
		EffectUtillity::DrawEffect(GetElapsed(), _effectTime, 0xFF0000, owner().GetOwner()->GetTransform().Position);
	}

	//攻撃状態のステート
	void PlayerShotState::OnEnter(State* prevState)
	{
		auto* attack = owner().GetAttack();
		attack->SetCurrentAttackIndex(_attackIndex);
		attack->InitializeShotSetting(attack->GetAttackType());

		//入った直後は撃てる状態にし、切り替えには待ち時間を付ける
		SetGunState(_shotCoolTime, false, false);
	}
	void PlayerShotState::OnUpdate(float deltaTime)
	{
		const auto& controller = owner();
		const auto& attack = controller.GetAttack();

		//クールダウン処理
		owner().GetAttack()->Cool(deltaTime);
		//武器の切り替え
		if (controller.GetCurrentInput().isChangePressed && _isChangeFlag)
		{
			stateMachine().Dispatch(_switchEvent);
			return;
		}

		if (controller.GetCurrentInput().isShotHeld && !_isShotFlag && attack->CanFire())
		{
			SetShotInfo(true, _shotCoolTime);
			Shoot(*attack);
			attack->AddHeat(_heatPerShot);
		}
		ElapsedTimer(deltaTime);
	}

	void LongRangeShot::Shoot(PlayerAttack& attack)
	{
		owner().GetSoundManager()->PlaySe(SoundId::PlayerShot);
		attack.LongShot(owner().IsMoving());
	}
	void SpreadShotState::Shoot(PlayerAttack& attack)
	{
		owner().GetSoundManager()->PlaySe(SoundId::PlayerShotGun);
		attack.SpreadShot(owner().IsMoving());
	}



	//コマンドのステート
	void CommandIdleState::OnEnter(State* prevState)
	{
		TimedState::OnEnter(prevState);
		owner().GetCommand()->Stop();
	}
	void CommandIdleState::OnTimeUp()
	{
		owner().GetCommand()->Offer(MakeCandidates(*owner().GetBuff(), IsPinch(*owner().GetStatus())), OfferTime);
		stateMachine().Dispatch(CommandEvent::EventCommandOffer);
	}

	void CommandOfferState::OnUpdate(float deltaTime)
	{
		auto* command = owner().GetCommand();
		const auto& keys = owner().GetCurrentInput().commandList.commands;

		for (size_t index = 0; index < keys.size(); ++index)
		{
			if (!command->Select(keys[index])) continue;//どの候補でもないキーは無視

			//同じフレームの残りのキーは2文字目以降として渡す
			for (size_t rest = index + 1; rest < keys.size(); ++rest)
			{
				if (command->Input(keys[rest]) == CommandResult::Completed)
				{
					ApplyCommand(owner(), command->GetType());
					stateMachine().Dispatch(CommandEvent::EventCommandEnd);
					return;
				}
			}
			stateMachine().Dispatch(CommandEvent::EventCommandStart);
			return;
		}

		if (command->Tick(deltaTime) == CommandResult::TimeOut)
		{
			stateMachine().Dispatch(CommandEvent::EventCommandEnd);
		}
	}

	void CommandInputState::OnUpdate(float deltaTime)
	{
		auto* command = owner().GetCommand();

		//このフレームに押された矢印を、押された順に1つずつ渡す
		for (InputKeyName key : owner().GetCurrentInput().commandList.commands)
		{
			//最後まで打てたら強化をかけて待機へ
			//ミスしたときは最初からやり直しになるだけなので、入力中のまま続ける
			if (command->Input(key) == CommandResult::Completed)
			{
				ApplyCommand(owner(), command->GetType());
				stateMachine().Dispatch(CommandEvent::EventCommandEnd);
				return;
			}
		}

		//時間切れなら強化なしで待機へ
		if (command->Tick(deltaTime) == CommandResult::TimeOut)
		{
			stateMachine().Dispatch(CommandEvent::EventCommandEnd);
		}
	}

	void CommandDisabledState::OnEnter(State* prevState)
	{
		owner().GetCommand()->Stop();
	}
}
