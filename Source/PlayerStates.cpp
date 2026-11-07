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

	//次に上がる強化の段階ごとの、方向の数と制限時間。添字は段階-1
	constexpr std::array<int, PlayerBuff::MaxLevel> CommandLengths = { 5, 6,7 };
	constexpr std::array<float, PlayerBuff::MaxLevel> CommandTimeLimits = { 3.0f, 3.5f, 4.0f };

	constexpr std::array<InputKeyName, 4> CommandDirections =
	{
		InputKeyName::up, InputKeyName::down, InputKeyName::left, InputKeyName::right
	};

	//候補を出してから、選ばれずに消えるまでの秒数
	constexpr float OfferTime = 4.0f;

	constexpr float HealAmount = 5.0f;   //回復のコマンド1回で戻すHP
	constexpr float PinchHpRatio = 0.3f; //HPがこの割合以下だと、回復を必ず候補に入れる

	//1回に出す候補の数。強化の種類のうち、毎回ランダムにこの数だけ選んで出す
	constexpr size_t OfferCount = 3;

	//候補ごとに1文字目を別の方向にするので、出す候補は方向の数より多くできない
	static_assert(OfferCount <= CommandDirections.size());
	static_assert(OfferCount <= static_cast<size_t>(BuffType::Count));

	//並びをランダムに入れ替える（Fisher-Yates）
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

	//強化の種類からOfferCount個を選び、1つずつ候補を作る
	//mustHealがtrueなら、回復を必ず候補に入れる
	//1文字目はタイプごとに別の方向にして、打ち始めでどの候補を選んだか決まるようにする
	std::vector<CommandCandidate> MakeCandidates(const PlayerBuff& buff, bool mustHeal)
	{
		//タイプをシャッフルして先頭から選ぶ
		std::array<BuffType, static_cast<size_t>(BuffType::Count)> types{};
		for (size_t index = 0; index < types.size(); ++index)
			types[index] = static_cast<BuffType>(index);
		Shuffle(types);

		//回復が選ばれる範囲の外にあれば、先頭と入れ替えて選ばれるようにする
		if (mustHeal)
		{
			auto heal = std::find(types.begin(), types.end(), BuffType::Heal);
			if (heal >= types.begin() + OfferCount) std::iter_swap(types.begin(), heal);
		}

		//HUDの並びが毎回変わらないよう、選んだものはタイプ順に戻す
		std::sort(types.begin(), types.begin() + OfferCount);

		//方向をシャッフルして、先頭から順に各候補の1文字目に使う
		auto firstKeys = CommandDirections;
		Shuffle(firstKeys);

		std::vector<CommandCandidate> candidates;
		for (size_t index = 0; index < OfferCount; ++index)
		{
			const BuffType type = types[index];
			//次に上がる段階で、方向の数と時間を決める。最大段階でも、時間を戻すために入力できる
			//回復は段階を持たないので、いつも段階1の長さになる
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

	//HPがこの割合以下なら、回復を必ず候補に入れる
	bool IsPinch(const PlayerStatus& status)
	{
		return status.GetHealth() <= status.GetMaxHealth() * PinchHpRatio;
	}

	//入力しきったコマンドの効果を出す。回復はその場でHPを戻し、それ以外は強化をかける
	void ApplyCommand(PlayerController& controller, BuffType type)
	{
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
			if (!command->Select(keys[index])) continue;//どの候補の1文字目でもないキーは無視する

			//同じフレームに続けて押されたキーは、選んだ候補の2文字目以降として渡す
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

		for (InputKeyName key : owner().GetCurrentInput().commandList.commands)
		{
			if (command->Input(key) == CommandResult::Completed)
			{
				ApplyCommand(owner(), command->GetType());
				stateMachine().Dispatch(CommandEvent::EventCommandEnd);
				return;
			}
		}

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
