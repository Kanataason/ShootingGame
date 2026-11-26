#include <fstream>
#include <DxLib.h>

#include "JsonLoader.h"
#include "GameTitle.h"
#include "GameScene.h"
#include "GameResult.h"

//Jsonで読み込んだ値を変換するテーブルスペース
namespace
{
	//JSONはUTF-8なので、日本語のファイル名はDxLibが読めるShift-JISに直す
	std::string ToShiftJis(const std::string& utf8)
	{
		const int wideLength = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, nullptr, 0);
		if (wideLength <= 0) return utf8;
		std::wstring wide(wideLength, L'\0');
		MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, wide.data(), wideLength);

		const int length = WideCharToMultiByte(CP_ACP, 0, wide.c_str(), -1, nullptr, 0, nullptr, nullptr);
		if (length <= 0) return utf8;
		std::string result(length, '\0');
		WideCharToMultiByte(CP_ACP, 0, wide.c_str(), -1, result.data(), length, nullptr, nullptr);
		result.pop_back();//最後の'\0'を消す
		return result;
	}

	//ScenePathは後で設定するのでここではempty
	const std::unordered_map<std::string, SceneData> sceneTable =
	{
		{ "Title",{SceneName::EGameTitle,"", [] { return std::make_unique<Title>(); }}},
		{ "GameScene", { SceneName::EGameScene, "",[] { return std::make_unique<GameScene>();}}},
		{"Result",{SceneName::EGameOver,"",[] {return std::make_unique<GameResult>("GAME OVER");}}},
		{"Clear",{SceneName::EGameClear,"",[] {return std::make_unique<GameResult>("GAME CLEAR");}}}
	};
	const std::unordered_map<std::string, ObjectType> objectTypeTable =
	{
		{"Player",ObjectType::Player},
		{"Enemy",ObjectType::Enemy},
		{"Item",ObjectType::Item},
		{"Boss",ObjectType::Boss}
	};
	const std::unordered_map<std::string, CharacterType> characterTypeTable =
	{
		{"Player",CharacterType::Player},
		{"MoveEnemy",CharacterType::MoveEnemy},
		{"ReflectShotEnemy",CharacterType::ReflectShotEnemy},
		{"SpreadEnemy",CharacterType::SpreadEnemy},
		{"Boss",CharacterType::Boss},
		{"Item",CharacterType::Item}
	};

	const std::unordered_map<CharacterType ,std::function<SizeData(const json& root)>> readSizeTable =
	{
		{CharacterType::Player,JsonLoader::LoadRadius},
		{CharacterType::ReflectShotEnemy,JsonLoader::LoadBoxMinAndMax},
		{CharacterType::MoveEnemy,JsonLoader::LoadRadius},
		{CharacterType::SpreadEnemy,JsonLoader::LoadBoxMinAndMax},
		{CharacterType::Boss,JsonLoader::LoadBoxMinAndMax},
		{CharacterType::Item,JsonLoader::LoadRadius}
	};
	const std::unordered_map<std::string, MovePattern> movePatternTable =
	{
		{ "EnterAndStay", MovePattern::EnterAndStay },
		{ "PassThrough",  MovePattern::PassThrough  },
	};
	//音のデータ
	const  std::unordered_map<std::string, SoundId> soundTable =
	{
		{"PlayerShot",SoundId::PlayerShot},
		{"PlayerShotGun",SoundId::PlayerShotGun},
		{"PlayerHit",SoundId::PlayerHit},
		{"CommandAttack",SoundId::CommandAttack},
		{"CommandDefense",SoundId::CommandDefense},
		{"CommandSpeed",SoundId::CommandSpeed},
		{"CommandPredict",SoundId::CommandPredict},
		{"CommandHeal",SoundId::CommandHeal},
		{"CharacterExplosion",SoundId::CharacterExplosion},
		{"BossExplosion",SoundId::BossExplosion},
		{"BgmBoss",SoundId::BgmBoss},
		{"BgmGame",SoundId::BgmGame},
		{"BgmClear",SoundId::BgmGameClear}
	};
}


json JsonLoader::LoadJsonData(const std::string& assetPath)const
{
	std::ifstream file(assetPath);
	//ファイルが無いまま進むと、後で原因の分かりにくい落ち方をするので、ここで止める
	if (!file.is_open())
	{
		throw std::runtime_error("Failed to open json file.");
	}
	json j;
	file >> j;
	return j;
}

SceneData JsonLoader::LoadScenes(const json& root)const
{
	//名前とファイルのどちらかが無いとシーンを作れないので、どちらも必須にする
	if (!root.contains("SceneName"))
		throw std::runtime_error("Not find SceneName.");

	//SceneDataに変換してから返す
	SceneData data = ToSceneData(root.at("SceneName").get<std::string>());

	if(!root.contains("SceneDataPath"))
		throw std::runtime_error("Not find ScenePath");

	data.path = root.at("SceneDataPath").get<std::string>();

	return data;
}


