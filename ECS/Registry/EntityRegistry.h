#pragma once

#include <unordered_map>
#include <memory>
#include <vector>
#include <functional>
#include <stdexcept>
#include <fstream>
#include <set>
#include <unordered_set>

#include "../TypeInformation/reflection.h"
#include "../Sorting/SorterThreadPool.h"
#include "TypeBinding.h"
#include "TypeView.h"
#include "../System/System.h"

class EntityRegistry final
{
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
	void AddSystem(const std::string& name, const std::function<void(Type&)>& function, int32_t executionOrder = int32_t(ExecutionTime::Update));

	template <typename... Types>
	void AddSystem(const std::string& name, const std::function<void(Types&...)>& function, int32_t executionOrder = int32_t(ExecutionTime::Update)) requires (sizeof...(Types) >= 2);

	template <typename System>
	void AddSystem(const std::string& name, int32_t executionOrder = int32_t(ExecutionTime::Update)) requires std::is_base_of_v<SystemBase, System>;

	template <typename System>
	void AddSystem() requires std::is_base_of_v<SystemBase, System>;

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

	void Update();

	void Serialize(std::ofstream& stream);

	void Deserialize(std::ifstream& stream);

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

	std::unordered_set<entityId> m_Entities;

	std::unordered_map<uint32_t, std::unique_ptr<TypeViewBase>> m_TypeViews;

	std::vector<TypeBinding*> m_TypeBindings;

	std::vector<TypeViewBase*> m_GameComponentTypeViews;
	
	std::vector<entityId> m_RemovedEntities;

	std::vector<std::pair<uint32_t, entityId>> m_RemovedComponents;

	std::multiset<std::unique_ptr<SystemBase>,
		decltype([](const std::unique_ptr<SystemBase>& v0, const std::unique_ptr<SystemBase>& v1)
			{return v0->GetExecutionOrder() < v1->GetExecutionOrder(); }) > m_Systems;

private:

