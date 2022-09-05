#pragma once

#include <unordered_map>
#include <memory>
#include <vector>
#include <functional>
#include <stdexcept>
#include <fstream>
#include <set>
#include <unordered_set>
#include <sstream>

#include "../TypeInformation/reflection.h"
#include "../TypeInformation/TypeInformation.h"
#include "../Sorting/SorterThreadPool.h"
#include "TypeBinding.h"
#include "TypeView.h"
#include "../System/System.h"

class EntityRegistry final
{
private:

	/** System Profiler info*/
	struct ProfilerInfo
	{
		uint64_t timesExecuted;
		std::chrono::milliseconds timeToExecuteSystem;
		std::chrono::microseconds timeToExecutePerComponent;
	};

public:

	EntityRegistry() = default;
	~EntityRegistry();

	EntityRegistry(const EntityRegistry&)				= delete;
	EntityRegistry(EntityRegistry&&)					= delete;
	EntityRegistry& operator=(const EntityRegistry&)	= delete;
	EntityRegistry& operator=(EntityRegistry&&)			= delete;

public:

	/**
	 * SYSTEMS
	 */

	template <typename Type>
	void AddSystem(const SystemParameters& parameters, const std::function<void(Type&)>& function);

	template <typename... Types>
	void AddSystem(const SystemParameters& parameters, const std::function<void(Types&...)>& function) requires (sizeof...(Types) >= 2);

	template <typename System>
	void AddSystem(const SystemParameters& parameters) requires std::is_base_of_v<SystemBase, System>;

	void AddSystem(const std::string& name);

	void PrintSystems() const;
	void PrintSystems(std::ostream& stream) const;

	void PrintSystemInformation() const;
	void PrintSystemInformation(std::ostream& stream) const;

	void RemoveSystem(std::string name);

	/**
	 * VIEWS
	 */

	TypeViewBase* AddView(uint32_t typeId);

	TypeViewBase* GetTypeView(uint32_t typeId) const;

	TypeViewBase* GetOrCreateView(uint32_t typeId);

	template <typename T>
	TypeView<T>& AddView();

	template <typename T>
	TypeView<T>& GetTypeView() const;

	template <typename T>
	TypeView<T>& GetOrCreateView();

	/**
	 * ENTITIES
	 */

	Entity CreateEntity();

	void RemoveEntity(const Entity& entity);
	void RemoveEntity(entityId id);

	const std::unordered_set<entityId>& GetEntities() const { return m_Entities; }

	const Entity CreateOrGetEntity(entityId id);

	/**
	 * BINDINGS
	 */

	template <typename... Types>
	TypeBinding* AddBinding();

	TypeBinding* AddBinding(const uint32_t* typeIds, size_t size);

	template <typename... Types>
	[[nodiscard]] TypeBinding* GetBinding();

	[[nodiscard]] TypeBinding* GetBinding(const uint32_t* types, const size_t size) const;

	[[nodiscard]] TypeBinding* GetOrCreateBinding(const uint32_t* types, const size_t size);

	template <typename... Types>
	[[nodiscard]] TypeBinding* GetOrCreateBinding();

	/**
	 * MISC
	 */

	void Update(float deltaTime);

	void Serialize(std::ostream& stream) const;

	void Deserialize(std::istream& stream);

	/**
	 * COMPONENTS
	 */

	template <typename T>
	Reference<T> GetComponent(const Entity& entity);

	template <typename T>
	Reference<T> GetComponent(entityId id);

	template <typename T>
	Reference<T> AddComponentInstantly(entityId id);

	template <typename T>
	Reference<T> AddComponentInstantly(Entity entity);

	template <typename T>
	T* AddComponent(entityId id);

	template <typename T>
	T* AddComponent(Entity entity);

	template <typename T>
	void RemoveComponent(entityId id);

	template <typename T>
	void RemoveComponent(Entity entity);

private:

	/** Systems*/
	template <typename Component>
	void AddDynamicViewSubSystems(const SystemParameters& parameters, const std::function<void(Component&)>& function);

	template <typename... Components>
	void AddDynamicBindingSubSystems(const SystemParameters& parameters, const std::function<void(Components&...)>& function);

