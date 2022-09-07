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

	/** Add Dynamic View System to the Registry*/
	template <typename Component>
	SystemBase* AddSystem(const SystemParameters& parameters, const std::function<void(Component&)>& function, bool AddSubSystems = true);

	/** Add Dynamic Binding System to the Registry*/
	template <typename... Component>
	SystemBase* AddSystem(const SystemParameters& parameters, const std::function<void(Component&...)>& function, bool AddSubSystems = true) requires (sizeof...(Component) >= 2);

	template <typename Component>
	SystemBase* AddSystem(const SystemParameters& parameters, const std::function<void(float, Component&)>& functionDT, bool AddSubSystems = true);

	template <typename... Components>
	SystemBase* AddSystem(const SystemParameters& parameters, const std::function<void(float, Components&...)>& functionDT, bool AddSubSystems = true) requires (sizeof...(Components) >= 2);

	/** Add the specific System to the Registry given as the template parameter*/
	template <typename System>
	SystemBase* AddSystem(const SystemParameters& parameters, bool AddSubSystems = true) requires std::is_base_of_v<SystemBase, System>;

	template <typename Component>
	void AddDefaultSystems();

	void AddDefaultSystems(uint32_t typeId);

	/**
	 * Add a System to the registry given the name of the system
	 * The System has to be registered using the RegisterSystem<System>() or RegisterDynamicSystem(function)
	 * Look at TypeInformation/TypeInfoGenerator.h for more info
	 */
	SystemBase* AddSystem(const std::string& name);

	/** Prints the names of the added Systems to the stream without default and subsystems*/
	void PrintSystems(std::ostream& stream) const;
	/** Prints the names of the added Systems to the console*/
	void PrintSystems() const;

	void PrintAllSystems(std::ostream& stream) const;
	void PrintAllSystems() const;

	/**
	 * Prints information about each registered system to the console. Information includes:
	 *  - Name of the system
	 *	- When the system executes compared to other systems
	 *	- The time it takes for systems to update
	 * Additionally if macro SYSTEM_PROFILE is defined:
	 *  - How many times the system has executed
	 *  - The time it takes to execute the system once
	 *	- The time to execute the system divided by the amount of Components inside the System
	 */
	void PrintSystemInformation(std::ostream& stream) const;
	/** Prints information about the registered Systems to the console*/
	void PrintSystemInformation() const;

	/**
	 * Removes the System from the registry given its name
	 */
	void RemoveSystem(std::string name);

	/**
	 * VIEWS
	 */

	/**
	 * Adds the view to the Registry given the typeId (reflection::typeId<Component>()) of the Component.
	 * For this to work the Component should have been registered before hand using RegisterClass<> found in TypeInfoGenerator.h
	 */
	TypeViewBase* AddView(uint32_t typeId);

	/** Returns the type view that corresponds with the typeId (reflection::typeId<Component>()) of the class.*/
	TypeViewBase* GetTypeView(uint32_t typeId) const;

	/**
	 * Returns the type view that corresponds with the typeId (reflection::typeId<Component>()) of the class
	 * If the View does not exist in the Registry it will create it.
	 */
	TypeViewBase* GetOrCreateView(uint32_t typeId);

	/** Adds the Component View to the Registry given the Component class.*/
	template <typename Component>
	TypeView<Component>& AddView();

	/** Gets the Component View corresponding with the given Component type*/
	template <typename Component>
	TypeView<Component>& GetTypeView() const;

	/**
	 * Gets the Component View corresponding with the given Component type.
	 * If the View is not in the Registry it will create it.
	 */
	template <typename Component>
	TypeView<Component>& GetOrCreateView();

	/**
	 * ENTITIES
	 */

	/** Creates an Entity that is linked to the Registry*/
	Entity CreateEntity();

	/** Removes the Entity from the Registry and removes its components*/
	void RemoveEntity(const Entity& entity);
	void RemoveEntity(entityId id);

	/** Gets the container with all the Entities*/
	const std::unordered_set<entityId>& GetEntities() const { return m_Entities; }

	/** Gets the entity from the Registry or creates it if it does not exist in the Registry*/
	const Entity CreateOrGetEntity(entityId id);

	/**
	 * BINDINGS
	 */

	/** Add a Component Binding to the Registry. If one already exists using the same Components but in a different order this method will assert*/
	template <typename... Components>
	TypeBinding* AddBinding();

	/** Add a Component Binding using TypeIds instead of templates*/
	TypeBinding* AddBinding(const uint32_t* typeIds, size_t size);

	/** Gets the Component Binding corresponding the Given Components*/
	template <typename... Components>
	[[nodiscard]] TypeBinding* GetBinding();

	/** Returns the Component Binding corresponding to the given TypeIds*/
	[[nodiscard]] TypeBinding* GetBinding(const uint32_t* types, const size_t size) const;

	/**
	 * Returns the Component Binding corresponding to the given TypeIds.
	 * If the Component Binding does not exist it will create it.
	 */
	[[nodiscard]] TypeBinding* GetOrCreateBinding(const uint32_t* types, const size_t size);

	/**
	 * Returns the Component Binding corresponding to the given Components.
	 * If the Component Binding does not exist it will create it.
	 */
	template <typename... Components>
	[[nodiscard]] TypeBinding* GetOrCreateBinding();

	/**
	 * MISC
	 */

	/** Updates the Registry using delta Time.*/
	void Update(float deltaTime);

	/** Serializes the Registry to the given stream.*/
	void Serialize(std::ostream& stream) const;

	/** Deserialize the Registry from the given stream.*/
	void Deserialize(std::istream& stream);

	/**
	 * COMPONENTS
	 */

	/** Gets the Component that is attached to the given entity*/
	template <typename Component>
	Reference<Component> GetComponent(const Entity& entity);
	template <typename Component>
	Reference<Component> GetComponent(entityId id);

	/** Adds a Component to the given entity*/
	template <typename Component>
	Reference<Component> AddComponentInstantly(const Entity& entity);
	template <typename Component>
	Reference<Component> AddComponentInstantly(entityId id);

	/** Adds the component to the given entity at the end of the Update cycle*/
	template <typename Component>
	Component* AddComponent(const Entity& entity);
	template <typename Component>
	Component* AddComponent(entityId id);

	/** Removes the component from the given Entity*/
	template <typename Component>
	void RemoveComponent(const Entity& entity);
	template <typename Component>
	void RemoveComponent(entityId id);

