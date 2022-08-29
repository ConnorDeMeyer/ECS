#pragma once
#include <tuple>
#include <array>
#include <cassert>
#include <iterator>
#include <unordered_map>

#include "TypeViewBase.h"
#include "../TypeInformation/reflection.h"
#include "TypeBindingBase.h"

class EntityRegistry;

template<typename... Types>
class TypeBinding final : public TypeBindingBase
{
	static_assert(sizeof...(Types) >= 2);

	friend class EntityRegistry;

private:

	/** Function used to get the position of the Type inside of Types...*/
	template <typename CompareT, typename T, typename... Ts>
	static constexpr size_t GetTypePosCompare(size_t counter = 0)
	{
		if (reflection::type_id<T>() == reflection::type_id<CompareT>())
			return counter;

		if constexpr (sizeof...(Ts) != 0)
			return GetTypePosCompare<CompareT, Ts...>(++counter);
		else
			return size_t(-1);
	}

public:

	/** Wrapper class for the std::array<void*, sizeof...(Types)>*/
	class BindingView final
	{
	public:
		BindingView() = default;
		BindingView(const std::array<VoidReference, sizeof...(Types)>& data) : m_Data{ data } {}
		BindingView(std::array<VoidReference, sizeof...(Types)>&& data) : m_Data{ data } {}

		BindingView(const BindingView&) = default;
		BindingView(BindingView&&) noexcept = default;
		BindingView& operator=(const BindingView&) = default;
		BindingView& operator=(BindingView&&) noexcept = default;
		virtual ~BindingView() = default;

	public:

		template <typename T>
		Reference<T> Get() const
		{
			constexpr size_t pos{ GetTypePosCompare<T, Types...>() };
			assert(pos < sizeof...(Types));
			auto& voidRef = m_Data[pos];
			auto ref = voidRef.ToReference<T>();
			return ref;
		}

		VoidReference Get(size_t index) const { return m_Data[index]; }

		VoidReference& Get(size_t index) { return m_Data[index]; }

		VoidReference operator[](size_t index) const { return m_Data[index]; }

		VoidReference& operator[](size_t index) { return m_Data[index]; }

		void ApplyFunction(const std::function<void(Types&...)>& function)
		{
			if constexpr (sizeof...(Types) == 2)
			{
				function(*Get<std::tuple_element_t<0, std::tuple<Types...>>>().get(),
					*Get<std::tuple_element_t<1, std::tuple<Types...>>>().get());
			}
			else if constexpr (sizeof...(Types) == 3)
			{
				function(*Get<std::tuple_element_t<0, std::tuple<Types...>>>().get(),
					*Get<std::tuple_element_t<1, std::tuple<Types...>>>().get(),
					*Get<std::tuple_element_t<2, std::tuple<Types...>>>().get());
			}
			else if constexpr (sizeof...(Types) == 4)
			{
				function(*Get<std::tuple_element_t<0, std::tuple<Types...>>>().get(),
					*Get<std::tuple_element_t<1, std::tuple<Types...>>>().get(),
					*Get<std::tuple_element_t<2, std::tuple<Types...>>>().get(),
					*Get<std::tuple_element_t<3, std::tuple<Types...>>>().get());
			}
			else if constexpr (sizeof...(Types) == 5)
			{
				function(*Get<std::tuple_element_t<0, std::tuple<Types...>>>().get(),
					*Get<std::tuple_element_t<1, std::tuple<Types...>>>().get(),
					*Get<std::tuple_element_t<2, std::tuple<Types...>>>().get(),
					*Get<std::tuple_element_t<3, std::tuple<Types...>>>().get(),
					*Get<std::tuple_element_t<4, std::tuple<Types...>>>().get());
			}
			else if constexpr (sizeof...(Types) == 6)
			{
				function(*Get<std::tuple_element_t<0, std::tuple<Types...>>>().get(),
					*Get<std::tuple_element_t<1, std::tuple<Types...>>>().get(),
					*Get<std::tuple_element_t<2, std::tuple<Types...>>>().get(),
					*Get<std::tuple_element_t<3, std::tuple<Types...>>>().get(),
					*Get<std::tuple_element_t<4, std::tuple<Types...>>>().get(),
					*Get<std::tuple_element_t<5, std::tuple<Types...>>>().get());
			}
			static_assert(sizeof...(Types) <= 6, "Please expand this sequence");
		}

	private:
		std::array<VoidReference, sizeof...(Types)> m_Data;
	};

public:

	TypeBinding(EntityRegistry& registry)
		: m_Registry(registry)
	{
		m_typeIds = reflection::Type_ids<Types...>();
	}

	TypeBinding(const TypeBinding&) = delete;
	TypeBinding(TypeBinding&&) = delete;
	TypeBinding& operator=(const TypeBinding&) = delete;
	TypeBinding& operator=(TypeBinding&&) = delete;
	~TypeBinding() override = default;

public:

	constexpr uint32_t GetTypeId(size_t index)
	{
		return m_typeIds[index];
	}

	template <typename T>
	T* Get(size_t index)
	{
		return m_Data[index].Get<T>();
	}

