#include "GameHUD.h"
#include <algorithm>
#include <cmath>
#include <DxLib.h>

#include "GameWindow.h"

namespace
{
	//バーの高さと、ラベルをバーの何ピクセル上に書くか
	constexpr int BarHeight = 12;
	constexpr int LabelOffsetY = 18;

	constexpr int BarBackColor = 0x333333;
	constexpr int FrameColor = 0xFFFFFF;
	constexpr int TextColor = 0xFFFFFF;

	constexpr float PlayerDangerRatio = 0.3f;
	constexpr float HeatWarningRatio = 0.7f;

	//コマンドの表示位置と、方向どうしの間隔。候補は1行に1つずつ、強化の名前の右に方向を並べる
	constexpr int CommandX = 220;
	constexpr int CommandY = 60;
	constexpr int CommandNameWidth = 80;
	constexpr int CommandSpacing = 24;
	constexpr int CommandLineHeight = 20;
	constexpr int CommandDoneColor = 0x777777;
	constexpr int CommandNextColor = 0xFFFF00;
	constexpr int OfferTimeColor = 0xAAAAAA;//候補が消えるまでの時間
	constexpr int InputTimeColor = 0x00CCFF;//入力し終えるまでの時間

	constexpr int BuffX = 480;
	constexpr int BuffY = 380;//4種類とコンボの5行が、画面の下に収まる高さ
	constexpr int BuffLineHeight = 20;
	constexpr int ComboColor = 0xFF9922;//攻撃の赤と速度の黄の間

	//右上の、ウェーブと残り秒数
	constexpr int WaveInfoX = 440;
	constexpr int WaveInfoY = 10;

	//画面の真ん中に出す大きな文字
	constexpr int BannerY = 180;
	constexpr double BannerScale = 3.0;   //普通の文字の何倍で描くか
	constexpr float BannerShowTime = 2.0f;//出している秒数
	constexpr float BannerFadeTime = 0.5f;//そのうち、最後に消えていく秒数

	const char* GetBuffName(BuffType type)
	{
		switch (type)
		{
		case BuffType::Attack:  return "ATTACK";
		case BuffType::Defense: return "DEFENSE";
		case BuffType::Speed:   return "SPEED";
		case BuffType::Predict: return "PREDICT";
		case BuffType::Heal:    return "HEAL";
		default:                return "";
		}
	}
	const char* GetArrowText(InputKeyName key)
	{
		switch (key)
		{
		case InputKeyName::up:    return "↑";
		case InputKeyName::down:  return "↓";
		case InputKeyName::left:  return "←";
		case InputKeyName::right: return "→";
		default:                  return "";
		}
	}
}

void GameHUD::Initialize(const ServiceLocator& locator)
{
	_screenWidth = static_cast<int>(locator.gameWindow->GetBounds().width);
}

void GameHUD::Draw() const
{
	DrawPlayerHp();
	DrawHeat();
	DrawBossInfo();
	DrawCommand();
	DrawCommandResult();//方向列の上に重ねる
	DrawBuffs();
	DrawBanner();
}

void GameHUD::DrawPlayerHp() const
{
	float ratio = ToRatio(_playerHp, _playerMaxHp);
	int color = (ratio <= PlayerDangerRatio) ? 0xFF0000 : 0x00FF88;
	DrawBar(10, 450, 150, ratio, color, "PLAYER");
}

void GameHUD::DrawBossInfo() const
{
	if (_isBossVisible)
	{
		DrawBar(120, 20, 400, ToRatio(_bossHp, _bossMaxHp), 0xFF3333, "BOSS");
		return;
	}

	//切り上げて、0.5秒残っていれば「1秒」と出す。マイナスにはしない
	//DxLib経由で入るWindowsのmaxマクロと衝突しないように、かっこで囲む
	const int remaining = (std::max)(static_cast<int>(std::ceil(_waveRemaining)), 0);
	DrawFormatString(WaveInfoX, WaveInfoY, TextColor, "WAVE %d/%d  あと %d 秒", _wave, _waveCount, remaining);
}

void GameHUD::DrawBanner() const
{
	const float elapsed = Time::GetWorldTime() - _bannerTime;
	if (_banner.empty() || elapsed >= BannerShowTime) return;

	//最後のBannerFadeTime秒で、だんだん消える
	const float fade = (std::min)((BannerShowTime - elapsed) / BannerFadeTime, 1.0f);

	//画面の横の真ん中に来るよう、拡大したあとの幅から左端を決める
	const int width = static_cast<int>(GetDrawStringWidth(_banner.c_str(), static_cast<int>(_banner.size())) * BannerScale);
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, static_cast<int>(255 * fade));
	DrawExtendString((_screenWidth - width) / 2, BannerY, BannerScale, BannerScale, _banner.c_str(), _bannerColor);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

