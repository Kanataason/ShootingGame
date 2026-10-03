#pragma once
#include <string>


#include "GameObjectfactory.h"

class BaseScene;
class JsonLoader;

class SceneLoader
{
public:
	//Jsonに書かれているオブジェクトを読み取ってシーンに追加
	void LoadObjects(BaseScene* scene,const std::string& path,
		JsonLoader* loader, GameObjectFactory& factory);
};