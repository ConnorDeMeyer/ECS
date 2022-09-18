#include "EntityRegistry.h"

#include "../TypeInformation/TypeInformation.h"
#include "../TypeInformation/ECSTypeInformation.h"
#include "../Serialize/Serializer.h"

#include <cassert>
#include <chrono>
#include <unordered_map>

EntityRegistry::~EntityRegistry()
{
	for (size_t i{}; i < m_SortingProgress.size(); ++i)
	{
		m_SortingProgress[i] = SortingProgress::canceled;
	}
	m_SortingThreadPool.QuitAndWait();
}

void EntityRegistry::AddDefaultSystems(uint32_t typeId)
{
	ECSTypeInformation::AddDefaultSystems(typeId, this);
}

bool EntityRegistry::ContainsSystem(const std::string& name) const
{
	for (auto& system : m_Systems)
	{
		if (system->GetSystemParameters().name == name)
			return true;
	}
	return false;
}

SystemBase* EntityRegistry::AddSystem(const std::string& name)
{
	auto& systemAdder = ECSTypeInformation::GetSystemAdders();
	auto it = systemAdder.find(name);
	if (it != systemAdder.end())
	{
		return it->second(this);
	}
	else
	{
		std::cout << "Could not find system: " << name << '\n';
		std::cout << "The registered systems are:\n";
		for (auto& system : systemAdder)
		{
			std::cout << system.first << '\n';
		}
		assert(false); // System cannot be found
	}
	return nullptr;
}

void EntityRegistry::PrintSystems() const
{
	PrintSystems(std::cout);
}

void EntityRegistry::PrintAllSystems(std::ostream& stream) const
{
	for (auto& system : m_Systems)
	{
		stream << system->GetSystemParameters().name << '\n';
	}
}

void EntityRegistry::PrintAllSystems() const
{
	PrintAllSystems(std::cout);
}

void EntityRegistry::PrintSystemInformation() const
{
	PrintSystemInformation(std::cout);
}

void EntityRegistry::PrintSystemInformation(std::ostream& stream) const
{
	for (auto& system : m_Systems)
	{

		stream << "\nName: " << system->GetSystemParameters().name;
		stream << "\nExecution Time: " << system->GetSystemParameters().executionTime;
		stream << "\nUpdate Interval: " << system->GetSystemParameters().updateInterval;

#ifdef SYSTEM_PROFILER
		auto it = m_ProfilerInfo.find(system->GetSystemParameters().name);
		if (it != m_ProfilerInfo.end())
		{
			stream << "\nTimes Executed: " << it->second.timesExecuted;
			stream << "\nTime to Execute System: " << it->second.timeToExecuteSystem;
			//stream << "\nRelative Time to Execute: " << it->second.timeToExecuteSystem
			stream << "\nTime to execute per Component: " << it->second.timeToExecutePerComponent;
		}

#endif
		stream << '\n';

	}
}

void EntityRegistry::PrintSystems(std::ostream& stream) const
{
	for (auto& system : m_Systems)
	{
		if (!system->IsSubSystem() && !system->IsDefaulySystem())
			stream << system->GetSystemParameters().name << '\n';
	}
}

void EntityRegistry::RemoveSystem(std::string name)
{
	auto it = std::find_if(m_Systems.begin(), m_Systems.end(), [&name](const std::unique_ptr<SystemBase>& sys)
		{
			return sys->GetSystemParameters().name == name;
		});

	if (it != m_Systems.end())
	{
		auto typeIds = it->get()->GetTypeIds();

		m_Systems.erase(it);

		// Remove subsystems
		for (uint32_t typeId : typeIds)
			for (auto sysIt{ m_Systems.begin() }; sysIt != m_Systems.end();)
				if (sysIt->get()->IsSubSystem(typeId))
					m_Systems.erase(sysIt++);
				else
					++sysIt;
	}
}

TypeViewBase* EntityRegistry::AddView(uint32_t typeId)
{
	return ECSTypeInformation::AddTypeView(typeId, this);
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
	entityId newId = m_EntityCounter++;
	assert(!m_Entities.contains(newId));
	m_Entities.insert(newId);
	return { *this, newId };
}

void EntityRegistry::RemoveEntity(const Entity& entity)
{
	RemoveEntity(entity.GetId());
}

void EntityRegistry::RemoveEntity(entityId id)
{
	if (id != Entity::InvalidId && m_Entities.contains(id))
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
	for (auto& binding : m_TypeBindings)
		if (binding->Compare(types, size))
			return binding.get();

	throw std::runtime_error("No Binding inside of this type inside of registry");
}

