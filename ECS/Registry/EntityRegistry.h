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

class TypeBindingIdentifier final
{
public:

	TypeBindingIdentifier() = default;

	~TypeBindingIdentifier() = default;

	TypeBindingIdentifier(const TypeBindingIdentifier&) = delete;
	TypeBindingIdentifier& operator=(const TypeBindingIdentifier&) = delete;

	TypeBindingIdentifier(TypeBindingIdentifier&& other) noexcept = default;
	TypeBindingIdentifier& operator=(TypeBindingIdentifier&& other) noexcept = default;

	template <typename... Types>
	void Initialize(TypeBindingBase* TypeBinding)
	{
		m_TypeBinding = std::unique_ptr<TypeBindingBase>(TypeBinding);
		m_TypesAmount = sizeof...(Types);
		m_TypesHashes = std::unique_ptr<uint32_t[]>(new uint32_t[m_TypesAmount]);

		auto types = reflection::Type_ids<Types...>();
		std::memcpy(m_TypesHashes.get(), types.data(), sizeof(uint32_t) * m_TypesAmount);
	}

	template <typename... Types>
	bool Compare()
	{
		auto types = reflection::Type_ids<Types...>();
		return Compare(types.data(), types.size());
	}

	bool Compare(const uint32_t* types, const size_t size);


	bool Contains(uint32_t id);

	template <typename Type>
	bool Contains()
	{
		constexpr uint32_t Id{ reflection::type_id<Type>() };
		return Contains(Id);
	}
	
	TypeBindingBase* GetTypeBinding() const { return m_TypeBinding.get(); }
	size_t GetTypesAmount() const { return m_TypesAmount; }
	const uint32_t* GetTypeIds() const { return m_TypesHashes.get(); }

private:

	bool Assert(const uint32_t* types, const size_t size);
	
private:
	std::unique_ptr<TypeBindingBase> m_TypeBinding{};
	size_t m_TypesAmount{};
	std::unique_ptr<uint32_t[]> m_TypesHashes{};

};

/** A reference version of TypeInfoIdentifier that can be passed around and copied without having to worry about it going out of scope*/
struct TypeBindingIdentifierReference final
{
	TypeBindingIdentifierReference(const TypeBindingIdentifier& bindingId)
		: TypeBinding(bindingId.GetTypeBinding())
		, TypesAmount(bindingId.GetTypesAmount())
		, TypeIds(bindingId.GetTypeIds())
	{}

	TypeBindingBase* TypeBinding;
	size_t TypesAmount;
	const uint32_t* TypeIds;
};


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

	template <typename Type>
	void AddSystem(const std::string& name, const std::function<void(Type&)>& function, int32_t executionOrder = int32_t(ExecutionTime::Update));

	template <typename... Types>
	void AddSystem(const std::string& name, const std::function<void(Types&...)>& function, int32_t executionOrder = int32_t(ExecutionTime::Update)) requires (sizeof...(Types) >= 2);

	template <typename System>
	void AddSystem(const std::string& name, int32_t executionOrder = int32_t(ExecutionTime::Update)) requires std::is_base_of_v<SystemBase, System>;

	template <typename System>
	void AddSystem() requires std::is_base_of_v<SystemBase, System>;

	void RemoveSystem(std::string name);

	template<typename T>
	TypeView<T>& AddView();

	Entity CreateEntity();

	void RemoveEntity(const Entity& entity);
	void RemoveEntity(entityId id);

	const std::unordered_set<entityId>& GetEntities() const { return m_Entities; }

	const Entity CreateOrGetEntity(entityId id);

	template <typename... Types>
	TypeBinding<Types...>& AddBinding();

	template <typename... Types>
	[[nodiscard]] TypeBinding<Types...>& GetBinding();

	[[nodiscard]] TypeBindingBase* GetBinding(const uint32_t* types, const size_t size);

	void Update();

	void Serialize(std::ofstream& stream);

	void Deserialize(std::ifstream& stream);

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

	void RegisterBinding(const TypeBindingIdentifier& identifier);

private:

	std::unordered_set<entityId> m_Entities;

	std::unordered_map<uint32_t, std::unique_ptr<TypeViewBase>> m_TypeViews;

	std::vector<TypeBindingIdentifier> m_TypeBindings;

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
	std::vector<uint32_t> subClasses;
	size_t processedCounter{};

	for (auto& childId : TypeInformation::Data::ClassHierarchy[reflection::type_id<Type>()])
		subClasses.emplace_back(childId);

	while (subClasses.size() != processedCounter)
	{
		for (auto& childId : TypeInformation::Data::ClassHierarchy[subClasses[processedCounter]])
		{
			subClasses.emplace_back(childId);
		}
		++processedCounter;
	}

	for (auto SubClassId : subClasses)
	{
		TypeView<Type>* view{};
		auto it = m_TypeViews.find(SubClassId);
		if (it == m_TypeViews.end())
			view = reinterpret_cast<TypeView<Type>*>(&AddView<Type>());
		else
		{
			TypeViewBase* viewBase = it->second.get();
			view = reinterpret_cast<TypeView<Type>*>(viewBase);
		}

		auto typeName = TypeInformation::Data::TypeInformation[SubClassId].m_TypeName;
		auto system = new ViewSystemDynamic<Type>{ name + "_" + std::string(typeName), view, function, executionOrder};

		m_Systems.emplace(system);
	}
}

template <typename ... Types>
void EntityRegistry::AddSystem(const std::string& name, const std::function<void(Types&...)>& function, int32_t executionOrder) requires (sizeof...(Types) >= 2)
{
	// Make sure the name is not in there already
	assert(m_Systems.end() == std::find_if(m_Systems.begin(), m_Systems.end(), [name](const std::unique_ptr<SystemBase>& sys) {return sys->GetName() == name; }));

	TypeBinding<Types...>* binding{};
	try
	{
		binding = &GetBinding<Types...>();
	}
	catch (const std::runtime_error&)
	{
		binding = &AddBinding<Types...>();
	}

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
		TypeBindingBase* binding{ GetBinding(types.data(), types.size()) };
		
		auto system = new System(name, System::ReinterpretCast(binding), executionOrder);
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
		TypeBindingBase* binding{ GetBinding(types.data(), types.size()) };

		auto system = new System(System::ReinterpretCast(binding));
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

template <typename ... Types>
[[nodiscard]] TypeBinding<Types...>& EntityRegistry::GetBinding()
{
	auto types = reflection::Type_ids<Types...>();
	return reinterpret_cast<TypeBinding<Types...>&>(*GetBinding(types.data(), types.size()));
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
TypeBinding<Types...>& EntityRegistry::AddBinding()
{
	auto binding = new TypeBinding<Types...>(*this);
	
	TypeBindingIdentifier identifier{};
	identifier.Initialize<Types...>(binding);

	auto& bindingIdentifier = m_TypeBindings.emplace_back(std::move(identifier));

	RegisterBinding(bindingIdentifier);

	return *binding;
}