private:

	/**
	 * Systems Helper function
	 */

	template <typename Component>
	void AddDynamicViewSubSystems(const SystemParameters& parameters, const std::function<void(Component&)>& function);

	template <typename... Components>
	void AddDynamicBindingSubSystems(const SystemParameters& parameters, const std::function<void(Components&...)>& function);

	template <typename Component>
	void AddDynamicViewSubSystemsDT(const SystemParameters& parameters, const std::function<void(float, Component&)>& functionDT);

	template <typename... Components>
	void AddDynamicBindingSubSystemsDT(const SystemParameters& parameters, const std::function<void(float, Components&...)>& functionDT);

	template <typename System>
	SystemBase* AddViewSystem(const SystemParameters& parameters, bool AddSubSystems = true);

	template <typename System>
	SystemBase* AddBindingSystem(const SystemParameters& parameters, bool AddSubSystems = true);

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

template <typename Component>
SystemBase* EntityRegistry::AddSystem(const SystemParameters& parameters, const std::function<void(Component&)>& function, bool AddSubSystems)
{
	// Make sure the name is not in there already
	assert(m_Systems.end() == std::find_if(m_Systems.begin(), m_Systems.end(), [parameters](const std::unique_ptr<SystemBase>& sys) {return sys->GetSystemParameters().name == parameters.name; }));

	auto view = &GetOrCreateView<Component>();
	auto system = new ViewSystemDynamic<Component>{ parameters, function };

	system->SetTypeView(view);
	system->Initialize();

	m_Systems.emplace(system);

	if (AddSubSystems)
		AddDynamicViewSubSystems<Component>(parameters, function);

	return system;
}

