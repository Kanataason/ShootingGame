#include "GameTitle.h"
#include "GameObject.h"
#include "Transform.h"

#include "InputManager.h"
#include "SceneManager.h"
#include "EffectUtillity.h"
#include "DrawManager.h"
#include "GameSettings.h"

namespace
{
	//難易度の一覧の位置。タイトルと「PRESS SPACE」の間に縦に並べる
	constexpr int DifficultyX = 270;
	constexpr int DifficultyTopY = 225;
	constexpr int DifficultyLineHeight = 30;
	constexpr int CursorOffsetX = 25;   //選択中の印を名前の左にずらす量
	constexpr int SelectedColor = 0xFFFF00;
	constexpr int UnselectedColor = 0x888888;
	constexpr int TextColor = 0xFFFFFF;
}

Title::~Title()
{
	//シーン削除時フォント解放
	if (_titleFont != -1) DeleteFontToHandle(_titleFont);
	if (_messageFont != -1) DeleteFontToHandle(_messageFont);
}

void Title::Initialize(ServiceLocator& locator)
{
	_elapsed = 0.0f;
	BaseScene::Initialize(locator);
	_titleFont = CreateFontToHandle(nullptr, 50, 3);
	_messageFont = CreateFontToHandle(nullptr, 20, 3);
}
void Title::Update(float deltaTime)
{
	BaseScene::Update(deltaTime);

	//点滅のための経過時間
	_elapsed += deltaTime;
	//描画はUIレイヤーに登録するだけ。実際に描くのはフレームの最後
	_locator.drawManager->Register(DrawLayer::UI, [this]
		{
			DrawStringToHandle(150, 150, "SHOOTING GAME", TextColor, _titleFont);
			DrawDifficulties();
			//案内の文字だけ点滅させて、押せば進むことに気づきやすくする
			if (EffectUtillity::IsBlinkVisible(_elapsed, _interval))
				DrawStringToHandle(200, 340, "PRESS SPACE TO START", TextColor, _messageFont);
			DrawStringToHandle(215, 380, "↑↓ : 難易度を選ぶ", UnselectedColor, _messageFont);
		});

	//難易度は矢印キーでもWSでも選べるようにする。ゲーム中の移動と同じキーでも迷わないため
	GameSettings* settings = _locator.gameSettings;
	if (_locator.input->GetKeyDown(VK_UP) || _locator.input->GetKeyDown('W'))
		settings->SelectPrevious();
	if (_locator.input->GetKeyDown(VK_DOWN) || _locator.input->GetKeyDown('S'))
		settings->SelectNext();

	//切り替えは予約だけ。実際の切り替えは次のフレームの最初に行われる
	if (_locator.input->GetKeyDown(VK_SPACE))
		_locator.sceneManager->RequestChange(SceneName::EGameScene);
}

void Title::DrawDifficulties() const
{
	const GameSettings* settings = _locator.gameSettings;
	const auto& difficulties = settings->GetDifficulties();
	for (size_t index = 0; index < difficulties.size(); ++index)
	{
		//選んでいるものだけ黄色にして、左に印を付ける
		const bool isSelected = (index == settings->GetSelectedIndex());
		const int y = DifficultyTopY + static_cast<int>(index) * DifficultyLineHeight;
		const int color = isSelected ? SelectedColor : UnselectedColor;
		if (isSelected)
			DrawStringToHandle(DifficultyX - CursorOffsetX, y, ">", color, _messageFont);
		DrawStringToHandle(DifficultyX, y, difficulties[index].name.c_str(), color, _messageFont);
	}
}
