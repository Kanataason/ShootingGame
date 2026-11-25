#include "GameSettings.h"

void GameSettings::Initialize(DifficultyTable table)
{
	_table = std::move(table);
	//JsonLoaderで範囲内に直してあるので、そのまま使う
	_selectedIndex = _table.defaultIndex;
}

void GameSettings::SelectNext()
{
	//一番下で押しても一番上に戻らないようにする。回り込むと、どこにいるか分かりにくいため
	if (_selectedIndex + 1 < _table.difficulties.size()) ++_selectedIndex;
}

void GameSettings::SelectPrevious()
{
	if (_selectedIndex > 0) --_selectedIndex;
}