	constexpr size_t AmountOfTypes() const
	{
		return sizeof...(Types);
	}

	auto begin() { return m_Data.begin(); }
	auto end() { return m_Data.end(); }

protected:
	void AddEmptyData(size_t& outPos) override
	{
		m_Data.emplace_back();
		outPos = m_Data.size() - 1;
	}

	void RegisterEntity(entityId entity, size_t offset) override
	{
		m_ContainedEntities.emplace(entity, offset);
	}

	void SetVoidReference(const VoidReference& ref, size_t bindingView, size_t viewPos) override
	{
		m_Data[bindingView][viewPos] = ref;
	}

	void RemoveId(entityId id) override
	{
		m_ContainedEntities.erase(id);
	}

	void SwapRemove(size_t offset, std::unordered_map<uint32_t, std::unique_ptr<TypeViewBase>>& typeViews) override
	{
		// If the element is already at the end
		if (offset == m_Data.size() - 1)
		{
			m_Data.pop_back();
		}
		else
		{
			// use swap remove to remove the binding
			// we need will also need to update the position in the m_ContainedEntities map
			const auto elementAddress = m_Data.back()[0].GetReferencePointer<void>().m_ptr;

			const uint32_t typeId = m_typeIds[0];
			const entityId otherId = typeViews.find(typeId)->second->GetEntityId(elementAddress);

			// update the position of the swapped element
			assert(m_ContainedEntities.contains(otherId));
			m_ContainedEntities[otherId] = offset;

			// swap remove
			m_Data[offset] = std::move(m_Data.back());
			m_Data.pop_back();
		}
	}

	bool Contains(entityId id) const override { return m_ContainedEntities.contains(id); }
	bool Contains(entityId id, size_t& offset) const override
	{
		auto it = m_ContainedEntities.find(id);
		if (it != m_ContainedEntities.end())
		{
			offset = it->second;
			return true;
		}
		return false;
	}

private:


private:

	std::vector<BindingView> m_Data;
	
	std::array<uint32_t, sizeof...(Types)> m_typeIds;

	// entity and offset from m_typeIds.data()
	std::unordered_map<entityId, size_t> m_ContainedEntities;

	EntityRegistry& m_Registry;

};



/** Iterator for iterating over the elements of the binding*/
	//class BindingIterator final : public std::iterator<
	//	std::random_access_iterator_tag,
	//	std::array<void*, sizeof...(Types)>,
	//	size_t,
	//	std::array<void*, sizeof...(Types)>*,
	//	std::array<void*, sizeof...(Types)>&
	//>
	//{
	//public:
	//	using element = std::array<void*, sizeof...(Types)>;
	//public:
	//	BindingIterator() = default;
	//	BindingIterator(const BindingIterator&) = default;
	//	BindingIterator(BindingIterator&&) = default;
	//	BindingIterator& operator=(const BindingIterator&) = default;
	//	BindingIterator& operator=(BindingIterator&&) = default;
	//	~BindingIterator() = default;
	//	BindingIterator(element* ptr) : m_ptr(ptr) {}
	//public:
	//	bool operator==(const BindingIterator& rhs) const { return m_ptr == rhs.m_ptr; }
	//	bool operator!=(const BindingIterator& rhs) const { return m_ptr != rhs.m_ptr; }
	//	bool operator<(const BindingIterator& rhs) const { return m_ptr < rhs.m_ptr; }
	//	bool operator>(const BindingIterator& rhs) const { return m_ptr > rhs.m_ptr; }
	//	bool operator<=(const BindingIterator& rhs) const { return m_ptr <= rhs.m_ptr; }
	//	bool operator>=(const BindingIterator& rhs) const { return m_ptr >= rhs.m_ptr; }

	//	element& operator*() { return *m_ptr; }
	//	element operator*() const { return *m_ptr; }
	//	element& operator->() { return m_ptr; }

	//	BindingIterator& operator++() { ++m_ptr; return *this; }
	//	BindingIterator& operator--() { --m_ptr; return *this; }
	//	BindingIterator operator++(int) { BindingIterator ph{ *this }; ++m_ptr; return ph; }
	//	BindingIterator operator--(int) { BindingIterator ph{ *this }; --m_ptr; return ph; }

	//	BindingIterator& operator+=(size_t rhs) { m_ptr += rhs; return *this; }
	//	BindingIterator& operator-=(size_t rhs) { m_ptr -= rhs; return *this; }

	//	BindingIterator operator+(size_t rhs) { return BindingIterator{ m_ptr + rhs }; }
	//	BindingIterator operator-(size_t rhs) { return BindingIterator{ m_ptr - rhs }; }
	//	BindingIterator operator+(const BindingIterator& rhs) { return BindingIterator{ m_ptr + rhs.m_ptr }; }
	//	BindingIterator operator-(const BindingIterator& rhs) { return BindingIterator{ m_ptr - rhs.m_ptr }; }

	//	element& operator[](size_t offset) { return m_ptr[offset]; }
	//public:

	//	template <typename T>
	//	T* Get() { return reinterpret_cast<T*>(m_ptr[GetTypePosCompare<T, Types...>()]); }

	//private:
	//	element* m_ptr{};
	//};