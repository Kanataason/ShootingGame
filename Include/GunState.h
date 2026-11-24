#pragma once

//撃つ間隔と、武器を切り替えられるまでの時間を数える
//プレイヤーと敵の攻撃ステートが継承する
class GunState
{
public:

	void SetShotInfo(bool isShot, float coolDown) { _isShotFlag = isShot;_coolDown = coolDown; }
	//初期値設定関数
	void SetGunState(float coolDown, bool shotFlag = false, bool changeFlag = false);

	//動いてないときでもカウントは計算されるから範囲内に収めるための処理
	void Clamp();
	//カウントダウン用の関数
	void ElapsedTimer(float deltaTime);
protected:
	float _maxChangeTime = 1.0f;
	float _maxCoolDown = 0.2f;

	bool _isChangeFlag = false;
	bool _isShotFlag = false;
	float _coolDown = 0.0f;

	//武器切り替えクールタイム
	float _changeCoolDown = 0.0f;
};
