#pragma once

class GameObject;

//敵などがプレイヤーを参照するための窓口
//プレイヤーは死亡しても削除しないので、シーンが作り直されるまでポインタは有効
class PlayerProvider
{
public:
	//リトライ対策で、生存状態と予測の段階もリセット
	void SetPlayer(GameObject* player) { _player = player; _active = true; _predictLevel = 0; }
	GameObject* GetPlayer() const { return _player; }

	void SetPlayerActive(bool active) { _active = active; }
	bool GetActive()const { return _active; }

	//予測の段階。敵の予告の強さに使う
	//GameSceneが入れる
	void SetPredictLevel(int level) { _predictLevel = level; }
	int GetPredictLevel() const { return _predictLevel; }
private:
	GameObject* _player = nullptr;
	bool _active = true;
	int _predictLevel = 0;
};