void GameHUD::DrawHeat() const
{
	float ratio = ToRatio(_heat, _maxHeat);
	int color = _isOverHeat ? 0xFF0000
		: (ratio >= HeatWarningRatio) ? 0xFF8800
		: 0xFFFF00;
	const char* label = _isOverHeat ? "OVER HEAT" : "HEAT";
	DrawBar(10, 400, 100, ratio, color, label);
}
void GameHUD::DrawCommandResult()const
{
	if (_result == CommandResult::Progress) return;//まだ結果が出ていない

	const float elapsed = Time::GetWorldTime() - _resultTime;
	if (elapsed >= _resultShowTime)return;

	const char* text = (_result == CommandResult::Completed) ? "SUCCESS!"
		: (_result == CommandResult::Missed) ? "MISS"
		: "TIME UP";
	const int color = (_result == CommandResult::Completed) ? 0x00FF88
		: (_result == CommandResult::Missed) ? 0xFF4444
		: 0xAAAAAA;

	//だんだん消える
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, static_cast<int>(255 * (1.0f - elapsed / _resultShowTime)));
	DrawStringToHandle(CommandX, CommandY - 28, text, color, DX_DEFAULT_FONT_HANDLE);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

}

void GameHUD::DrawCommand() const
{
	int lineCount = 0;
	int timeColor = TextColor;

	switch (_command.phase)
	{
	case CommandPhase::Offer:
		//どの候補も、選ぶために打つ1文字目を黄色で出す
		for (const auto& candidate : _command.candidates)
			DrawCommandLine(CommandY + lineCount++ * CommandLineHeight, candidate.type, candidate.sequence, 0);
		timeColor = OfferTimeColor;
		break;
	case CommandPhase::Input:
		DrawCommandLine(CommandY, _command.type, _command.sequence, _command.progress);
		lineCount = 1;
		timeColor = InputTimeColor;
		break;
	default:
		return;
	}

	//残り時間。ラベルは強化の名前で出しているので空にする
	DrawBar(CommandX, CommandY + lineCount * CommandLineHeight, 150,
		ToRatio(_command.remaining, _command.timeLimit), timeColor, "");
}

void GameHUD::DrawCommandLine(int y, BuffType type, const std::vector<InputKeyName>& sequence, int progress)
{
	DrawString(CommandX, y, GetBuffName(type), TextColor);
	for (size_t index = 0; index < sequence.size(); ++index)
	{
		const int order = static_cast<int>(index);
		int color = (order < progress) ? CommandDoneColor
			: (order == progress) ? CommandNextColor
			: TextColor;
		DrawString(CommandX + CommandNameWidth + order * CommandSpacing, y, GetArrowText(sequence[index]), color);
	}
}

void GameHUD::DrawBuffs() const
{
	int line = 0;
	for (size_t index = 0; index < _buffs.size(); ++index)
	{
		const BuffSlot& buff = _buffs[index];
		if (buff.level == 0) continue;

		DrawFormatString(BuffX, BuffY + line * BuffLineHeight, TextColor, "%s Lv%d %.1f",
			GetBuffName(static_cast<BuffType>(index)), buff.level, buff.remaining);
		++line;
	}

	//攻撃と速度が両方かかっている間は、コンボ中だと出す
	const bool isCombo = _buffs[static_cast<size_t>(BuffType::Attack)].level > 0
		&& _buffs[static_cast<size_t>(BuffType::Speed)].level > 0;
	if (isCombo)
		DrawFormatString(BuffX, BuffY + line * BuffLineHeight, ComboColor, "COMBO MOVE x%.2f", PlayerBuff::ComboRate);
}

void GameHUD::DrawBar(int x, int y, int width, float ratio, int color, const char* label)
{
	DrawString(x, y - LabelOffsetY, label, TextColor);
	DrawBox(x, y, x + width, y + BarHeight, BarBackColor, TRUE);                             //背景
	DrawBox(x, y, x + static_cast<int>(width * ratio), y + BarHeight, color, TRUE);         //中身
	DrawBox(x, y, x + width, y + BarHeight, FrameColor, FALSE);                             //枠
}

float GameHUD::ToRatio(float value, float max)
{
	if (max <= 0.0f) return 0.0f;
	//HPがマイナスになってもバーが逆に伸びないように、0から1に収める
	return std::clamp(value / max, 0.0f, 1.0f);
}
