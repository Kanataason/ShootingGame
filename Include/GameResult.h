#pragma once
#include <string>
#include "BaseScene.h"
#include "ServiceLocator.h"

//ゲームクリア・ゲームオーバー画面。表示する文字だけが違う
//スペースキーでタイトルへ戻る
class GameResult :public BaseScene
{
public:
	explicit GameResult(std::string message) : _message(std::move(message)) {}
	~GameResult() override;

	void Initialize(ServiceLocator& locator)override;
	void Update(float deltaTime)override;
private:
	std::string _message;
	float _elapsed = 0.0f;
	const float _interval = 0.5f;

	//SetFontSizeを毎フレーム呼ぶとフォントが作り直されて重いので、ハンドルを1回だけ作る
	int _titleFont = -1;
	int _messageFont = -1;
};