	template <typename System>
	void AddViewSystem(const SystemParameters& parameters);

	template <typename System>
	void AddBindingSystem(const SystemParameters& parameters);

	template <typename System>
	void AddViewSubSystem(const SystemParameters& parameters);

	template <typename System>
	void AddBindingSubSystem(const SystemParameters& parameters);


private:

	/** Entities*/

	std::unordered_set<entityId> m_Entities;
	size_t m_EntityCounter{};

	/** Component views*/

	std::unordered_map<uint32_t, std::unique_ptr<TypeViewBase>> m_TypeViews;

	/** Component bindings*/

	std::vector<std::unique_ptr<TypeBinding>> m_TypeBindings;

	/** Removing entities*/

	std::vector<entityId> m_RemovedEntities;
	std::vector<std::pair<uint32_t, entityId>> m_RemovedComponents;

	/** Systems*/

	std::multiset < std::unique_ptr<SystemBase>,
		decltype([](const std::unique_ptr<SystemBase>& v0, const std::unique_ptr<SystemBase>& v1)
			{return v0->GetSystemParameters().executionTime < v1->GetSystemParameters().executionTime; }) > m_Systems;

	/** Sorting*/
	static constexpr size_t ThreadPoolSize{ 4 };

	ThreadPool m_SortingThreadPool{ThreadPoolSize};
	std::array<volatile SortingProgress, ThreadPoolSize> m_SortingProgress{};

#ifdef SYSTEM_PROFILER
	std::unordered_map<std::string, ProfilerInfo> m_ProfilerInfo;
#endif
};

//template <typename T>
//TypeView<T>& EntityRegistry::GetView()
//{
//	auto it = m_TypeViews.find(reflection::type_id<T>());
//	if (it != m_TypeViews.end())
//	{
//		return *reinterpret_cast<TypeView<T>*>(it->second);
//	}
//
//	throw std::runtime_error("View was not found inside of registry");
//}

template <typename Component>
void EntityRegistry::AddSystem(const SystemParameters& parameters, const std::function<void(Component&)>& function)
{
	// Make sure the name is not in there already
	assert(m_Systems.end() == std::find_if(m_Systems.begin(), m_Systems.end(), [parameters](const std::unique_ptr<SystemBase>& sys) {return sys->GetSystemParameters().name == parameters.name; }));

	auto view = &GetOrCreateView<Component>();
	auto system = new ViewSystemDynamic<Component>{ parameters, function };

	system->SetTypeView(view);
	system->Initialize();

	m_Systems.emplace(system);

	AddDynamicViewSubSystems<Component>(parameters, function);
}

template <typename... Components>
void EntityRegistry::AddSystem(const SystemParameters& parameters, const std::function<void(Components&...)>& function) requires (sizeof...(Components) >= 2)
{
	// Make sure the name is not in there already
	assert(m_Systems.end() == std::find_if(m_Systems.begin(), m_Systems.end(), [parameters](const std::unique_ptr<SystemBase>& sys) {return sys->GetSystemParameters().name == parameters.name; }));

	TypeBinding* binding{ GetOrCreateBinding<Components...>() };
	auto system = new BindingSystemDynamic<Components...>{ parameters, function };

	system->SetTypeBinding(binding);
	system->Initialize();

	m_Systems.emplace(system);

	AddDynamicBindingSubSystems(parameters, function);
}

template <typename System>	
void EntityRegistry::AddSystem(const SystemParameters& parameters) requires std::is_base_of_v<SystemBase, System>
{
	assert(m_Systems.end() == std::find_if(m_Systems.begin(), m_Systems.end(), [parameters](const std::unique_ptr<SystemBase>& sys) {return sys->GetSystemParameters().name == parameters.name; }));

	if constexpr (isBindingSystem<System>)
	{
		AddBindingSystem<System>(parameters);
	}
	else
	{
		AddViewSystem<System>(parameters);
	}
}

template <typename T>
TypeView<T>& EntityRegistry::AddView()
{
	auto view = new TypeView<T>(this);
	constexpr uint32_t typeId{ reflection::type_id<T>() };
	m_TypeViews.emplace(typeId, view);

	return *view;
}

