#pragma once
#include <cassert>
#include <memory>
#include <tuple>

#include "TypeViewBase.h"
#include "../Entity/Entity.h"
#include "../TypeInformation/TypeInformation.h"

class EntityRegistry;

class TypeBinding final
{
public:

	template <typename... Types>
	TypeBinding(EntityRegistry* pRegistry)
		: m_pRegistry(pRegistry)
		, m_pTypes(std::make_unique<uint32_t[]>(sizeof...(Types)))
		, m_TypesAmount(sizeof...(Types))
	{
		static_assert(sizeof...(Types) >= 2);

		auto types = reflection::Type_ids<Types...>();
		for (size_t i{}; i < types.size(); ++i)
		{
			m_pTypes[i] = types[i];
		}
		Initialize();
	}

	TypeBinding(EntityRegistry* pRegistry, const uint32_t* types, size_t amount)
		: m_pRegistry(pRegistry)
		, m_pTypes(std::make_unique<uint32_t[]>(amount))
		, m_TypesAmount(amount)
	{
		assert(amount >= 2);

		for (size_t i{}; i < amount; ++i)
		{
			m_pTypes[i] = types[i];
		}
		Initialize();
	}

	template <typename T>
	size_t GetTypePos() const
	{
		return GetTypePos(reflection::type_id<T>());
	}

	size_t GetTypePos(uint32_t typeId) const
	{
		for (size_t i{}; i < m_TypesAmount; ++i)
			if (m_pTypes[i] == typeId)
				return i;
		return std::numeric_limits<uint32_t>::max();
	}

	size_t GetEntityPos(entityId id) const
	{
		assert(m_ContainedEntities.contains(id));
		return m_ContainedEntities.find(id)->first;
	}

	const VoidReference& Get(size_t typePos, size_t elementPos) const
	{
		return m_Data[elementPos * m_TypesAmount + typePos];
	}

	const VoidReference& GetWithTypeId(uint32_t typeId, size_t elementPos) const
	{
		return Get(GetTypePos(typeId), elementPos);
	}

	const VoidReference& GetEntity(size_t typePos, entityId id) const
	{
		return Get(typePos, GetEntityPos(id));
	}

	const VoidReference& GetEntityWithTypeId(uint32_t typeId, entityId id) const
	{
		return GetWithTypeId(typeId, GetEntityPos(id));
	}

	VoidReference& Get(size_t typePos, size_t elementPos)
	{
		return m_Data[elementPos * m_TypesAmount + typePos];
	}

	VoidReference& GetWithTypeId(uint32_t typeId, size_t elementPos)
	{
		return Get(GetTypePos(typeId), elementPos);
	}

	VoidReference& GetEntity(size_t typePos, entityId id)
	{
		return Get(typePos, GetEntityPos(id));
	}

	VoidReference& GetEntityWithTypeId(uint32_t typeId, entityId id)
	{
		return GetWithTypeId(typeId, GetEntityPos(id));
	}

	template <typename T>
	Reference<T> Get(size_t typePos, size_t elementPos) const
	{
		return Get(typePos, elementPos).ToReference<T>();
	}

	template <typename T>
	Reference<T> Get(size_t elementPos) const
	{
		return Get(reflection::type_id<T>(), elementPos).ToReference<T>();
	}

	template <typename T>
	Reference<T> GetEntity(size_t typePos, entityId id) const
	{
		return GetEntity(typePos, id).ToReference<T>();
	}

	template <typename T>
	Reference<T> GetEntityWithTypeId(entityId id) const
	{
		return GetEntity(reflection::type_id<T>(), id).ToReference<T>();
	}

	const uint32_t* GetTypeIds(size_t& size) const
	{
		size = m_TypesAmount;
		return m_pTypes.get();
	}

	const uint32_t* GetTypeIds() const
	{
		return m_pTypes.get();
	}

	size_t GetTypeAmount() const
	{
		return m_TypesAmount;
	}

	template <typename... Types>
	bool Assert() const;

	bool AssertTypeCombination(const uint32_t* types, size_t size) const;

	EntityRegistry* GetRegistry() { return m_pRegistry; }
	const EntityRegistry* GetRegistry() const { return m_pRegistry; }

	template <typename... Types>
	void ApplyFunction(const std::function<void(Types&...)>& function, size_t pos);

	template <typename... Types>
	void ApplyFunctionOnEntity(const std::function<void(Types&...)>& function, entityId id);

