#include "EntityRegistry.h"

#include "../TypeInformation/TypeInformation.h"
#include "../TypeInformation/ECSTypeInformation.h"

bool TypeBindingIdentifier::Compare(const uint32_t* types, const size_t size)
{
	if (size != m_TypesAmount)
		return false;

	for (size_t i{}; i < size; ++i)
	{
		if (types[i] != m_TypesHashes[i])
			return false;
	}

	assert(Assert(types, size) && "Please use the correct sequence of Types used when creating the binding for optimization purposes");
	return true;
}

bool TypeBindingIdentifier::Contains(uint32_t id)
{
	for (uint32_t i{}; i < m_TypesAmount; ++i)
	{
		if (m_TypesHashes[i] == id)
			return true;
	}
	return false;
}

bool TypeBindingIdentifier::Assert(const uint32_t* types, const size_t size)
{
	for (size_t i{}; i < size; ++i)
	{
		if (Contains(types[i]) && types[i] != m_TypesHashes[i])
			return false;
	}
	return true;
}

EntityRegistry::~EntityRegistry()
{
	for (size_t i{}; i < m_SortingProgress.size(); ++i)
	{
		m_SortingProgress[i] = SortingProgress::canceled;
	}
	m_SortingThreadPool.QuitAndWait();
}

void EntityRegistry::RemoveSystem(std::string name)
{
	auto it = std::find_if(m_Systems.begin(), m_Systems.end(), [name](const std::unique_ptr<SystemBase>& sys)
		{
			return sys->GetName() == name;
		});

	if (it != m_Systems.end())
	{
		m_Systems.erase(it);
	}
}

Entity EntityRegistry::CreateEntity()
{
	entityId newId = !m_Entities.empty() ? *--m_Entities.end() + 1 : 0;
	m_Entities.insert(newId);
	return { *this, newId };
}

void EntityRegistry::RemoveEntity(const Entity& entity)
{
	RemoveEntity(entity.GetId());
}

void EntityRegistry::RemoveEntity(entityId id)
{
	if (id != Entity::InvalidId)
		m_RemovedEntities.emplace_back(id);
}

const Entity EntityRegistry::CreateOrGetEntity(entityId id)
{
	assert(id != Entity::InvalidId);
	auto it = m_Entities.find(id);
	if (it == m_Entities.end())
	{
		m_Entities.emplace(id);
	}
	return Entity(*this, id);
}

void EntityRegistry::RegisterBinding(const TypeBindingIdentifier& identifier)
{
	auto binding = identifier.GetTypeBinding();

	int32_t firstTypeId = identifier.GetTypeIds()[0];
	auto& view = m_TypeViews[firstTypeId];
	auto& entities = view->GetRegisteredEntities();

	for (auto entity : entities)
	{
		bool presentInAll{ true };

		for (size_t i{ 1 }; i < identifier.GetTypesAmount(); ++i)
		{
			auto typeId = identifier.GetTypeIds()[i];
			if (!m_TypeViews[typeId]->Contains(entity))
			{
				presentInAll = false;
				break;
			}
		}

		if (presentInAll)
		{
			size_t offset{};
			binding->AddEmptyData(offset);
			
			binding->RegisterEntity(entity, offset);

			for (size_t i{ }; i < identifier.GetTypesAmount(); ++i)
			{
				auto typeId = identifier.GetTypeIds()[i];
				auto& viewI = m_TypeViews[typeId];
				auto voidReference = viewI->GetVoidReference(entity);
				binding->SetVoidReference(voidReference, offset, i);
			}
		}
	}

	TypeBindingIdentifierReference IdentifierRef{ identifier };

	for (size_t i{ }; i < identifier.GetTypesAmount(); ++i)
	{
		uint32_t typeId{ identifier.GetTypeIds()[i] };
		auto& typeViews{ m_TypeViews };
		auto& viewI = m_TypeViews[typeId];

		auto OnElementAddFunction = [IdentifierRef, &typeViews](TypeViewBase*, entityId id)
		{
			bool presentInAll{};

			for (size_t i{ }; i < IdentifierRef.TypesAmount; ++i)
			{
				auto typeId = IdentifierRef.TypeIds[i];
				if (!typeViews[typeId]->Contains(id))
				{
					return;
				}

				presentInAll = true;
			}

			if (presentInAll)
			{
				size_t offset{};
				IdentifierRef.TypeBinding->AddEmptyData(offset);
				IdentifierRef.TypeBinding->RegisterEntity(id, offset);

				for (size_t i{ }; i < IdentifierRef.TypesAmount; ++i)
				{
					auto typeId = IdentifierRef.TypeIds[i];
					auto& viewI = typeViews[typeId];
					auto voidReference = viewI->GetVoidReference(id);
					IdentifierRef.TypeBinding->SetVoidReference(voidReference, offset, i);
				}
			}
		};

		auto OnElementRemoveFunction = [IdentifierRef, &typeViews](TypeViewBase*, entityId id)
		{
			size_t offset{};
			if (IdentifierRef.TypeBinding->Contains(id, offset))
			{
				IdentifierRef.TypeBinding->SwapRemove(offset, typeViews);

				IdentifierRef.TypeBinding->RemoveId(id);
			}
		};

		viewI->OnElementAdd.emplace_back(OnElementAddFunction);
		viewI->OnElementRemove.emplace_back(OnElementRemoveFunction);
	}
}

