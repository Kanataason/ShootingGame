#pragma once
#include <array>
#include <string>

#include "GameTime.h"
#include "ServiceLocator.h"
#include "GameProgress.h"
#include "PlayerBuff.h"
#include "PlayerCommand.h"

//ゲーム中のUIを描画する
//プレイヤーのHP・ヒートゲージ・ボスのHPか、今のウェーブ・入力中のコマンド・かかっている強化
//値はGameSceneがEventで受け取ってSetで入れる
class GameHUD
{
public:
	//ShowBannerで使う色
	static constexpr int WaveBannerColor = 0x00CCFF;
	static constexpr int BossBannerColor = 0xFF3333;

	void Initialize(const ServiceLocator& locator);
	void Draw() const;

	//今のウェーブと、そのウェーブの残り秒数
	void SetWave(int wave, int waveCount, float remaining) { _wave = wave; _waveCount = waveCount; _waveRemaining = remaining; }
	//画面の真ん中に大きな文字を一定時間出す。ウェーブの始まりやボスの登場に使う
	void ShowBanner(std::string text, int color) { _banner = std::move(text); _bannerColor = color; _bannerTime = Time::GetWorldTime(); }

	void SetPlayerHp(float hp, float maxHp) { _playerHp = hp; _playerMaxHp = maxHp; }
	//呼ばれた時点でボスが出たとみなし、残り撃破数の代わりにHPバーを出す
	void SetBossHp(float hp, float maxHp) { _bossHp = hp; _bossMaxHp = maxHp; _isBossVisible = true; }
	void SetHeat(float heat, float maxHeat, bool isOverHeat) { _heat = heat; _maxHeat = maxHeat; _isOverHeat = isOverHeat; }
	void SetCommand(const CommandView& command) { _command = command; }
	void SetBuff(BuffType type, int level, float remaining) { _buffs[static_cast<size_t>(type)] = { level, remaining }; }
	void SetCommandResult(CommandResult result) { _result = result; _resultTime = Time::GetWorldTime(); }
private:
	//コマンドの結果を、一定時間だけ画面に出す
	void DrawCommandResult() const;

	//残り3割以下で赤くする
	void DrawPlayerHp() const;
	//ボスが出ていればHPバー、出ていなければ今のウェーブと残り秒数
	void DrawBossInfo() const;
	//ShowBannerの文字。最後のほうでだんだん消える
	void DrawBanner() const;
	//オーバーヒート中は赤、7割以上はオレンジ、通常は黄
	void DrawHeat() const;
	//コマンドの候補か、入力中のコマンド。入力済みは灰色、次に打つ方向は黄色で出す
	void DrawCommand() const;
	//強化の名前と方向列を1行描く。progressは入力済みの数
	static void DrawCommandLine(int y, BuffType type, const std::vector<InputKeyName>& sequence, int progress);
	//コマンドの後ろの半透明の板。longestは一番長い方向列の数
	static void DrawCommandBack(int lineCount, size_t longest);
	//かかっている強化の段階と残り秒数
	void DrawBuffs() const;

	//ラベル付きのバーを描く。ratioは0から1
	static void DrawBar(int x, int y, int width, float ratio, int color, const char* label);
	//valueがmaxの何割かを0から1で返す。maxが0以下なら0
	static float ToRatio(float value, float max);

private:
	int _screenWidth = 640;

	int _wave = 0;
	int _waveCount = 0;
	float _waveRemaining = 0.0f;

	std::string _banner;
	int _bannerColor = 0xFFFFFF;
	float _bannerTime = -100.0f;//始まった直後に文字が出ないよう、ずっと前にしておく

	float _playerHp = 0.0f;
	float _playerMaxHp = 0.0f;

	float _heat = 0.0f;
	float _maxHeat = 0.0f;
	bool _isOverHeat = false;

	float _bossHp = 0.0f;
	float _bossMaxHp = 0.0f;
	bool _isBossVisible = false;

	CommandResult _result = CommandResult::Progress;
	float _resultTime = -100.0f;//始まった直後に文字が出ないよう、ずっと前にしておく
	float _resultShowTime = 2.0f;

	CommandView _command;
	std::array<BuffSlot, static_cast<size_t>(BuffType::Count)> _buffs{};
};
