#include "GameObject.h"

GameObject::GameObject(const GameObject& source)
	: _transform(source._transform)
{
	//コンポーネントはunique_ptrなのでそのままはコピーできない。Cloneで1つずつ複製する
	_components.reserve(source._components.size());
	for (const auto& sourceComponent : source._components)
	{
		AttachComponent(sourceComponent->Clone());
	}
}

void GameObject::AttachComponent(std::unique_ptr<Component> component)
{
	//コンポーネントから持ち主のGetComponentなどを使えるように、先に持ち主を教える
	component->SetOwner(this);
	_components.push_back(std::move(component));
}

void GameObject::Initialize(ServiceLocator& locator)
{
	for (auto& com : _components)
		com->Initialize(locator);
}
void GameObject::Update(float deltaTime)
{
	for (auto& com : _components)
		com->Update(deltaTime);
}
void GameObject::Draw()
{
	for (auto& com : _components)
		com->Draw();
}

void GameObject::OnColliderEnter(float damage)
{
	for (auto& com : _components)
		com->OnColliderEnter(damage);
}
void GameObject::OnPickup(GameObject& other)
{
	for (auto& com : _components)
		com->OnPickup(other);
}