#include "EntityRegistry.h"

#include "../TypeInformation/TypeInformation.h"
#include "../TypeInformation/ECSTypeInformation.h"

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

TypeViewBase* EntityRegistry::AddView(uint32_t typeId)
{
	return TypeInformation::AddTypeView(typeId, this);
}

TypeViewBase* EntityRegistry::GetTypeView(uint32_t typeId) const
{
	return m_TypeViews.find(typeId)->second.get();
}

TypeViewBase* EntityRegistry::GetOrCreateView(uint32_t typeId)
{
	auto it = m_TypeViews.find(typeId);
	if (it != m_TypeViews.end())
		return it->second.get();
	else
		return AddView(typeId);
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

TypeBinding* EntityRegistry::AddBinding(const uint32_t* typeIds, size_t size)
{
	auto binding = new TypeBinding(this, typeIds, size);

	m_TypeBindings.emplace_back(binding);

	return binding;
}

[[nodiscard]] TypeBinding* EntityRegistry::GetBinding(const uint32_t* types, const size_t size) const
{
	for (auto binding : m_TypeBindings)
		if (binding->Compare(types, size))
			return binding;

	throw std::runtime_error("No Binding inside of this type inside of registry");
}

TypeBinding* EntityRegistry::GetOrCreateBinding(const uint32_t* types, const size_t size)
{
	for (auto binding : m_TypeBindings)
		if (binding->Compare(types, size))
			return binding;

	return AddBinding(types, size);
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

