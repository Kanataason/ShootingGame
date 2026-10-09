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
	locator.soundManager->StopBgm();

	_elapsed = 0.0f;
	BaseScene::Initialize(locator);
	_titleFont = CreateFontToHandle(nullptr, 50, 3);
	_messageFont = CreateFontToHandle(nullptr, 20, 3);
}
void GameResult::Update(float deltaTime)
{
	BaseScene::Update(deltaTime);

	_elapsed += deltaTime;
	_locator.drawManager->Register(DrawLayer::UI, [this]
		{
			DrawStringToHandle(190, 150, _message.c_str(), 0xFFFFFF, _titleFont);
			if (EffectUtillity::IsBlinkVisible(_elapsed, _interval))
				DrawStringToHandle(205, 300, "PRESS SPACE TO TITLE", 0xFFFFFF, _messageFont);
		});

	if (_locator.input->GetKeyDown(VK_SPACE))
		_locator.sceneManager->RequestChange(SceneName::EGameTitle);
}