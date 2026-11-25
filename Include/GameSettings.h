#pragma once
#include "Datas.h"

//タイトルで選んだ難易度を持つ
//シーンは切り替えのたびに作り直されるので、選んだ難易度はシーンの外（GameManager）に置く
//リトライしても、タイトルで選び直すまで同じ難易度のまま
class GameSettings
{
public:
	void Initialize(DifficultyTable table);

	//一覧の次・前の難易度を選ぶ。端では止まる
	void SelectNext();
	void SelectPrevious();

	//今選ばれている難易度の倍率
	const DifficultyData& GetDifficulty() const { return _table.difficulties[_selectedIndex]; }
	//タイトルで一覧を並べるために使う
	const std::vector<DifficultyData>& GetDifficulties() const { return _table.difficulties; }
	size_t GetSelectedIndex() const { return _selectedIndex; }

private:
	DifficultyTable _table;
	size_t _selectedIndex = 0;
};