template <typename T>
TypeView<T>& EntityRegistry::GetOrCreateView()
{
	auto it = m_TypeViews.find(reflection::type_id<T>());
	if (it != m_TypeViews.end())
		return *reinterpret_cast<TypeView<T>*>(it->second.get());
	else
		return AddView<T>();
}

template <typename ... Types>
[[nodiscard]] TypeBinding* EntityRegistry::GetBinding()
{
	auto types = reflection::Type_ids<Types...>();
	return GetBinding(types.data(), types.size());
}

template <typename ... Types>
TypeBinding* EntityRegistry::GetOrCreateBinding()
{
	auto types = reflection::Type_ids<Types...>();
	return GetOrCreateBinding(types.data(), types.size());
}

template <typename T>
Reference<T> EntityRegistry::GetComponent(const Entity& entity)
{
	return GetComponent<T>(entity.GetId());
}

template <typename T>
Reference<T> EntityRegistry::GetComponent(entityId id)
{
	assert(id != Entity::InvalidId);
	auto it = m_TypeViews.find(reflection::type_id<T>());
	if (it != m_TypeViews.end())
	{
		return reinterpret_cast<TypeView<T>*>(it->second.get())->Get(id);
	}
	return Reference<T>::InvalidRef();
}

template <typename T>
Reference<T> EntityRegistry::AddComponentInstantly(entityId id)
{
	assert(id != Entity::InvalidId);
	assert(m_Entities.contains(id));
	constexpr uint32_t typeId = reflection::type_id<T>();
	auto it = m_TypeViews.find(typeId);
	if (it != m_TypeViews.end())
	{
		return reinterpret_cast<TypeView<T>*>(it->second.get())->Add(id);
	}
	else
	{
		AddView<T>();
		return AddComponentInstantly<T>(id);
	}
}

template <typename T>
Reference<T> EntityRegistry::AddComponentInstantly(Entity entity)
{
	return AddComponentInstantly<T>(entity.GetId());
}

template <typename T>
T* EntityRegistry::AddComponent(entityId id)
{
	assert(id != Entity::InvalidId);
	assert(m_Entities.contains(id));

	constexpr uint32_t typeId = reflection::type_id<T>();
	auto it = m_TypeViews.find(typeId);
	if (it != m_TypeViews.end())
	{
		return reinterpret_cast<TypeView<T>*>(it->second.get())->AddAfterUpdate(id);
	}
	else
	{
		AddView<T>();
		return AddComponent<T>(id);
	}
}

template <typename T>
T* EntityRegistry::AddComponent(Entity entity)
{
	return AddComponent<T>(entity.GetId());
}

template <typename T>
void EntityRegistry::RemoveComponent(entityId id)
{
	m_RemovedComponents.emplace_back(reflection::type_id<T>(), id);
}

template <typename T>
void EntityRegistry::RemoveComponent(Entity entity)
{
	RemoveComponent<T>(entity.GetId());
}

template <typename Component>
void EntityRegistry::AddDynamicViewSubSystems(const SystemParameters& parameters,
	const std::function<void(Component&)>& function)
{
	constexpr uint32_t typeId = reflection::type_id<Component>();
	const std::vector<uint32_t> subClasses{ TypeInformation::GetSubClasses(typeId) };
	for (auto subclassId : subClasses)
	{
		TypeView<Component>* view = reinterpret_cast<TypeView<Component>*>(GetOrCreateView(subclassId));

		SystemParameters newParams = parameters;
		newParams.name = parameters.name + "_" + std::string(TypeInformation::GetTypeName(subclassId));

		auto system = new ViewSystemDynamic<Component>{ newParams, function };

		system->SetTypeView(view);
		system->Initialize();

		m_Systems.emplace(system);
	}
}

