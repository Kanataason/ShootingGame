#pragma once
#include <vector>
#include <deque>
#include <optional>

#include "Component.h"
#include "Event.h"
#include "InputManager.h"
#include "PlayerBuff.h"

enum class CommandResult
{
	Progress,  //まだ途中
	Completed, //最後まで正しく入力できた
	Missed,    //誤入力。最初からやり直し
	TimeOut    //時間切れ。強化は不発
};

enum class CommandPhase
{
	None,  //何も出していない
	Offer, //候補を出して、選ばれるのを待っている
	Input  //選ばれた候補を入力している
};

//強化1タイプ分の候補。1文字目を打つと、この候補が選ばれる
struct CommandCandidate
{
	BuffType type = BuffType::Count;
	std::vector<InputKeyName> sequence;//お手本の方向列
	float timeLimit = 0.0f;            //選んでから入力し終えるまでの秒数
};

//HUDへ送る、コマンドの見た目
struct CommandView
{
	CommandPhase phase = CommandPhase::None;
	std::vector<CommandCandidate> candidates;//Offerの間に出す候補

	//ここから下はInputの間だけ使う
	BuffType type = BuffType::Count;
	std::vector<InputKeyName> sequence;//お手本の方向列
	int progress = 0;                  //入力済みの数

	//OfferとInputのどちらでも、今の段階の残り時間
	float remaining = 0.0f;
	float timeLimit = 0.0f;

	//アイテムを拾ったまま、出番を待っている強化。古い順
	//今のコマンドが終わると、先頭から順に出る
	std::vector<BuffType> queued;
};

//コマンドの候補と入力の進み具合を持ち、入力を判定する
//候補を出すタイミングと成功時の処理はステート側
class PlayerCommand : public Component
{
public:
	Event<const CommandView&> OnCommandChanged;//HUDへ送るイベント
	Event<CommandResult> OnCommandResult;// 成功時や失敗時に呼んで、HUDに結果を送るイベント

	std::unique_ptr<Component> Clone()const override
	{
		return std::make_unique<PlayerCommand>(*this);
	}

	void Initialize(ServiceLocator& locator)override {}
	void Update(float deltaTime)override {}

	//候補を出して、選ばれるのを待つ。offerTime秒たっても選ばれなければ時間切れ
	void Offer(std::vector<CommandCandidate> candidates, float offerTime);
	//1文字目が合う候補を選び、そのキーを1文字目として入力を始める。合う候補が無ければfalse
	bool Select(InputKeyName key);
	//候補も入力もやめて、HUDから消す
	void Stop();

	//選ばれた方向列に、方向を1つずつ渡す
	CommandResult Input(InputKeyName key);
	//残り時間と、同時押しの猶予を減らす。毎フレーム1回呼ぶ
	CommandResult Tick(float deltaTime);

	BuffType GetType() const { return _view.type; }

	//拾ったアイテムの強化を、出番待ちの最後に足す
	//コマンドの入力中に拾っても無駄にならないよう、今のコマンドが終わるまで取っておく
	void Request(BuffType type);
	//出番待ちの先頭を取り出す。待っているものが無ければ空。コマンドの待機ステートが読む
	std::optional<BuffType> PopRequest();

private:
	//今の見た目に出番待ちを入れて、HUDに送る
	//_viewは候補を出すたびに作り直すので、出番待ちは別に持ち、送る直前に入れる
	void NotifyChanged();
	//成功時や失敗時に呼んで、HUDに結果を表示させる
	void NotifyResult(CommandResult result);

	CommandResult Judge(InputKeyName key);
	void ResetProgress();
	//index番目のお手本。範囲外ならnone
	InputKeyName Expected(int index) const;

	//同時押しで順番が逆になったときの猶予
	static constexpr float GraceTime = 0.05f;

	CommandView _view;
	std::deque<BuffType> _requests;//出番待ち。拾った順
	InputKeyName _pendingKey = InputKeyName::none;//猶予中で保留しているキー
	float _graceRemaining = 0.0f;
};