[[nodiscard]] TypeBindingBase* EntityRegistry::GetBinding(const uint32_t* types, const size_t size)
{
	for (auto& binding : m_TypeBindings)
		if (binding.Compare(types, size))
			return binding.GetTypeBinding();

	throw std::runtime_error("No Binding inside of this type inside of registry");
}

void EntityRegistry::Update()
{
	for (auto& typeView : m_TypeViews)
	{
		if (typeView.second->GetDataFlag() == ViewDataFlag::dirty)
		{
			for (size_t i{}; i < ThreadPoolSize; ++i)
			{
				auto& progress = m_SortingProgress[i];
				if (progress == SortingProgress::none)
				{
					auto& quitBool{ m_SortingThreadPool.GetQuitBool() };
					m_SortingThreadPool.AddFunction([&typeView, &progress, &quitBool] {typeView.second->SortData(progress, quitBool); });
					break;
				}
			}
		}
	}

	for (size_t i{}; i < ThreadPoolSize; ++i)
	{
		auto& progress = m_SortingProgress[i];
		switch (progress)
		{
		case SortingProgress::done:
			{
			progress = SortingProgress::copying;
				while (progress != SortingProgress::none)
				{}
			}
			break;
		case SortingProgress::canceled:
			progress = SortingProgress::none;
			break;
		}
	}

	for (auto& system : m_Systems)
	{
		system->Execute();
	}

	for (auto id : m_RemovedEntities)
	{
		m_Entities.erase(id);
		for (auto& view : m_TypeViews)
		{
			if (view.second->Contains(id))
			{
				view.second->Remove(id);
			}
		}
	}
	m_RemovedEntities.clear();

	for (auto& idComp : m_RemovedComponents)
	{
		auto it = m_TypeViews.find(idComp.first);
		if (it != m_TypeViews.end())
		{
			it->second->Remove(idComp.second);
		}
	}
	m_RemovedComponents.clear();

	for (auto& typeView : m_TypeViews)
	{
		typeView.second->Update();
	}
		
}

void EntityRegistry::Serialize(std::ofstream& stream)
{
	stream << m_Entities.size();
	for (auto& entity : m_Entities)
		stream << entity;

	for (auto& typeView : m_TypeViews)
	{
		stream << typeView.first;
		typeView.second->SerializeView(stream);
	}
}

void EntityRegistry::Deserialize(std::ifstream& stream)
{
	size_t entityAmount{};
	stream >> entityAmount;
	for (size_t i{}; i < entityAmount; ++i)
	{
		entityId id{};
		stream >> id;
		m_Entities.emplace(id);
	}

	while (stream)
	{
		uint32_t id;
		stream >> id;
		if (!m_TypeViews.contains(id))
			TypeInformation::GetTypeInfo(id).m_ViewAdder(this);
		m_TypeViews[id]->DeserializeView(stream);
	}
}

