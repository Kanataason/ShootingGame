#include <typeinfo>
#include <cmath>
#include "PlayerController.h"
#include "GameObject.h"
#include "GameTime.h"
#include "SceneManager.h"

#include "PlayerMovement.h"
#include "PlayerAttack.h"
#include "PlayerStatus.h"
#include "PlayerBuff.h"
#include "PlayerCommand.h"
#include "Collider.h"
#include "DrawManager.h"
#include "EffectUtillity.h"

#include "PlayerStates.h"

using namespace PlayerStates;
using namespace InputKey;

namespace
{
	//強化中の機体の光
	constexpr float GlowPulseSpeed = 6.0f;//約1秒で1回
	constexpr int GlowBaseAlpha = 80;     //基本の濃さ
	constexpr int GlowPulseAlpha = 80;    //脈の濃さの幅

	//オーラの半径。強化ごとに外へずらす
	constexpr float AuraRadius = 26.0f;
	constexpr float AuraRadiusStep = 7.0f;

	//コンボ中の色の切り替え間隔
	constexpr float ComboBlinkInterval = 0.2f;

	//回復エフェクトの長さ
	constexpr float HealEffectTime = 0.6f;
}

void PlayerController::Initialize(ServiceLocator& locator)
{
	_movement = GetOwner()->GetComponent<PlayerMovement>();
	_attack = GetOwner()->GetComponent<PlayerAttack>();
	_status = GetOwner()->GetComponent<PlayerStatus>();
	_buff = GetOwner()->GetComponent<PlayerBuff>();
	_command = GetOwner()->GetComponent<PlayerCommand>();
	_sizeStatus = GetOwner()->GetComponent<CircleShape>();
	_collider = GetOwner()->GetComponent<Collider>();

	SubScribeEvents();

	InitializeStateMachine();

	_soundManager = locator.soundManager;
	_input = locator.input;
	_sceneManager = locator.sceneManager;
	_drawManager = locator.drawManager;
}
void PlayerController::SubScribeEvents()
{
	//回復では無敵にしない
	_status->OnDamaged.Subscribe([this]
		{
			if (_status->GetHealth() > 0.0f) _normalState.Dispatch(LifeEvent::EventDamaged);
		});
	//回復したら緑の輪を出す
	_status->OnHealed.Subscribe([this] { _healEffectStartTime = Time::GetWorldTime(); });
	_status->OnHitSound.Subscribe([this](SoundId id) {PlaySe(id);});
}

void PlayerController::InitializeStateMachine()
{


	//各ステートマシーンの遷移先を登録
	_normalState.AddTransition<AliveState, InvincibleState>(LifeEvent::EventDamaged);
	_normalState.AddTransition<InvincibleState, AliveState>(LifeEvent::EventInvincibleEnd);


	//武器は切り替えボタンで順番に回る
    //武器を増やしたら遷移とswitchEventもつなぎ直す
    //切り替わらないときは遷移の登録漏れ
	_attackState.AddTransition<LongRangeShot, SpreadShotState>(AttackEvent::EventSpreadShot);
	_attackState.AddTransition<SpreadShotState, LongRangeShot>(AttackEvent::EventLongShot);

	//コマンド：待機→候補→入力→待機
	_commandState.AddTransition<CommandIdleState, CommandOfferState>(CommandEvent::EventCommandOffer);
	_commandState.AddTransition<CommandOfferState, CommandInputState>(CommandEvent::EventCommandStart);
	_commandState.AddTransition<CommandOfferState, CommandIdleState>(CommandEvent::EventCommandEnd);
	_commandState.AddTransition<CommandInputState, CommandIdleState>(CommandEvent::EventCommandEnd);

	//すべてのステートから遷移できるステートを登録
	_normalState.AnyAddTransition<DeadState>(LifeEvent::EventDied);
	_attackState.AnyAddTransition<AttackDisabledState>(AttackEvent::EventAttackStop);
	_commandState.AnyAddTransition<CommandDisabledState>(CommandEvent::EventCommandStop);

	//最初のステートを設定
	_attackState.Start<LongRangeShot>();
	_normalState.Start<AliveState>();
	_commandState.Start<CommandIdleState>();
}

