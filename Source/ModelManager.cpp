#include "ModelManager.h"

const GameObject* ModelManager::LoadedObjectData(const CharacterType& Ctype)const
{
	const auto& it = _models.find(Ctype);
	if (it != _models.end())
	{
		return it->second.get();
	}
	return nullptr;
}

void ModelManager::SetObjectData(const GameObject& object, const CharacterType& Ctype)
{
	if (!_models.contains(Ctype))
	{
		_models[Ctype] = std::make_unique<GameObject>(object);
	}
}