TypeBinding* EntityRegistry::GetOrCreateBinding(const uint32_t* types, const size_t size)
{
	for (auto& binding : m_TypeBindings)
		if (binding->Compare(types, size))
			return binding.get();

	return AddBinding(types, size);
}

void EntityRegistry::Update(float deltaTime)
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

	// Update systems
	for (auto& system : m_Systems)
	{
#ifdef SYSTEM_PROFILER
		auto begin = std::chrono::high_resolution_clock::now();
		system->Update(deltaTime);
		auto end = std::chrono::high_resolution_clock::now();

		if (!m_ProfilerInfo.contains(system->GetSystemParameters().name))
			m_ProfilerInfo.emplace(system->GetSystemParameters().name, ProfilerInfo{ system.get() });

		// If leftover deltaTime is smaller than given DeltaTime it means it executed
		if (system->GetAccumulatedTime() < deltaTime)
		{
			float updateInterval = system->GetSystemParameters().updateInterval;
			int timeAdjustment = (updateInterval != 0.f) ? int(updateInterval / deltaTime) : 1;

			auto& profilerInfo = m_ProfilerInfo.find(system->GetSystemParameters().name)->second;
			++profilerInfo.timesExecuted;
			profilerInfo.timeToExecuteSystem = std::chrono::milliseconds((end - begin).count() / timeAdjustment);
			profilerInfo.timeToExecutePerComponent = profilerInfo.timeToExecuteSystem / system->GetEntityAmount() / timeAdjustment;
		}
#else
		system->Update(deltaTime);
#endif
	}

	// Remove deleted entities
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

	// Remove deleted components
	for (auto& idComp : m_RemovedComponents)
	{
		auto it = m_TypeViews.find(idComp.first);
		if (it != m_TypeViews.end())
		{
			it->second->Remove(idComp.second);
		}
	}
	m_RemovedComponents.clear();

	// Update Type views
	for (auto& typeView : m_TypeViews)
	{
		typeView.second->Update(deltaTime);
	}
		
}

void EntityRegistry::Serialize(std::ostream& stream) const
{
	{ // Get amount of systems that are not subsystems or default systems
		size_t SystemAmount{  };
		for (auto& system : m_Systems)
			if (!system->IsDefaulySystem() && !system->IsSubSystem())
				++SystemAmount;
		WriteStream(stream, SystemAmount);
	}

	for (auto& system : m_Systems)
	{
		if (!system->IsSubSystem() && !system->IsDefaulySystem())
		{
			stream << system->GetSystemParameters().name << '\n';
			WriteStream(stream, system->IsEnabled());
		}
	}
	
	WriteStream(stream, m_Entities.size());
	for (auto entity : m_Entities)
	{
		WriteStream(stream, entity);
	}

	WriteStream(stream, m_TypeViews.size());
	for (auto& typeView : m_TypeViews)
	{
		WriteStream(stream, typeView.first);
		typeView.second->SerializeView(stream);
	}
}

void EntityRegistry::Deserialize(std::istream& stream)
{
	size_t systemAmount{};
	ReadStream(stream, systemAmount);
	for (size_t i{}; i < systemAmount; ++i)
	{
		std::string systemName;
		std::getline(stream, systemName, '\n');

		bool isEnabled{};
		ReadStream(stream, isEnabled);

		auto pSystem = AddSystem(systemName);
		if (!isEnabled)
			pSystem->Disable();
	}

	size_t entityAmount{};
	ReadStream(stream, entityAmount);
	for (size_t i{}; i < entityAmount; ++i)
	{
		entityId id{};
		ReadStream(stream, id);
		m_Entities.emplace(id);
	}

	size_t viewsAmount{};
	ReadStream(stream, viewsAmount);
	for (size_t i{}; i < viewsAmount; ++i)
	{
		uint32_t id;
		ReadStream(stream, id);
		if (!m_TypeViews.contains(id))
			ECSTypeInformation::GetTypeViewAdders().find(id)->second(this);
		m_TypeViews[id]->DeserializeView(stream);
	}
}

VoidReference EntityRegistry::GetComponent(uint32_t typeId, entityId id)
{
	assert(id != Entity::InvalidId);
	auto it = m_TypeViews.find(typeId);
	if (it != m_TypeViews.end())
	{
		return it->second->GetVoidReference(id);
	}
	return VoidReference(nullptr);
}

