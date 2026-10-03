#pragma once

class GameObject;

//撃破数とボスを持つ。GameSceneが所有する
class GameProgress
{
public:
	void AddKill() { ++_killCount; }
	int GetKillCount() const { return _killCount; }

private:
	int _killCount = 0;
};