template <typename... Components>
SystemBase* EntityRegistry::AddSystem(const SystemParameters& parameters, const std::function<void(Components&...)>& function, bool AddSubSystems) requires (sizeof...(Components) >= 2)
{
	// Make sure the name is not in there already
	assert(m_Systems.end() == std::find_if(m_Systems.begin(), m_Systems.end(), [parameters](const std::unique_ptr<SystemBase>& sys) {return sys->GetSystemParameters().name == parameters.name; }));

	TypeBinding* binding{ GetOrCreateBinding<Components...>() };
	auto system = new BindingSystemDynamic<Components...>{ parameters, function };

	system->SetTypeBinding(binding);
	system->Initialize();

	m_Systems.emplace(system);

	if (AddSubSystems)
		AddDynamicBindingSubSystems(parameters, function);

	return system;
}

template <typename Component>
SystemBase* EntityRegistry::AddSystem(const SystemParameters& parameters, const std::function<void(float, Component&)>& functionDT,
	bool AddSubSystems)
{
	// Make sure the name is not in there already
	assert(m_Systems.end() == std::find_if(m_Systems.begin(), m_Systems.end(), [parameters](const std::unique_ptr<SystemBase>& sys) {return sys->GetSystemParameters().name == parameters.name; }));

	auto view = &GetOrCreateView<Component>();
	auto system = new ViewSystemDynamicDT<Component>{ parameters, functionDT };

	system->SetTypeView(view);
	system->Initialize();

	m_Systems.emplace(system);

	if (AddSubSystems)
		AddDynamicViewSubSystemsDT<Component>(parameters, functionDT);

	return system;
}

template <typename ... Components>
SystemBase* EntityRegistry::AddSystem(const SystemParameters& parameters,
	const std::function<void(float, Components&...)>& functionDT, bool AddSubSystems) requires (sizeof...(Components) >= 2)
{
	// Make sure the name is not in there already
	assert(m_Systems.end() == std::find_if(m_Systems.begin(), m_Systems.end(), [parameters](const std::unique_ptr<SystemBase>& sys) {return sys->GetSystemParameters().name == parameters.name; }));

	TypeBinding* binding{ GetOrCreateBinding<Components...>() };
	auto system = new BindingSystemDynamicDT<Components...>{ parameters, functionDT };

	system->SetTypeBinding(binding);
	system->Initialize();

	m_Systems.emplace(system);

	if (AddSubSystems)
		AddDynamicBindingSubSystems(parameters, functionDT);

	return system;
}

template <typename System>	
SystemBase* EntityRegistry::AddSystem(const SystemParameters& parameters, bool AddSubSystems) requires std::is_base_of_v<SystemBase, System>
{
	assert(m_Systems.end() == std::find_if(m_Systems.begin(), m_Systems.end(), [parameters](const std::unique_ptr<SystemBase>& sys) {return sys->GetSystemParameters().name == parameters.name; }));

	if constexpr (isBindingSystem<System>)
	{
		return AddBindingSystem<System>(parameters, AddSubSystems);
	}
	else
	{
		return AddViewSystem<System>(parameters, AddSubSystems);
	}
}

template <typename Component>
void EntityRegistry::AddDefaultSystems()
{
	constexpr uint32_t typeId{ reflection::type_id<Component>() };
	AddDefaultSystems(typeId);
}

template <typename Component>
TypeView<Component>& EntityRegistry::AddView()
{
	auto view = new TypeView<Component>(this);
	constexpr uint32_t typeId{ reflection::type_id<Component>() };
	m_TypeViews.emplace(typeId, view);

	// Add default Systems
	AddDefaultSystems(typeId);

	return *view;
}