template <typename ... Components>
void EntityRegistry::AddDynamicBindingSubSystems(const SystemParameters& parameters,
	const std::function<void(Components&...)>& function)
{
	// Get The Combinations that can be made using the given components and their child classes
	constexpr size_t typesAmount{ sizeof...(Components) };
	constexpr auto typeIds{ reflection::Type_ids<Components...>() };
	const std::vector<uint32_t> SubClassesCombinations{ TypeInformation::GetSubTypeCombinations(typeIds.data(), typeIds.size()) };

	for (size_t i{}; i < SubClassesCombinations.size(); i += typesAmount)
	{
		std::array<uint32_t, typesAmount> subTypeIds;

		std::stringstream sBuffer;
		sBuffer << parameters.name;
		for (size_t j{}; j < typesAmount; ++j)
		{
			subTypeIds[j] = SubClassesCombinations[i];

			if (j != typesAmount - 1)
				sBuffer << '_';
			sBuffer << TypeInformation::GetTypeName(SubClassesCombinations[i]);
		}
		SystemParameters newParams = parameters;
		newParams.name = sBuffer.str();

		auto binding = GetOrCreateBinding(subTypeIds.data(), subTypeIds.size());

		auto subSystem = new BindingSystemDynamic<Components...>{ newParams, function };

		subSystem->SetTypeBinding(binding);
		subSystem->Initialize();

		m_Systems.emplace(subSystem);
	}
}

template <typename System>
void EntityRegistry::AddViewSystem(const SystemParameters& parameters)
{
	using Component = System::ComponentType;

	auto view = &GetOrCreateView<Component>();
	auto system = new System(parameters);

	system->SetTypeView(view);
	system->Initialize();

	m_Systems.emplace(system);

	AddViewSubSystem<System>(parameters);
}

template <typename System>
void EntityRegistry::AddBindingSystem(const SystemParameters& parameters)
{
	constexpr auto types = System::GetTypes();
	TypeBinding* binding{ GetOrCreateBinding(types.data(), types.size()) };
	auto system = new System{ parameters };

	system->SetTypeBinding(binding);
	system->Initialize();

	m_Systems.emplace(system);

	AddBindingSubSystem<System>(parameters);
}

template <typename System>
void EntityRegistry::AddViewSubSystem(const SystemParameters& parameters)
{
	using Component = System::ComponentType;

	constexpr uint32_t typeId{ reflection::type_id<Component>() };
	const std::vector<uint32_t> subClasses{ TypeInformation::GetSubClasses(typeId) };
	for (auto SubClassId : subClasses)
	{
		TypeView<Component>* subView = reinterpret_cast<TypeView<Component>*>(GetOrCreateView(SubClassId));

		SystemParameters newParams = parameters;
		newParams.name = parameters.name + "_" + std::string(TypeInformation::GetTypeName(SubClassId));

		auto system = new System( newParams );

		system->SetTypeView(subView);
		system->Initialize();

		m_Systems.emplace(system);
	}
}

template <typename System>
void EntityRegistry::AddBindingSubSystem(const SystemParameters& parameters)
{
	// Get The Combinations that can be made using the given components and their child classes
	constexpr auto typeIds = System::GetTypes();
	constexpr size_t typesAmount{ typeIds.size() };
	const std::vector<uint32_t> SubClassesCombinations{ TypeInformation::GetSubTypeCombinations(typeIds.data(), typeIds.size()) };

	for (size_t i{}; i < SubClassesCombinations.size(); i += typesAmount)
	{
		std::array<uint32_t, typesAmount> subTypeIds;

		std::stringstream sBuffer;
		sBuffer << parameters.name;
		for (size_t j{}; j < typesAmount; ++j)
		{
			subTypeIds[j] = SubClassesCombinations[i];

			if (j != typesAmount - 1)
				sBuffer << '_';
			sBuffer << TypeInformation::GetTypeName(SubClassesCombinations[i]);
		}
		SystemParameters newParams = parameters;
		newParams.name = sBuffer.str();

		auto binding = GetOrCreateBinding(subTypeIds.data(), subTypeIds.size());

		auto subSystem = new System{ newParams };

		subSystem->SetTypeBinding(binding);
		subSystem->Initialize();

		m_Systems.emplace(subSystem);
	}
}

template <typename ... Types>
TypeBinding* EntityRegistry::AddBinding()
{
	auto typeIds = reflection::Type_ids<Types...>();
	return AddBinding(typeIds.data(), typeIds.size());
}