ObjectData  JsonLoader::LoadObjectData(const json& root)const
{
	ObjectData data{};

	//Nameで、どの組み立て処理を使うかが決まるので必須にする
	if (!root.contains("Name"))
		throw std::runtime_error("Not find Name");

	data.name = ToObjectType(root.at("Name").get<std::string>());

	data.transform = LoadTransform(root.at("Transform"));

	data.statusData = LoadStatusData(root.at("Status"));

	//攻撃しないもの（アイテムなど）は書かなくてよい
	if (root.contains("AttackType"))
		LoadAttackType(root, data);

	//当たり判定の形（円か四角か）はキャラクターの種類で決まるので、種類ごとの読み方を表で引く
	auto it = readSizeTable.find(data.statusData.characterType);
	if (it == readSizeTable.end())
	{
		return data;   //サイズ情報を持たないオブジェクト
	}

	data.sizeData = it->second(root);

	return data;
}

std::vector<SpawnData> JsonLoader::LoadSpawnData(const std::string& path) const
{
	const json root = LoadJsonData(path);

	std::vector<SpawnData> result;
	for (const auto& templateRoot : root.at("Templates"))
	{
		//見た目や強さはシーンに置くオブジェクトと同じ形で読み、出し方の設定だけを足す
		SpawnData data;
		data.objectData = LoadObjectData(templateRoot);
		//出し方の設定は書かなくてもよいように、既定値を入れる
		data.isRandomX = templateRoot.value("RandomX", true);
		data.maxCount = templateRoot.value("MaxCount", -1);
		data.firstWave = templateRoot.value("FirstWave", 1);
		result.push_back(std::move(data));
	}
	return result;
}

std::vector<WaveData> JsonLoader::LoadWaveData(const std::string& path) const
{
	const json root = LoadJsonData(path);

	std::vector<WaveData> result;
	if (root.contains("Waves"))
	{
		for (const auto& waveRoot : root.at("Waves"))
		{
			//書いていない項目はWaveDataの既定値を使う。アイテムのウェーブはDurationを書かない
			WaveData data;
			data.duration = waveRoot.value("Duration", data.duration);
			data.minInterval = waveRoot.value("MinInterval", data.minInterval);
			data.maxInterval = waveRoot.value("MaxInterval", data.maxInterval);
			result.push_back(data);
		}
	}
	//ウェーブが無いと進まないので1つ入れる
	if (result.empty()) result.push_back(WaveData{});
	return result;
}

ItemDropData JsonLoader::LoadItemDropData(const std::string& path) const
{
	const json root = LoadJsonData(path);

	//書いていない項目はItemDropDataの既定値を使う
	//Dropごと無くても、既定値のままアイテムは落ちるようにする
	ItemDropData data;
	if (!root.contains("Drop")) return data;

	const json& dropRoot = root.at("Drop");
	//雑魚を倒したときの落とし方。確率は0〜1で書く
	data.dropChance = dropRoot.value("Chance", data.dropChance);
	data.pityCount = dropRoot.value("PityCount", data.pityCount);
	//HPがこの割合以下なら、種類を回復に決める
	data.pinchHpRatio = dropRoot.value("PinchHpRatio", data.pinchHpRatio);
	//ボス戦で上から降らせる間隔。雑魚のウェーブと同じWaveDataに入れて、スポナーにそのまま渡せるようにする
	data.bossFall.minInterval = dropRoot.value("BossMinInterval", data.bossFall.minInterval);
	data.bossFall.maxInterval = dropRoot.value("BossMaxInterval", data.bossFall.maxInterval);
	return data;
}

DifficultyTable JsonLoader::LoadDifficultyTable(const std::string& path) const
{
	const json root = LoadJsonData(path);

	DifficultyTable table;
	for (const auto& difficultyRoot : root.at("Difficulties"))
	{
		//書いていない倍率はDifficultyDataの既定値（1）のままにして、Normalは名前だけで済むようにする
		DifficultyData data;
		data.name = difficultyRoot.value("Name", data.name);
		data.damageTakenRate = difficultyRoot.value("DamageTakenRate", data.damageTakenRate);
		data.spawnIntervalRate = difficultyRoot.value("SpawnIntervalRate", data.spawnIntervalRate);
		data.dropChanceRate = difficultyRoot.value("DropChanceRate", data.dropChanceRate);
		data.pityCountRate = difficultyRoot.value("PityCountRate", data.pityCountRate);
		data.commandTimeRate = difficultyRoot.value("CommandTimeRate", data.commandTimeRate);
		data.warnTimeRate = difficultyRoot.value("WarnTimeRate", data.warnTimeRate);
		data.bossHealthRate = difficultyRoot.value("BossHealthRate", data.bossHealthRate);
		table.difficulties.push_back(std::move(data));
	}
	//難易度が1つも無いと選べないので、ここで止める
	if (table.difficulties.empty())
		throw std::runtime_error("Difficulty.json has no Difficulties.");

	//範囲外の番号が書かれていたら、先頭にする
	table.defaultIndex = root.value("Default", static_cast<size_t>(0));
	if (table.defaultIndex >= table.difficulties.size()) table.defaultIndex = 0;
	return table;
}

