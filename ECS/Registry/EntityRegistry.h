#pragma once

#include <unordered_map>
#include <memory>
#include <vector>
#include <functional>
#include <stdexcept>

#include "../TypeInformation/reflection.h"
#include "TypeView.h"
#include "TypeBinding.h"

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

		FillTypeHashes<Types...>(0);
	}

	template <typename... Types>
	bool Compare()
	{
		return CompareAllTypes<Types...>();
	}

	TypeBindingBase* GetTypeBinding() const { return m_TypeBinding; }

	template <typename Type>
	bool Contains()
	{
		constexpr uint32_t Id{ reflection::type_id<Type>() };
		for (uint32_t i{}; i < m_TypesAmount; ++i)
		{
			if (m_TypesHashes[i] == Id)
				return true;
		}
		return false;
	}

private:

	template <typename Type, typename... Types>
	void FillTypeHashes(uint32_t counter)
	{
		if constexpr (sizeof...(Types) != 0)
		{
			m_TypesHashes[counter] = reflection::type_id<Type>();
			FillTypeHashes<Types...>(++counter);
		}
	}

	template <typename Type, typename... Types>
	bool CompareAllTypes()
	{
		if constexpr (sizeof...(Types) != 0)
		{
			return Contains<Type>()
				|| CompareAllTypes<Types...>();
		}
		else
		{
			return Contains<Type>();
		}
	}

private:
	TypeBindingBase*	m_TypeBinding{};
	uint32_t			m_TypesAmount{};
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

	template<typename T>
	TypeView<T>& GetView();

	template<typename T>
	TypeView<T>& AddView();

	entityId CreateEntity();

	const std::vector<entityId>& GetEntities() const { return m_Entities; }

	template <typename... Types>
	TypeBinding<Types...>& AddBinding();

	template <typename... Types>
	TypeBinding<Types...>& GetBinding();

	//const std::vector<TypeView<GameComponentClass>*>& GetGameComponentViews() const { return m_GameComponentTypeViews; }

	void ForEachGameComponent(const std::function<void(GameComponentClass*)>& function);

private:



private:

	std::vector<entityId> m_Entities;

	std::unordered_map<uint32_t, std::unique_ptr<TypeViewBase>> m_TypeViews;

	std::vector<TypeBindingIdentifier> m_TypeBindings;

	std::vector<TypeViewBase*> m_GameComponentTypeViews;

};

template <typename T>
TypeView<T>& EntityRegistry::GetView()
{
	auto it = m_TypeViews.find(reflection::type_id<T>());
	if (it != m_TypeViews.end())
	{
		return *reinterpret_cast<TypeView<T>*>(it->second);
	}

	throw std::runtime_error("View was not found inside of registry");
}

template <typename T>
TypeView<T>& EntityRegistry::AddView()
{
	auto view = new TypeView<T>();
	m_TypeViews.emplace(reflection::type_id<T>(), view);

	if constexpr (std::is_base_of_v<GameComponentClass, T>)
	{
		m_GameComponentTypeViews.push_back(view);
	}

	return *view;
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

		for (size_t i{1}; i < binding->AmountOfTypes(); ++i)
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
				//int32_t firstTypeId = binding->GetTypeId(0);
				//auto& view = typeViews[firstTypeId];
				
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

					for (size_t i{ }; i < binding->AmountOfTypes(); ++i)
					{
						auto typeId = binding->GetTypeId(i);
						auto& viewI = typeViews[typeId];
						auto voidReference = viewI->GetVoidReference(id);
						binding->m_Data.back()[i] = voidReference;
					}
				}

			});
	}
	
	return *binding;
}

template <typename ... Types>
TypeBinding<Types...>& EntityRegistry::GetBinding()
{
	for (auto& binding : m_TypeBindings)
	{
		if (binding.Compare<Types...>())
		{
			return *reinterpret_cast<TypeBinding<Types...>*>(binding.GetTypeBinding());
		}
	}

	throw std::runtime_error("No Binding inside of this type inside of registry");
}

	