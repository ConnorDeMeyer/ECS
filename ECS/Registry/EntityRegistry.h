#pragma once

#include <unordered_map>
#include <memory>
#include <vector>
#include <functional>
#include <stdexcept>

#include "../TypeInformation/reflection.h"
#include "TypeView.h"
#include "TypeBinding.h"

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

private:



private:

	std::vector<entityId> m_Entities;

	std::unordered_map<uint32_t, std::unique_ptr<TypeViewBase>> m_TypeViews;

	std::vector<std::unique_ptr<TypeBindingBase>> m_TypeBinders;
	

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

	return *view;
}

template <typename ... Types>
TypeBinding<Types...>& EntityRegistry::AddBinding()
{
	auto binding = new TypeBinding<Types...>(*this);
	m_TypeBinders.emplace_back(binding);
	
	
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

	