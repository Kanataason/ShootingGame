#include "GameResult.h"
#include "InputManager.h"
#include "SceneManager.h"
#include "EffectUtillity.h"
#include "DrawManager.h"
#include "SoundManager.h"

GameResult::~GameResult()
{
	//シーンが消されたとき、Fontを解放
	if (_titleFont != -1) DeleteFontToHandle(_titleFont);
	if (_messageFont != -1) DeleteFontToHandle(_messageFont);
}

void GameResult::Initialize(ServiceLocator& locator)
{
	//ゲーム中やボス戦のBGMが結果画面まで流れ続けないように止める
	locator.soundManager->StopBgm();

	_elapsed = 0.0f;
	BaseScene::Initialize(locator);
	//SetFontSizeを毎フレーム呼ぶと重いので、大きさごとのフォントを1回だけ作る
	_titleFont = CreateFontToHandle(nullptr, 50, 3);
	_messageFont = CreateFontToHandle(nullptr, 20, 3);
}
void GameResult::Update(float deltaTime)
{
	BaseScene::Update(deltaTime);

	//点滅のための経過時間
	_elapsed += deltaTime;
	//描画はUIレイヤーに登録するだけ。実際に描くのはフレームの最後
	_locator.drawManager->Register(DrawLayer::UI, [this]
		{
			//クリアとゲームオーバーで違うのは、この文字だけ
			DrawStringToHandle(190, 150, _message.c_str(), 0xFFFFFF, _titleFont);
			//案内の文字だけ点滅させて、押せば戻れることに気づきやすくする
			if (EffectUtillity::IsBlinkVisible(_elapsed, _interval))
				DrawStringToHandle(205, 300, "PRESS SPACE TO TITLE", 0xFFFFFF, _messageFont);
		});

	//切り替えは予約だけ。実際の切り替えは次のフレームの最初に行われる
	if (_locator.input->GetKeyDown(VK_SPACE))
		_locator.sceneManager->RequestChange(SceneName::EGameTitle);
}