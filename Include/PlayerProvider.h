#pragma once

class GameObject;

//敵などがプレイヤーを参照するための窓口
//プレイヤーは死亡しても削除しないので、シーンが作り直されるまでポインタは有効
class PlayerProvider
{
public:
	//リトライ対策として、プレイヤーを設定し直すときに生存状態と予測の段階もリセットする
	void SetPlayer(GameObject* player) { _player = player; _active = true; _predictLevel = 0; }
	GameObject* GetPlayer() const { return _player; }

	void SetPlayerActive(bool active) { _active = active; }
	bool GetActive()const { return _active; }

	//プレイヤーにかかっている予測の強化の段階。敵の攻撃予告の強さに使う
	//敵がプレイヤーの強化を直接見ないように、GameSceneがここへ入れる
	void SetPredictLevel(int level) { _predictLevel = level; }
	int GetPredictLevel() const { return _predictLevel; }
private:
	GameObject* _player = nullptr;
	bool _active = true;
	int _predictLevel = 0;
};