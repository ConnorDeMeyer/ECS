#include "EntityRegistry.h"

#include "../GameComponent/GameComponent.h"

TypeBindingIdentifier::~TypeBindingIdentifier()
{
	delete[] m_TypesHashes;
	delete m_TypeBinding;
}

TypeBindingIdentifier::TypeBindingIdentifier(TypeBindingIdentifier&& other) noexcept
	: m_TypeBinding{other.m_TypeBinding}
	, m_TypesAmount{other.m_TypesAmount}
	, m_TypesHashes{other.m_TypesHashes}
{
	other.m_TypesHashes = nullptr;
	other.m_TypesAmount = 0;
	other.m_TypeBinding = nullptr;
}

TypeBindingIdentifier& TypeBindingIdentifier::operator=(TypeBindingIdentifier&& other) noexcept
{
	m_TypeBinding = other.m_TypeBinding;
	m_TypesAmount = other.m_TypesAmount;
	m_TypesHashes = other.m_TypesHashes;

	other.m_TypesHashes = nullptr;
	other.m_TypesAmount = 0;
	other.m_TypeBinding = nullptr;
	return *this;
}

entityId EntityRegistry::CreateEntity()
{
	entityId newId = !m_Entities.empty() ? m_Entities.back() + 1 : 0;
	m_Entities.emplace_back(newId);
	return newId;
}

void EntityRegistry::ForEachGameComponent(const std::function<void(GameComponentClass*)>& function)
{
	for (auto view : m_GameComponentTypeViews)
	{
		for (auto it = view->GetVoidIterator(); it != view->GetVoidIteratorEnd(); ++it)
		{
			function(it.get<GameComponentClass>());
		}
	}
}