void PlayerController::Update(float deltaTime)
{
	//死んだらすべてのステートを死亡用に変える
	if (!_status->GetIsActive() && !_isDead)
	{
		_isDead = true;
		_normalState.Dispatch(LifeEvent::EventDied);   
		_attackState.Dispatch(AttackEvent::EventAttackStop);
		_commandState.Dispatch(CommandEvent::EventCommandStop);
	}



	_currentInput = ReadInput();
	_normalState.Update(deltaTime);
	_attackState.Update(deltaTime);
	_commandState.Update(deltaTime);
}
void PlayerController::Draw()
{
	//本体・点滅・撃破エフェクトの描き分けはステートが行う
	_drawManager->Register(DrawLayer::Player, [this] { _normalState.Draw(); });
	_drawManager->Register(DrawLayer::Effect, [this] { DrawHealEffect(); });
}
void PlayerController::DrawHealEffect() const
{
	const float elapsed = Time::GetWorldTime() - _healEffectStartTime;
	if (elapsed >= HealEffectTime) return;

	EffectUtillity::DrawEffect(elapsed, HealEffectTime, GetBuffColor(BuffType::Heal), GetOwner()->GetTransform().Position);
}
void PlayerController::DrawBody() const
{
	if (!_status->GetIsActive())return;

	const auto& pos = GetOwner()->GetTransform().Position;

	//オーラは機体の下に描く
	for (size_t index = 0; index < static_cast<size_t>(BuffType::Count); ++index)
	{
		const BuffType type = static_cast<BuffType>(index);
		const int level = _buff->GetLevel(type);
		if (level == 0) continue;
		EffectUtillity::DrawAura(pos, AuraRadius + index * AuraRadiusStep, GetBuffColor(type), level, Time::GetWorldTime());
	}

	//座標が画像の中心になる。画像は表示する大きさに加工済みなので、等倍で描く
	DrawRotaGraph((int)pos.x, (int)pos.y, 1.0, 0.0, _status->GetImagePath(), TRUE);

	//強化中は同じ画像を色付きで加算して光らせる
	const int color = GetBodyColor();
	if (color == -1) return;

	const int red = (color >> 16) & 0xFF;
	const int green = (color >> 8) & 0xFF;
	const int blue = color & 0xFF;

	//0〜1で脈打つ
	const float pulse = 0.5f + 0.5f * std::sin(Time::GetWorldTime() * GlowPulseSpeed);

	SetDrawBright(red, green, blue);
	SetDrawBlendMode(DX_BLENDMODE_ADD, GlowBaseAlpha + static_cast<int>(pulse * GlowPulseAlpha));
	DrawRotaGraph((int)pos.x, (int)pos.y, 1.0, 0.0, _status->GetImagePath(), TRUE);
	//戻さないと他の描画にも効く
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	SetDrawBright(255, 255, 255);
}
int PlayerController::GetBodyColor() const
{
	//コンボ中は赤と黄を交互に
	if (_buff->IsComboActive())
	{
		const bool isAttackTurn = std::fmod(Time::GetWorldTime(), ComboBlinkInterval * 2.0f) < ComboBlinkInterval;
		return GetBuffColor(isAttackTurn ? BuffType::Attack : BuffType::Speed);
	}

	int bestLevel = 0;
	int color = -1;
	for (size_t index = 0; index < static_cast<size_t>(BuffType::Count); ++index)
	{
		const BuffType type = static_cast<BuffType>(index);
		const int level = _buff->GetLevel(type);
		if (level > bestLevel)
		{
			bestLevel = level;
			color = GetBuffColor(type);
		}
	}
	return color;
}
void PlayerController::MoveByInput(float deltaTime, float moveSpeed)
{
	//速度の強化中は移動を速くする
	_movement->Move(_currentInput.direction, moveSpeed * _buff->GetSpeedRate() * deltaTime);
	_movement->MovementRestrictions();
}
void PlayerController::PlaySe(SoundId id)
{
	_soundManager->PlaySe(id);
}

PlayerInput PlayerController::ReadInput()const
{
	PlayerInput input;

	if (_input->GetKey(rightKey)) input.direction.x += 1.0f;
	if (_input->GetKey(leftKey))  input.direction.x -= 1.0f;
	if (_input->GetKey(upKey))    input.direction.y -= 1.0f;   
	if (_input->GetKey(downKey))  input.direction.y += 1.0f;

	input.isShotHeld = _input->GetKey(shotKey);
	input.isChangePressed = _input->GetKeyDown(changeKey);

	input.commandList = ReadInputCommand();

	return input;
}

InputCommand PlayerController::ReadInputCommand()const
{
	InputCommand input;

	if (_input->GetKeyDown(leftArrowKey))input.commands.push_back(InputKeyName::left);
	if (_input->GetKeyDown(rightArrowKey))input.commands.push_back(InputKeyName::right);
	if (_input->GetKeyDown(upArrowKey))input.commands.push_back(InputKeyName::up);
	if (_input->GetKeyDown(downArrowKey))input.commands.push_back(InputKeyName::down);

	return input;
}