	template <typename... Types>
	void ApplyFunctionOnAll(const std::function<void(Types&...)>& function);

	bool Compare(const uint32_t* types, size_t size) const;

	bool Contains(uint32_t typeId) const;

	void PrintTypes(std::ostream& stream);

	size_t GetSize() const { return m_ContainedEntities.size(); }

private:

	void Initialize();
	void push_back();
	void pop_back();
	void move(size_t source, size_t target);
	size_t back();
	void SwapRemove(size_t pos);

private:
	EntityRegistry* m_pRegistry;

	std::vector<VoidReference> m_Data;
	const std::unique_ptr<uint32_t[]> m_pTypes;
	const size_t m_TypesAmount{};

	std::unordered_map<entityId, size_t> m_ContainedEntities;

};

template <typename ... Types>
bool TypeBinding::Assert() const
{
	if (m_TypesAmount != sizeof...(Types))
		return false;

	auto typeIds = reflection::Type_ids<Types...>();
	for (size_t i{}; i < m_TypesAmount; ++i)
	{
		if (typeIds[i] != m_pTypes[i] && !TypeInformation::IsSubClass(m_pTypes[i], typeIds[i]))
			return false;
	}
	return true;
}

template <typename ... Types>
void TypeBinding::ApplyFunctionOnEntity(const std::function<void(Types&...)>& function, entityId id) 
{
	ApplyFunction(function, GetEntityPos(id));
}

template <typename ... Types>
void TypeBinding::ApplyFunctionOnAll(const std::function<void(Types&...)>& function) 
{
	const size_t size{ m_Data.size() / m_TypesAmount };
	for (size_t i{}; i < size; ++i)
	{
		ApplyFunction(function, i);
	}
}

template <typename ... Types>
void TypeBinding::ApplyFunction(const std::function<void(Types&...)>& function, size_t pos) 
{
	assert(sizeof...(Types) == m_TypesAmount);
	assert(Assert<Types...>());

	if constexpr (sizeof...(Types) == 2)
	{
		function(
			*Get<std::tuple_element_t<0, std::tuple<Types...>>>(0, pos).get(),
			*Get<std::tuple_element_t<1, std::tuple<Types...>>>(1, pos).get());
	}
	else if constexpr (sizeof...(Types) == 3)
	{
		function(
			*Get<std::tuple_element_t<0, std::tuple<Types...>>>(0, pos).get(),
			*Get<std::tuple_element_t<1, std::tuple<Types...>>>(1, pos).get(),
			*Get<std::tuple_element_t<2, std::tuple<Types...>>>(2, pos).get());
	}
	else if constexpr (sizeof...(Types) == 4)
	{
		function(
			*Get<std::tuple_element_t<0, std::tuple<Types...>>>(0, pos).get(),
			*Get<std::tuple_element_t<1, std::tuple<Types...>>>(1, pos).get(),
			*Get<std::tuple_element_t<2, std::tuple<Types...>>>(2, pos).get(),
			*Get<std::tuple_element_t<3, std::tuple<Types...>>>(3, pos).get());
	}
	else if constexpr (sizeof...(Types) == 5)
	{
		function(
			*Get<std::tuple_element_t<0, std::tuple<Types...>>>(0, pos).get(),
			*Get<std::tuple_element_t<1, std::tuple<Types...>>>(1, pos).get(),
			*Get<std::tuple_element_t<2, std::tuple<Types...>>>(2, pos).get(),
			*Get<std::tuple_element_t<3, std::tuple<Types...>>>(3, pos).get(),
			*Get<std::tuple_element_t<4, std::tuple<Types...>>>(4, pos).get());
	}
	else if constexpr (sizeof...(Types) == 6)
	{
		function(
			*Get<std::tuple_element_t<0, std::tuple<Types...>>>(0, pos).get(),
			*Get<std::tuple_element_t<1, std::tuple<Types...>>>(1, pos).get(),
			*Get<std::tuple_element_t<2, std::tuple<Types...>>>(2, pos).get(),
			*Get<std::tuple_element_t<3, std::tuple<Types...>>>(3, pos).get(),
			*Get<std::tuple_element_t<4, std::tuple<Types...>>>(4, pos).get(),
			*Get<std::tuple_element_t<5, std::tuple<Types...>>>(5, pos).get());
	}
	static_assert(sizeof...(Types) <= 6, "Please expand this sequence");
}

