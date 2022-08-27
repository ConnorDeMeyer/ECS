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

using GameComponentClass = class GameComponent;

class TypeBindingIdentifier final
{
public:

	TypeBindingIdentifier() = default;

	~TypeBindingIdentifier();

	TypeBindingIdentifier(const TypeBindingIdentifier&) = delete;
	TypeBindingIdentifier& operator=(const TypeBindingIdentifier&) = delete;

	TypeBindingIdentifier(TypeBindingIdentifier&& other) noexcept;
	TypeBindingIdentifier& operator=(TypeBindingIdentifier&& other) noexcept;

	template <typename... Types>
	void Initialize(TypeBindingBase* TypeBinding)
	{
		m_TypeBinding = TypeBinding;
		m_TypesAmount = sizeof...(Types);
		m_TypesHashes = new uint32_t[m_TypesAmount];

		auto types = reflection::Type_ids<Types...>();
		std::memcpy(m_TypesHashes, types.data(), sizeof(uint32_t) * m_TypesAmount);
	}

	template <typename... Types>
	bool Compare()
	{
		auto types = reflection::Type_ids<Types...>();
		return Compare(types.data(), types.size());
	}

	bool Compare(const uint32_t* types, const size_t size);

	TypeBindingBase* GetTypeBinding() const { return m_TypeBinding; }

	bool Contains(uint32_t id);

	template <typename Type>
	bool Contains()
	{
		constexpr uint32_t Id{ reflection::type_id<Type>() };
		return Contains(Id);
	}

private:

	bool Assert(const uint32_t* types, const size_t size);
	
private:
	TypeBindingBase*	m_TypeBinding{};
	size_t				m_TypesAmount{};
	uint32_t*			m_TypesHashes{};

};


class EntityRegistry final
{
public:

	EntityRegistry() = default;
	~EntityRegistry() = default;

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

	void ForEachGameComponent(const std::function<void(GameComponentClass*)>& function);

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
	inline static ThreadPool SortingThreadPool;
	std::array<volatile SortingProgress, ThreadPool::MaxThreads> m_SortingProgress;

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

template <typename T>
TypeView<T>& EntityRegistry::AddView()
{
	auto view = new TypeView<T>(this);
	m_TypeViews.emplace(reflection::type_id<T>(), view);

	if constexpr (std::is_base_of_v<GameComponentClass, T>)
	{
		m_GameComponentTypeViews.push_back(view);
	}

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
	{
		TypeBindingIdentifier identifier{};
		identifier.Initialize<Types...>(binding);

		m_TypeBindings.emplace_back(std::move(identifier));
	}

	int32_t firstTypeId = binding->GetTypeId(0);
	auto& view = m_TypeViews[firstTypeId];
	auto& entities = view->GetRegisteredEntities();

	for (auto entity : entities)
	{
		bool presentInAll{ true };

		for (size_t i{ 1 }; i < binding->AmountOfTypes(); ++i)
		{
			auto typeId = binding->GetTypeId(i);
			if (!m_TypeViews[typeId]->Contains(entity))
			{
				presentInAll = false;
				break;
			}
		}

		if (presentInAll)
		{
			binding->m_Data.emplace_back();
			
			size_t offset = &binding->m_Data.back() - binding->m_Data.data();
			binding->m_ContainedEntities.emplace(entity, offset);

			for (size_t i{ }; i < binding->AmountOfTypes(); ++i)
			{
				auto typeId = binding->GetTypeId(i);
				auto& viewI = m_TypeViews[typeId];
				auto voidReference = viewI->GetVoidReference(entity);
				binding->m_Data.back()[i] = voidReference;
			}
		}
	}

	for (size_t i{ }; i < binding->AmountOfTypes(); ++i)
	{
		uint32_t typeId{ binding->GetTypeId(i) };
		auto& typeViews{ m_TypeViews };
		auto& viewI = m_TypeViews[typeId];

		viewI->OnElementAdd.emplace_back([binding, &typeViews](TypeViewBase*, entityId id)
			{
				bool presentInAll{};

				for (size_t i{ }; i < binding->AmountOfTypes(); ++i)
				{
					auto typeId = binding->GetTypeId(i);
					if (!typeViews[typeId]->Contains(id))
					{
						return;
					}

					presentInAll = true;
				}

				if (presentInAll)
				{
					binding->m_Data.emplace_back();

					size_t offset = &binding->m_Data.back() - binding->m_Data.data();
					binding->m_ContainedEntities.emplace(id, offset);

					for (size_t i{ }; i < binding->AmountOfTypes(); ++i)
					{
						auto typeId = binding->GetTypeId(i);
						auto& viewI = typeViews[typeId];
						auto voidReference = viewI->GetVoidReference(id);
						binding->m_Data.back()[i] = voidReference;
					}
				}

			});

		viewI->OnElementRemove.emplace_back([binding, &typeViews](TypeViewBase*, entityId id)
			{
				auto it = binding->m_ContainedEntities.find(id);
				if (it != binding->m_ContainedEntities.end())
				{
					// Get the position of the binding
					size_t offset = it->second;

					// If the element is already at the end
					if (offset == binding->m_Data.size() - 1)
					{
						binding->m_Data.pop_back();
					}
					else
					{
						// use swap remove to remove the binding
						// we need will also need to update the position in the m_ContainedEntities map
						const auto elementAddress = binding->m_Data.back()[0].GetReferencePointer<void>().m_ptr;

						const uint32_t typeId = binding->m_typeIds[0];
						const entityId otherId = typeViews.find(typeId)->second->GetEntityId(elementAddress);

						// update the position of the swapped element
						assert(binding->m_ContainedEntities.contains(otherId));
						binding->m_ContainedEntities[otherId] = offset;

						// swap remove
						binding->m_Data[offset] = std::move(binding->m_Data.back());
						binding->m_Data.pop_back();
					}

					binding->m_ContainedEntities.erase(it);
				}
			});
	}

	return *binding;
}