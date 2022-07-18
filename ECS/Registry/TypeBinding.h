#pragma once
#include <tuple>
#include <array>
#include <iterator>

#include "TypeView.h"
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
			return size_t( -1 );
	}

public:

	/** Wrapper class for the std::array<void*, sizeof...(Types)>*/
	class BindingView : TypeBindingBase
	{
	public:
		BindingView() = default;
		BindingView(const std::array<VoidReference, sizeof...(Types)>& data) : m_Data{data} {}
		BindingView(std::array<VoidReference, sizeof...(Types)>&& data) : m_Data{data} {}

		BindingView(const BindingView&)					= default;
		BindingView(BindingView&&) noexcept				= default;
		BindingView& operator=(const BindingView&)		= default;
		BindingView& operator=(BindingView&&) noexcept	= default;
		virtual ~BindingView()							= default;

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

	private:
		std::array<VoidReference, sizeof...(Types)> m_Data;
	};

public:

	TypeBinding(EntityRegistry& registry)
		: m_Registry(registry)
	{
		FillTypeIds<Types...>(0);
	}

	TypeBinding(const TypeBinding&)				= delete;
	TypeBinding(TypeBinding&&)					= delete;
	TypeBinding& operator=(const TypeBinding&)	= delete;
	TypeBinding& operator=(TypeBinding&&)		= delete;
	virtual ~TypeBinding()						= default;

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

private:

	template <typename T, typename... Ts>
	void FillTypeIds(size_t index)
	{
		m_typeIds[index] = reflection::type_id<T>();
		if constexpr (sizeof...(Ts) != 0)
			return FillTypeIds<Ts...>(++index);
	}

	/** Helper function for GetTypeId */
	template <typename T, typename... Ts>
	constexpr uint32_t GetTypeCounter(size_t count) const
	{
		if constexpr (sizeof...(Ts) != 0)
			return (count) ? GetTypeCounter<Ts...>(--count) : reflection::type_id<T>();
		else
			return (count) ? 0 : reflection::type_id<T>();
	}

	template <typename T>
	constexpr size_t GetTypePos() const
	{
		return GetTypePosCompare<T, Types...>(0);
	}

private:

	std::vector<BindingView> m_Data;
	
	std::array<uint32_t, sizeof...(Types)> m_typeIds;

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