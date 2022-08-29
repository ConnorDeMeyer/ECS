
#include "EntityRegistry.h"
#include "TypeBinding.h"
#include "TypeViewBase.h"

bool TypeBinding::AssertTypeCombination(const uint32_t* types, size_t size) const
{
	if (m_TypesAmount != size)
		return false;

	bool ContainsAll{ true };
	for (size_t i{}; i < size; ++i)
	{
		if (Contains(types[i]))
			ContainsAll = false;
	}

	if (!ContainsAll)
		return true;

	bool isEqual{ true };
	for (size_t i{}; i < m_TypesAmount; ++i)
	{
		if (types[i] != m_pTypes[i])
			isEqual = false;
	}
	if (!isEqual && ContainsAll)
		return false;

	return true;
}

bool TypeBinding::Compare(const uint32_t* types, size_t size) const
{
	if (size != m_TypesAmount)
		return false;

	assert(AssertTypeCombination(types, size));

	for (size_t i{}; i < m_TypesAmount; ++i)
	{
		if (types[i] != m_pTypes[i])
			return false;
	}
	return true;
}

bool TypeBinding::Contains(uint32_t typeId) const
{
	for (size_t i{}; i < m_TypesAmount; ++i)
	{
		if (typeId == m_pTypes[i])
			return true;
	}
	return false;
}

void TypeBinding::Initialize()
{
	std::unique_ptr<TypeViewBase* []> typeViews = std::make_unique<TypeViewBase* []>(m_TypesAmount);
	for (size_t i{}; i < m_TypesAmount; ++i)
	{
		typeViews[i] = m_pRegistry->GetOrCreateView(m_pTypes[i]);
	}

	// Get all the entities of the first view
	auto& entities = typeViews[0]->GetRegisteredEntities();

	for (auto entity : entities)
	{
		bool presentInAll{ true };

		for (size_t i{ 1 }; i < m_TypesAmount; ++i)
		{
			if (!typeViews[i]->Contains(entity))
			{
				presentInAll = false;
				break;
			}
		}

		if (presentInAll)
		{
			push_back();
			m_ContainedEntities.emplace(entity, back());

			for (size_t i{ }; i < m_TypesAmount; ++i)
			{
				m_Data[m_Data.size() - m_TypesAmount + i] = typeViews[i]->GetVoidReference(entity);
			}
		}
	}
	
	auto OnElementAddFunction = [this](TypeViewBase*, entityId id)
	{
		bool presentInAll{ };

		for (size_t i{}; i < GetTypeAmount(); ++i)
		{
			const auto typeId = m_pTypes[i];
			if (!m_pRegistry->GetTypeView(typeId)->Contains(id))
			{
				return;
			}

			presentInAll = true;
		}

		if (presentInAll)
		{
			push_back();
			m_ContainedEntities.emplace(id, back());

			for (size_t i{ }; i < m_TypesAmount; ++i)
			{
				m_Data[m_Data.size() - m_TypesAmount + i] = m_pRegistry->GetTypeView(m_pTypes[i])->GetVoidReference(id);
			}
		}
	};

	auto OnElementRemoveFunction = [this](TypeViewBase*, entityId id)
	{
		auto it = m_ContainedEntities.find(id);
		if (it != m_ContainedEntities.end())
		{
			SwapRemove(it->second);
			m_ContainedEntities.erase(it);
		}
	};


	for (size_t i{}; i < m_TypesAmount; ++i)
	{
		typeViews[i]->OnElementAdd.emplace_back(OnElementAddFunction);
		typeViews[i]->OnElementRemove.emplace_back(OnElementRemoveFunction);
	}
}

void TypeBinding::push_back()
{
	for (size_t i{}; i < m_TypesAmount; ++i)
		m_Data.emplace_back();
}

void TypeBinding::pop_back()
{
	for (size_t i{}; i < m_TypesAmount; ++i)
		m_Data.pop_back();
}

void TypeBinding::move(size_t source, size_t target)
{
	for (size_t i{}; i < m_TypesAmount; ++i)
		m_Data[target + i] = m_Data[source + i];
}

size_t TypeBinding::back()
{
	return (m_Data.size() - m_TypesAmount) / m_TypesAmount;
}

void TypeBinding::SwapRemove(size_t pos)
{
	if (pos == (m_Data.size() - m_TypesAmount) / m_TypesAmount)
	{
		pop_back();
	}
	else
	{
		// use swap remove to remove the binding
		// we need will also need to update the position in the m_ContainedEntities map
		const auto elementAddress = m_Data[m_Data.size() - m_TypesAmount].GetReferencePointer<void>().m_ptr;

		const uint32_t typeId = m_pTypes[0];
		const entityId otherId = m_pRegistry->GetTypeView(typeId)->GetEntityId(elementAddress);

		// update the position of the swapped element
		assert(m_ContainedEntities.contains(otherId));
		m_ContainedEntities[otherId] = pos;

		// swap remove
		move(back(), pos);
		pop_back();
	}
}
