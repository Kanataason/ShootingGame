#pragma once
#include <vector>
#include <deque>
#include <optional>

#include "Component.h"
#include "PlayerBuff.h"
#include "StateMachine.h"
#include "InputManager.h"
#include "SoundManager.h"

class PlayerMovement;
class PlayerAttack;
class PlayerStatus;
class PlayerBuff;
class PlayerCommand;
class Collider;
class DrawManager;

namespace InputKey
{
	constexpr int upKey = 'W';
	constexpr int downKey = 'S';
	constexpr int leftKey = 'A';
	constexpr int rightKey = 'D';

	constexpr int leftArrowKey = VK_LEFT;
	constexpr int rightArrowKey = VK_RIGHT;
	constexpr int upArrowKey = VK_UP;
	constexpr int downArrowKey = VK_DOWN;

	constexpr int shotKey = VK_RETURN;
	constexpr int changeKey = 'R';
}


class PlayerController : public Component
{
public:
	std::unique_ptr<Component> Clone()const override
	{
		return std::make_unique<PlayerController>(*this);
	}
	PlayerController() :_normalState(*this), _attackState(*this), _commandState(*this) {}

	PlayerController(const PlayerController& source)
		: Component(source),_normalState(*this),_attackState(*this),_commandState(*this)
	{
	}

	void Initialize(ServiceLocator& locator)override;
	void Update(float deltaTime)override;
	void Draw()override;
	void DrawBody() const;

	//一フレームの入力を構造体に入れてStateMachineから値を取得できる状態にする
	PlayerInput ReadInput()const;
	InputCommand ReadInputCommand() const;

	const PlayerInput& GetCurrentInput()const { return _currentInput; }
	//移動中か（コンボ用）
	bool IsMoving()const { return _currentInput.direction.x != 0.0f || _currentInput.direction.y != 0.0f; }
    PlayerMovement* GetMovement()const { return _movement; }
	PlayerAttack* GetAttack()const { return _attack; }
	PlayerStatus* GetStatus()const { return _status; }
	PlayerBuff* GetBuff()const { return _buff; }
	PlayerCommand* GetCommand()const { return _command; }
	Collider* GetCollider() const { return _collider; }
	SoundManager* GetSoundManager()const { return _soundManager; }

	void PlaySe(SoundId id);
	void MoveByInput(float deltaTime,float moveSpeed);

	//アイテムを拾ったら、そのアイテムの強化のコマンドを出す予約をする
	void OnPickup(GameObject& item)override;
	//一番古い予約を取り出す。予約が無ければ空。コマンドの待機ステートが読む
	std::optional<BuffType> PopRequestedBuff();
	//難易度による、コマンドの制限時間の倍率。コマンドを作るステートが読む
	float GetCommandTimeRate() const { return _commandTimeRate; }

	//Moveを禁止
	PlayerController(PlayerController&&) = delete;
	PlayerController& operator=(PlayerController&&) = delete;
	PlayerController& operator=(const PlayerController&) = delete;
	
private:
	void SubScribeEvents();
	void InitializeStateMachine();
	//一番段階が高い強化の色。同じ段階なら先のタイプ。強化が無ければ-1
	//コンボ中は2色を交互に返す
	int GetBodyColor() const;
	//回復エフェクト
	void DrawHealEffect() const;
private:
	bool _isDead = false;
	//コマンドの入力中に拾っても無駄にならないよう、待機に戻るまで拾った順に取っておく
	std::deque<BuffType> _requestedBuffs;
	float _commandTimeRate = 1.0f;
	float _healEffectStartTime = -100.0f;//最初は表示しない

	PlayerInput _currentInput{};

	InputManager* _input = nullptr;
	PlayerMovement* _movement = nullptr;
	PlayerAttack* _attack = nullptr;
	PlayerStatus* _status = nullptr;
	PlayerBuff* _buff = nullptr;
	PlayerCommand* _command = nullptr;
	Collider* _collider = nullptr;

	SoundManager* _soundManager = nullptr;
	DrawManager* _drawManager = nullptr;

	StateMachine<PlayerController> _normalState;
	StateMachine<PlayerController> _attackState;
	StateMachine<PlayerController> _commandState;
};