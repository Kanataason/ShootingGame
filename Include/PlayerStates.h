#pragma once
#include <algorithm>

#include "Component.h"
#include "StateMachine.h"
#include "GunState.h"
#include "TimedState.h"

//使いやすくするためにusingでステートマシーンのStateを宣言
class PlayerController;
class PlayerAttack;
using PlayerStateBase = StateMachine<PlayerController>::State;

namespace PlayerStates
{
    //EnemyStatesにも同名のenumがあるため、namespaceの中に置いて衝突を防ぐ
    enum LifeEvent
    {
        EventDamaged,
        EventInvincibleEnd,
        EventDied,
    };
    enum AttackEvent
    {
        EventLongShot,
        EventSpreadShot,
        EventAttackStop,
    };
    enum CommandEvent
    {
        EventCommandOffer,
        EventCommandStart,
        EventCommandEnd,
        EventCommandStop,
    };

    //通常状態：入力で移動する
    class AliveState : public PlayerStateBase
    {
    protected:
        void OnUpdate(float deltaTime) override;
        void OnDraw()override;
    };

    //被弾後の無敵：一定時間当たり判定をなくして点滅する。移動はできる
    class InvincibleState : public TimedState<PlayerController>
    {
    public:
        InvincibleState() : TimedState(1.0f) {}
    protected:
        void OnEnter(State* previousState) override;
        void OnUpdate(float deltaTime) override;
        void OnExit(State* nextState) override;
        void OnTimeUp() override;
        void OnDraw() override;
    };

    //撃破エフェクトを出し、終わったらOnDeathFinishedで知らせる
    class DeadState : public TimedState<PlayerController>
    {
    public:
        DeadState() : TimedState(2.5f) {}//GameOverまでの時間
    protected:
        void OnEnter(State* previousState) override;
        void OnTimeUp() override;
        void OnDraw()override;
    private:
        float _effectTime = 1.0f;//エフェクトが広がる時間
    };


    //攻撃ステートの基底クラス
    //ボタンを押している間撃ち続け、切り替えボタンで別の攻撃ステートへ移る
    //入力・クールタイム・ヒート・切り替えはここで行うので、継承先はShootだけ書けばよい
    //武器を増やすときは、継承したステートを作り、PlayerControllerで切り替えの輪に入れる

    //攻撃ステートの基底クラス
    //ボタンを押している間撃ち続け、切り替えボタンで別の攻撃ステートへ移る
    class PlayerShotState : public PlayerStateBase, public GunState
    {
    protected:
        PlayerShotState(size_t attackIndex, float shotCoolTime, float heatPerShot, AttackEvent switchEvent)
            : _attackIndex(attackIndex), _shotCoolTime(shotCoolTime), _heatPerShot(heatPerShot), _switchEvent(switchEvent) {
        }

        void OnEnter(State* previousState) override;
        void OnUpdate(float deltaTime) override;

        //1回分の撃ち方
        virtual void Shoot(PlayerAttack& attack) = 0;
    private:
        float _heatPerShot = 0.0f; //銃によってヒート値を変える
        size_t _attackIndex;//JSONで宣言したAttackTypeの何番目を使うか
        float _shotCoolTime;
        AttackEvent _switchEvent;//切り替えボタンで移る先
    };

    class LongRangeShot : public PlayerShotState
    {
    public:
        LongRangeShot() : PlayerShotState(0, 0.1f, HeatAmount, EventSpreadShot) {}
    protected:
        void Shoot(PlayerAttack& attack) override;
    private:
        static constexpr float HeatAmount = 6.0f;//1発あたりのヒート量
    };

    class SpreadShotState : public PlayerShotState
    {
    public:
        SpreadShotState() : PlayerShotState(1, 0.7f, HeatAmount, EventLongShot) {}
    protected:
        void Shoot(PlayerAttack& attack) override;
    private:
        static constexpr float HeatAmount = 32.0f;//1発あたりのヒート量
    };

    class AttackDisabledState : public PlayerStateBase {};

    //コマンドの待機：一定時間たったら、強化の候補を出して選択待ちへ移る
    //入る時に、前のコマンドの表示を消す
    class CommandIdleState : public TimedState<PlayerController>
    {
    public:
        CommandIdleState() : TimedState(OfferInterval) {}
    protected:
        void OnEnter(State* previousState) override;
        void OnTimeUp() override;
    private:
        static constexpr float OfferInterval = 5.0f;//前のコマンドが終わってから、次の候補を出すまでの秒数
    };

    //コマンドの選択待ち：候補の1文字目が打たれたら、その候補の入力中へ移る
    //見送ってもペナルティは無く、時間切れで待機へ戻る
    class CommandOfferState : public PlayerStateBase
    {
    protected:
        void OnUpdate(float deltaTime) override;
    };

    //コマンドの入力中：方向キーを判定し、成功したら強化を付けて待機へ戻る。時間切れも待機へ戻る
    //入力中も移動と射撃はできる。別のステートマシンで動いているため
    class CommandInputState : public PlayerStateBase
    {
    protected:
        void OnUpdate(float deltaTime) override;
    };

    //死亡後：コマンドを出さない。入る時に表示を消す
    class CommandDisabledState : public PlayerStateBase
    {
    protected:
        void OnEnter(State* previousState) override;
    };
}