template <typename Component>
TypeView<Component>& EntityRegistry::GetTypeView() const
{
	auto it = m_TypeViews.find(reflection::type_id<Component>());
	if (it != m_TypeViews.end())
		return *reinterpret_cast<TypeView<Component>*>(it->second.get());

	throw std::runtime_error("Component View not in registry");
}

template <typename Component>
TypeView<Component>& EntityRegistry::GetOrCreateView()
{
	auto it = m_TypeViews.find(reflection::type_id<Component>());
	if (it != m_TypeViews.end())
		return *reinterpret_cast<TypeView<Component>*>(it->second.get());
	else
		return AddView<Component>();
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
Reference<T> EntityRegistry::AddComponentInstantly(const Entity& entity)
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
T* EntityRegistry::AddComponent(const Entity& entity)
{
	return AddComponent<T>(entity.GetId());
}

template <typename T>
void EntityRegistry::RemoveComponent(entityId id)
{
	m_RemovedComponents.emplace_back(reflection::type_id<T>(), id);
}

template <typename T>
void EntityRegistry::RemoveComponent(const Entity& entity)
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
		system->SetFlag(SystemFlags::SubSystem, true);
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
		subSystem->SetFlag(SystemFlags::SubSystem, true);
		subSystem->Initialize();

		m_Systems.emplace(subSystem);
	}
}

template <typename Component>
void EntityRegistry::AddDynamicViewSubSystemsDT(const SystemParameters& parameters,
	const std::function<void(float, Component&)>& functionDT)
{
	constexpr uint32_t typeId = reflection::type_id<Component>();
	const std::vector<uint32_t> subClasses{ TypeInformation::GetSubClasses(typeId) };
	for (auto subclassId : subClasses)
	{
		TypeView<Component>* view = reinterpret_cast<TypeView<Component>*>(GetOrCreateView(subclassId));

		SystemParameters newParams = parameters;
		newParams.name = parameters.name + "_" + std::string(TypeInformation::GetTypeName(subclassId));

		auto system = new ViewSystemDynamicDT<Component>{ newParams, functionDT };

		system->SetTypeView(view);
		system->SetFlag(SystemFlags::SubSystem, true);
		system->Initialize();

		m_Systems.emplace(system);
	}
}

template <typename ... Components>
void EntityRegistry::AddDynamicBindingSubSystemsDT(const SystemParameters& parameters,
	const std::function<void(float, Components&...)>& functionDT)
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

		auto subSystem = new BindingSystemDynamicDT<Components...>{ newParams, functionDT };

		subSystem->SetTypeBinding(binding);
		subSystem->SetFlag(SystemFlags::SubSystem, true);
		subSystem->Initialize();

		m_Systems.emplace(subSystem);
	}
}

template <typename System>
SystemBase* EntityRegistry::AddViewSystem(const SystemParameters& parameters, bool AddSubSystems)
{
	using Component = System::ComponentType;

	auto view = &GetOrCreateView<Component>();
	auto system = new System(parameters);

	system->SetTypeView(view);
	system->Initialize();

	m_Systems.emplace(system);

	if (AddSubSystems)
		AddViewSubSystem<System>(parameters);

	return system;
}

template <typename System>
SystemBase* EntityRegistry::AddBindingSystem(const SystemParameters& parameters, bool AddSubSystems)
{
	constexpr auto types = System::GetTypes();
	TypeBinding* binding{ GetOrCreateBinding(types.data(), types.size()) };
	auto system = new System{ parameters };

	system->SetTypeBinding(binding);
	system->Initialize();

	m_Systems.emplace(system);

	if (AddSubSystems)
		AddBindingSubSystem<System>(parameters);

	return system;
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
		system->SetFlag(SystemFlags::SubSystem, true);
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
		subSystem->SetFlag(SystemFlags::SubSystem, true);
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