ObjectData JsonLoader::LoadBossData(const std::string& path) const
{
	const json root = LoadJsonData(path);
	return LoadObjectData(root.at("Boss").at(0));
}

void JsonLoader::LoadAttackType(const json& root, ObjectData& data)const
{
	for (const auto& attackTypeId : root.at("AttackType"))
	{
		data.attackType.push_back(attackTypeId.get<int>());
	}
}

Transform JsonLoader::LoadTransform(const json& root)const
{
	Transform transform{};

	//Position [x, y, z] を読み込む
	if (root.contains("Position") &&
		root.at("Position").is_array() &&
		root.at("Position").size() >= 3)
	{
		transform.Position.x = root.at("Position")[0].get<float>();
		transform.Position.y = root.at("Position")[1].get<float>();
	}

	return transform;
}
std::vector<SoundData> JsonLoader::LoadSoundData(const json& root)const
{
	std::vector<SoundData> dataList{};

	for(const auto& soundRoot : root.at("Sounds"))
	{
		SoundData data{};
		//Nameで、コードから鳴らすときのSoundIdが決まるので必須にする
		if (!soundRoot.contains("Name"))
			throw std::runtime_error(
				"Not Find Name: "
			);

		data.id = ToSoundId(soundRoot.at("Name").get<std::string>());

		//ファイル名に日本語があるので、DxLibが読める文字コードに直してから持つ
		if (soundRoot.contains("Path"))
			data.path = ToShiftJis(soundRoot.at("Path").get<std::string>());

		//音量は0〜255。書いていなければ最大
		data.volume = soundRoot.value("Volume", 255);

		//同じ音を重ねて鳴らせる数。連射の音などは増やす
		data.bufferNum = soundRoot.value("BufferNum", 1);

		dataList.push_back(data);
	}

	return dataList;
}

SceneData JsonLoader::ToSceneData(const std::string& name)const
{
	const auto& it = sceneTable.find(name);
	if (it != sceneTable.end())
	{
		return it->second;
	}
	
	throw std::runtime_error(
		"Unknown SceneData: " + name
	);
}

MovePattern JsonLoader::ToMovePattern(const std::string& name) const
{
	auto it = movePatternTable.find(name);
	if (it != movePatternTable.end())
		return it->second;
	throw std::runtime_error("Unknown MovePattern: " + name);
}
CharacterType JsonLoader::ToCharacterType(const std::string& name)const
{
	const auto& it = characterTypeTable.find(name);
	if (it != characterTypeTable.end())
	{
		return it->second;
	}

	throw std::runtime_error(
		"Unknown Character: " + name
	);
}

ObjectType JsonLoader::ToObjectType(const std::string& name)const
{
	const auto& it = objectTypeTable.find(name);
	if (it != objectTypeTable.end())
	{
		return it->second;
	}

	throw std::runtime_error(
		"Unknown ObjectType: " + name
	);
}
SoundId JsonLoader::ToSoundId(const std::string& name)const
{
	const auto& it = soundTable.find(name);
	if (it != soundTable.end())
	{
		return it->second;
	}
	throw std::runtime_error(
		"Unknown SoundId: " + name
	);
}

SizeData JsonLoader::LoadRadius(const json& root)
{
	SphereData data{};

	//半径が無いと当たり判定が作れないので必須にする
	if(!root.contains("Radius"))
		throw std::runtime_error("Not find Radius");
		
	data.radius = root.at("Radius").get<float>();

	return data;
}
StatusData JsonLoader::LoadStatusData(const json& root)const
{
	StatusData data{};

	//どれも書かなくてよい。書いていないものはStatusDataの既定値のまま
	//キャラクターによって使う項目が違うため（アイテムはCharacterTypeだけ）
	if (root.contains("CharacterType"))
		data.characterType = ToCharacterType(root.at("CharacterType").get<std::string>());

	if (root.contains("Health"))
		data.health = root.at("Health").get<float>();

	//JSONでは0か1で書く
	if (root.contains("IsMove"))
		data.isMove = root.at("IsMove").get<int>();

	//画面にとどまるか、通り抜けて消えるか
	if (root.contains("MovePattern"))
		data.movePattern = ToMovePattern(root.at("MovePattern").get<std::string>());

	//画像の読み込みはGameObjectFactoryで行うので、ここではパスだけ持つ
	if(root.contains("ImagePath"))
		data.imagePath = root.at("ImagePath").get<std::string>();

	return data;
}

SizeData JsonLoader::LoadBoxMinAndMax(const json& root)
{
	BoxData data{};

	//[幅の半分, 高さの半分] の形で書く。形が違うときは大きさ0のまま
	if (root.contains("HalfSize") &&
		root.at("HalfSize").is_array() &&
		root.at("HalfSize").size() >= 2)
	{
		data.halfSize.x = root.at("HalfSize")[0].get<float>();
		data.halfSize.y = root.at("HalfSize")[1].get<float>();
	}

	return data;
}