	// sorting
	static constexpr size_t ThreadPoolSize{ 4 };
	ThreadPool m_SortingThreadPool{ThreadPoolSize};
	std::array<volatile SortingProgress, ThreadPoolSize> m_SortingProgress;

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

template <typename Type>
void EntityRegistry::AddSystem(const std::string& name, const std::function<void(Type&)>& function,
	int32_t executionOrder)
{
	// Make sure the name is not in there already
	assert(m_Systems.end() == std::find_if(m_Systems.begin(), m_Systems.end(), [name](const std::unique_ptr<SystemBase>& sys) {return sys->GetName() == name; }));

	{
		TypeView<Type>* view{};
		auto it = m_TypeViews.find(reflection::type_id<Type>());
		if (it == m_TypeViews.end())
			view = reinterpret_cast<TypeView<Type>*>(&AddView<Type>());
		else
		{
			TypeViewBase* viewBase = it->second.get();
			view = reinterpret_cast<TypeView<Type>*>(viewBase);
		}

		auto system = new ViewSystemDynamic<Type>{ name, view, function, executionOrder };

		m_Systems.emplace(system);
	}

	//Add subsystems
	//std::vector<uint32_t> subClasses;
	//size_t processedCounter{};
	//
	//for (auto& childId : TypeInformation::Data::ClassHierarchy[reflection::type_id<Type>()])
	//	subClasses.emplace_back(childId);
	//
	//while (subClasses.size() != processedCounter)
	//{
	//	for (auto& childId : TypeInformation::Data::ClassHierarchy[subClasses[processedCounter]])
	//	{
	//		subClasses.emplace_back(childId);
	//	}
	//	++processedCounter;
	//}
	//
	//for (auto SubClassId : subClasses)
	//{
	//	TypeView<Type>* view{};
	//	auto it = m_TypeViews.find(SubClassId);
	//	if (it == m_TypeViews.end())
	//		view = reinterpret_cast<TypeView<Type>*>(&AddView<Type>());
	//	else
	//	{
	//		TypeViewBase* viewBase = it->second.get();
	//		view = reinterpret_cast<TypeView<Type>*>(viewBase);
	//	}
	//
	//	auto typeName = TypeInformation::Data::TypeInformation[SubClassId].m_TypeName;
	//	auto system = new ViewSystemDynamic<Type>{ name + "_" + std::string(typeName), view, function, executionOrder};
	//
	//	m_Systems.emplace(system);
	//}
}

template <typename ... Types>
void EntityRegistry::AddSystem(const std::string& name, const std::function<void(Types&...)>& function, int32_t executionOrder) requires (sizeof...(Types) >= 2)
{
	// Make sure the name is not in there already
	assert(m_Systems.end() == std::find_if(m_Systems.begin(), m_Systems.end(), [name](const std::unique_ptr<SystemBase>& sys) {return sys->GetName() == name; }));

	TypeBinding* binding{GetOrCreateBinding<Types...>()};

	auto system = new BindingSystemDynamic<Types...>{ name, binding, function, executionOrder };

	m_Systems.emplace(system);
}

template <typename System>
concept SystemSimpleConstructor = requires(System sys, TypeView<int> view) { System{ view }; };

template <typename System>	
void EntityRegistry::AddSystem(const std::string& name, int32_t executionOrder) requires std::is_base_of_v<SystemBase, System>
{
	assert(m_Systems.end() == std::find_if(m_Systems.begin(), m_Systems.end(), [name](const std::unique_ptr<SystemBase>& sys) {return sys->GetName() == name; }));

	if constexpr (isViewSystem<System>)
	{
		using T = System::ElementType;

		TypeView<T>* view{};
		auto it = m_TypeViews.find(reflection::type_id<T>());
		if (it == m_TypeViews.end())
			view = reinterpret_cast<TypeView<T>*>(&AddView<T>());
		else
			view = reinterpret_cast<TypeView<T>*>(&*it);

		auto system = new System(name, view, executionOrder);
		m_Systems.emplace(system);
	}
	else if constexpr (isBindingSystem<System>)
	{
		auto types = System::GetTypes();
		TypeBinding* binding{ GetBinding(types.data(), types.size()) };
		
		auto system = new System(name, binding, executionOrder);
		m_Systems.emplace(system);
	}
}

template <typename System>
void EntityRegistry::AddSystem() requires std::is_base_of_v<SystemBase, System>
{
	if constexpr (isViewSystem<System>)
	{
		using T = System::ElementType;

		TypeView<T>* view{};
		auto it = m_TypeViews.find(reflection::type_id<T>());
		if (it == m_TypeViews.end())
			view = reinterpret_cast<TypeView<T>*>(&AddView<T>());
		else
			view = reinterpret_cast<TypeView<T>*>(&*it);

		auto system = new System(view);
		m_Systems.emplace(system);

		assert(m_Systems.end() == std::find_if(m_Systems.begin(), m_Systems.end(), [&system](const std::unique_ptr<SystemBase>& sys) {return sys->GetName() == system.GetName(); }));
	}
	else if constexpr (isBindingSystem<System>)
	{
		auto types = System::GetTypes();
		TypeBinding* binding{ GetBinding(types.data(), types.size()) };

		auto system = new System(binding);
		m_Systems.emplace(system);

		assert(m_Systems.end() == std::find_if(m_Systems.begin(), m_Systems.end(), [&system](const std::unique_ptr<SystemBase>& sys) {return sys->GetName() == system.GetName(); }));
	}
}

template <typename T>
TypeView<T>& EntityRegistry::AddView()
{
	auto view = new TypeView<T>(this);
	m_TypeViews.emplace(reflection::type_id<T>(), view);

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

template <typename ... Types>
TypeBinding* EntityRegistry::AddBinding()
{
	auto typeIds = reflection::Type_ids<Types...>();
	return AddBinding(typeIds.data(), typeIds.size());
}