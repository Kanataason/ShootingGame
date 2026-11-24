#include "Collider.h"

Collider::~Collider()
{
    //破棄されたコライダーが判定に使われないよう、レジストリから外す
    if (_registry != nullptr) { _registry->Unregister(this); }
}

void Collider::Initialize(ServiceLocator& locator)
{
    _registry = locator.colliderRegistry;
    _registry->Register(this);
}