VoidReference EntityRegistry::GetComponent(uint32_t typeId, const Entity& entity)
{
	return GetComponent(typeId, entity.GetId());
}

VoidReference EntityRegistry::AddComponentInstantly(uint32_t typeId, const Entity& entity)
{
	return AddComponentInstantly(typeId, entity.GetId());
}

VoidReference EntityRegistry::AddComponentInstantly(uint32_t typeId, entityId id)
{
	assert(id != Entity::InvalidId);
	assert(m_Entities.contains(id));
	auto it = m_TypeViews.find(typeId);
	if (it != m_TypeViews.end())
	{
		return it->second->AddEntity(id);
	}
	else
	{
		AddView(typeId);
		return AddComponentInstantly(typeId, id);
	}
}

void* EntityRegistry::AddComponent(uint32_t typeId, const Entity& entity)
{
	return AddComponent(typeId, entity.GetId());
}

void* EntityRegistry::AddComponent(uint32_t typeId, entityId id)
{
	assert(id != Entity::InvalidId);
	assert(m_Entities.contains(id));
	
	auto it = m_TypeViews.find(typeId);
	if (it != m_TypeViews.end())
	{
		return it->second->AddAfterUpdate_void(id);
	}
	else
	{
		AddView(typeId);
		return AddComponent(typeId, id);
	}
}

void EntityRegistry::RemoveComponent(uint32_t typeId, const Entity& entity)
{
	RemoveComponent(typeId, entity.GetId());
}

void EntityRegistry::RemoveComponent(uint32_t typeId, entityId id)
{
	m_RemovedComponents.emplace_back(typeId, id);
}

void EntityRegistry::RemoveComponentInstantly(uint32_t typeId, const Entity& entity)
{
	RemoveComponentInstantly(typeId, entity.GetId());
}

void EntityRegistry::RemoveComponentInstantly(uint32_t typeId, entityId id)
{
	m_Entities.erase(id);

	auto it = m_TypeViews.find(typeId);
	if (it != m_TypeViews.end())
	{
		it->second->Remove(id);
	}

}

void EntityRegistry::Enable(uint32_t typeId, entityId id)
{
	auto it = m_TypeViews.find(typeId);
	if (it != m_TypeViews.end())
	{
		it->second->Enable(id);
	}
}

void EntityRegistry::Enable(uint32_t typeId, const VoidReference& component)
{
	auto it = m_TypeViews.find(typeId);
	if (it != m_TypeViews.end())
	{
		it->second->Enable(component);
	}
}

void EntityRegistry::Enable(uint32_t typeId, const Entity& entity)
{
	Enable(typeId, entity.GetId());
}

void EntityRegistry::Disable(uint32_t typeId, entityId id)
{
	auto it = m_TypeViews.find(typeId);
	if (it != m_TypeViews.end())
	{
		it->second->Disable(id);
	}
}

void EntityRegistry::Disable(uint32_t typeId, const Entity& entity)
{
	Disable(typeId, entity.GetId());
}

void EntityRegistry::Disable(uint32_t typeId, const VoidReference& component)
{
	auto it = m_TypeViews.find(typeId);
	if (it != m_TypeViews.end())
	{
		it->second->Disable(component);
	}
}

bool EntityRegistry::IsEnabled(uint32_t typeId, entityId id) const
{
	auto it = m_TypeViews.find(typeId);
	if (it != m_TypeViews.end())
	{
		return it->second->IsEnabled(id);
	}
	return false;
}

bool EntityRegistry::IsEnabled(uint32_t typeId, const Entity& entity) const
{
	return IsEnabled(typeId, entity.GetId());
}

bool EntityRegistry::IsEnabled(uint32_t typeId, const VoidReference& component) const
{
	auto it = m_TypeViews.find(typeId);
	if (it != m_TypeViews.end())
	{
		return it->second->IsEnabled(component);
	}
	return false;
}

void EntityRegistry::EnableEntity(entityId id)
{
	for (auto& view : m_TypeViews)
		if (view.second->Contains(id))
			view.second->Enable(id);
}

void EntityRegistry::EnableEntity(const Entity& entity)
{
	EnableEntity(entity.GetId());
}

void EntityRegistry::DisableEntity(entityId id)
{
	for (auto& view : m_TypeViews)
		if (view.second->Contains(id))
			view.second->Disable(id);
}

void EntityRegistry::DisableEntity(const Entity& entity)
{
	DisableEntity(entity.GetId());
}
