#include "SceneLoader.h"
#include "JsonLoader.h" 
#include "BaseScene.h"

void SceneLoader::LoadObjects(BaseScene* scene,
	const std::string& path, JsonLoader* loader, GameObjectFactory& factory)
{
	const auto& sceneData = loader->LoadJsonData(path);
	if (!sceneData.contains("objects"))return;

	for (auto& roots : sceneData.at("objects"))
	{
		//Jsonから読み取ったオブジェクトをシーンに移す
		auto sData = loader->LoadSceneData(roots);
		scene->AddObject(std::move(factory.CreateObject(sData)));
	}
}