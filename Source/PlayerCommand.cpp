#include "PlayerCommand.h"

void PlayerCommand::Offer(std::vector<CommandCandidate> candidates, float offerTime)
{
	_view = {};
	_view.phase = CommandPhase::Offer;
	_view.candidates = std::move(candidates);
	_view.remaining = offerTime;
	_view.timeLimit = offerTime;
	ResetProgress();
	OnCommandChanged.Invoke(_view);
}

void PlayerCommand::NotifyResult(CommandResult result)
{
	if (CommandResult::Progress == result)return;

	if (CommandResult::TimeOut == result && _view.phase != CommandPhase::Input)return;

	OnCommandResult.Invoke(result);
}

bool PlayerCommand::Select(InputKeyName key)
{
	if (_view.phase != CommandPhase::Offer) return false;

	for (const auto& candidate : _view.candidates)
	{
		if (candidate.sequence.empty() || candidate.sequence.front() != key) continue;

		//選んだ候補に切り替えて、時間も測り直す
		_view.phase = CommandPhase::Input;
		_view.type = candidate.type;
		_view.sequence = candidate.sequence;
		_view.remaining = candidate.timeLimit;
		_view.timeLimit = candidate.timeLimit;
		_view.candidates.clear();//candidateはこの中を指しているので、使い終わってから消す
		ResetProgress();

		Judge(key);//選んだキーが1文字目になる
		OnCommandChanged.Invoke(_view);
		return true;
	}
	return false;
}

void PlayerCommand::Stop()
{
	_view = {};
	ResetProgress();
	OnCommandChanged.Invoke(_view);
}

CommandResult PlayerCommand::Input(InputKeyName key)
{
	if (_view.phase != CommandPhase::Input) return CommandResult::Progress;

	CommandResult result = Judge(key);
	OnCommandChanged.Invoke(_view);
	NotifyResult(result);//成功・誤入力
	return result;
}

CommandResult PlayerCommand::Judge(InputKeyName key)
{
	if (_pendingKey != InputKeyName::none)
	{
		//同時押しで逆順に来たら、入れ替えて両方正解にする
		bool isSwapped = key == Expected(_view.progress) && _pendingKey == Expected(_view.progress + 1);
		_pendingKey = InputKeyName::none;
		if (!isSwapped)
		{
			ResetProgress();
			return CommandResult::Missed;
		}
		_view.progress += 2;
	}
	else if (key == Expected(_view.progress))
	{
		++_view.progress;
	}
	else
	{
		//すぐミスにせず、猶予の間は次のキーを待つ
		_pendingKey = key;
		_graceRemaining = GraceTime;
		return CommandResult::Progress;
	}

	return (_view.progress >= static_cast<int>(_view.sequence.size()))
		? CommandResult::Completed
		: CommandResult::Progress;
}

CommandResult PlayerCommand::Tick(float deltaTime)
{
	if (_view.phase == CommandPhase::None) return CommandResult::Progress;

	CommandResult result = CommandResult::Progress;

	//猶予切れ。保留したキーをミスにする
	if (_pendingKey != InputKeyName::none)
	{
		_graceRemaining -= deltaTime;
		if (_graceRemaining <= 0.0f)
		{
			ResetProgress();
			result = CommandResult::Missed;
		}
	}

	_view.remaining -= deltaTime;
	if (_view.remaining <= 0.0f)
	{
		_view.remaining = 0.0f;
		result = CommandResult::TimeOut;
	}

	OnCommandChanged.Invoke(_view);
	NotifyResult(result);
	return result;
}

void PlayerCommand::ResetProgress()
{
	_view.progress = 0;
	_pendingKey = InputKeyName::none;
	_graceRemaining = 0.0f;
}

InputKeyName PlayerCommand::Expected(int index) const
{
	if (index < 0 || index >= static_cast<int>(_view.sequence.size())) return InputKeyName::none;
	return _view.sequence[index];
}
