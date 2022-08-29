#pragma once
#include <cstdint>

#include "../Entity/Entity.h"

enum class ViewDataFlag : uint8_t
{
	/**
	 * Valid: The array is ordered correctly in case a sorting function was specified
	 * Is always valid if the data array does not have to be sorted
	 */
	valid,

	/**
	 * The array has become unsorted but does not need an immediate sort of its elements.
	 * The array will be sorted on a different thread and be replaced by the newly sorted one when done sorting
	 */
	dirty,

	/**
	 * The array data is being sorted on a different thread and can be interrupted by setting the flag to dirty.
	 */
	sorting,

	/**
	 * The array has to be sorted immediately and will halt the program loop doing so.
	 */
	invalid,
};

enum class SortingProgress : uint8_t
{
	none,
	sorting,
	canceled,
	done,
	copying
};

template <typename T> class Reference;

template <typename T>
class ReferencePointer final
{
	friend class Reference<T>;

public:
	ReferencePointer(T* ptr) : m_ptr{ ptr } {}

	ReferencePointer(const ReferencePointer&)				= delete;
	ReferencePointer& operator=(const ReferencePointer&)	= delete;
	ReferencePointer(ReferencePointer&&)					= delete;
	ReferencePointer& operator=(ReferencePointer&&)			= delete;

	static ReferencePointer InvalidRef() { return nullptr; }

	bool IsValid() const { return m_ptr != nullptr; }
	size_t GetReferencesAmount() const { return m_Counter; }

	T* m_ptr{};
private:
	size_t m_Counter{};
};

template <typename T>
class Reference final
{
public:
	Reference(ReferencePointer<T>& ptr) : m_ReferencePointer{ &ptr } { ++ptr.m_Counter; }
	Reference(ReferencePointer<T>* ptr) : m_ReferencePointer{ ptr } { if (ptr) ++ptr->m_Counter; }
	~Reference() { if (m_ReferencePointer) --m_ReferencePointer->m_Counter; }

	const T* operator->() const { return m_ReferencePointer ? m_ReferencePointer->m_ptr : nullptr; }
	T* operator->() { return m_ReferencePointer ? m_ReferencePointer->m_ptr : nullptr; }
	T* get() { return m_ReferencePointer ? m_ReferencePointer->m_ptr : nullptr; }

	const ReferencePointer<T>& GetReferencePointer() const { return *m_ReferencePointer; }
	ReferencePointer<T>& GetReferencePointer() { return *m_ReferencePointer; }
	bool IsValid() const
	{
		if (m_ReferencePointer && m_ReferencePointer->m_ptr)
				return true;
		if (m_ReferencePointer && !m_ReferencePointer->m_ptr)
		{
			--m_ReferencePointer->m_Counter;
			m_ReferencePointer = nullptr;
		}
		return false;
	}

	static Reference InvalidRef() { return nullptr; }

private:
	ReferencePointer<T>* m_ReferencePointer;
};

class VoidReference final
{
public:
	VoidReference(void* ReferencePointer) : m_ReferencePointer{ ReferencePointer } {}
	VoidReference() = default;

	template <typename T>
	const ReferencePointer<T>& GetReferencePointer() const { return *static_cast<const ReferencePointer<T>*>(m_ReferencePointer); }

	template <typename T>
	Reference<T> ToReference() const
	{
		auto reference = static_cast<ReferencePointer<T>*>(m_ReferencePointer);
		return *reference;
	}

private:
	void* m_ReferencePointer{};
};

class VoidIterator final
{
	template <typename T>
	friend class VoidIteratorType;

public:
	using iterator_category		= std::random_access_iterator_tag;
	using value_type			= void*;
	using difference_type		= size_t;
	using pointer				= void*;
	using reference				= void*&;
public:
	VoidIterator(const VoidIterator&) = default;
	VoidIterator(VoidIterator&&) = default;
	VoidIterator& operator=(const VoidIterator&) = default;
	VoidIterator& operator=(VoidIterator&&) = default;
	~VoidIterator() = default;
	VoidIterator(void* ptr, size_t variableSize) : m_ptr(ptr), m_VariableSize(variableSize) {}

	template <typename T>
	VoidIterator(void* ptr) : m_ptr(ptr), m_VariableSize(sizeof(T)) {}
public:
	bool operator==(const VoidIterator& rhs) const { return m_ptr == rhs.m_ptr; }
	bool operator!=(const VoidIterator& rhs) const { return m_ptr != rhs.m_ptr; }
	bool operator<(	const VoidIterator& rhs) const { return m_ptr < rhs.m_ptr; }
	bool operator>(	const VoidIterator& rhs) const { return m_ptr > rhs.m_ptr; }
	bool operator<=(const VoidIterator& rhs) const { return m_ptr <= rhs.m_ptr; }
	bool operator>=(const VoidIterator& rhs) const { return m_ptr >= rhs.m_ptr; }

	void*& operator*() { return m_ptr; }
	void* operator*() const { return m_ptr; }
	void*& operator->() { return m_ptr; }

	VoidIterator& operator++() { m_ptr = BytePtr() + m_VariableSize; return *this; }
	VoidIterator& operator--() { m_ptr = BytePtr() - m_VariableSize; return *this; }
	VoidIterator operator++(int) { VoidIterator ph{ *this }; m_ptr = BytePtr() + m_VariableSize; return ph; }
	VoidIterator operator--(int) { VoidIterator ph{ *this }; m_ptr = BytePtr() - m_VariableSize; return ph; }

	VoidIterator& operator+=(size_t rhs) { m_ptr = BytePtr() + rhs * m_VariableSize; return *this; }
	VoidIterator& operator-=(size_t rhs) { m_ptr = BytePtr() - rhs * m_VariableSize; return *this; }

	VoidIterator operator+(size_t rhs) { return VoidIterator{ *this += rhs }; }
	VoidIterator operator-(size_t rhs) { return VoidIterator{ *this -= rhs }; }
	//VoidIterator operator+(const VoidIterator& rhs) { return VoidIterator{ m_ptr + rhs.m_ptr }; }
	//VoidIterator operator-(const VoidIterator& rhs) { return VoidIterator{ m_ptr - rhs.m_ptr }; }

	void* operator[](size_t offset) const { return static_cast<void*>(BytePtr() + offset * m_VariableSize); }
public:

	template <typename T>
	T* get() { return static_cast<T*>(m_ptr); }

private:

	uint8_t* BytePtr() const { return static_cast<uint8_t*>(m_ptr); }

private:
	void* m_ptr{};
	size_t m_VariableSize{};
};

template <typename T>
class VoidIteratorType final
{
public:
	using iterator_category = std::random_access_iterator_tag;
	using value_type = void*;
	using difference_type = size_t;
	using pointer = void*;
	using reference = void*&;
public:
	VoidIteratorType(const VoidIteratorType&) = default;
	VoidIteratorType(VoidIteratorType&&) = default;
	VoidIteratorType& operator=(const VoidIteratorType&) = default;
	VoidIteratorType& operator=(VoidIteratorType&&) = default;
	~VoidIteratorType() = default;
	VoidIteratorType(const VoidIterator& it) : m_VoidIterator(it) {}
	VoidIteratorType(void* address, size_t elementSize) : m_VoidIterator(address, elementSize) {}
	
public:
	bool operator==(const VoidIteratorType& rhs) const { return m_VoidIterator == rhs.m_VoidIterator; }
	bool operator!=(const VoidIteratorType& rhs) const { return m_VoidIterator != rhs.m_VoidIterator; }
	bool operator<( const VoidIteratorType& rhs) const { return m_VoidIterator < rhs.m_VoidIterator; }
	bool operator>( const VoidIteratorType& rhs) const { return m_VoidIterator > rhs.m_VoidIterator; }
	bool operator<=(const VoidIteratorType& rhs) const { return m_VoidIterator <= rhs.m_VoidIterator; }
	bool operator>=(const VoidIteratorType& rhs) const { return m_VoidIterator >= rhs.m_VoidIterator; }

	T& operator*() { return *static_cast<T*>(m_VoidIterator.m_ptr); }
	const T& operator*() const { return *static_cast<T*>(m_VoidIterator.m_ptr); }
	T& operator->() { return *static_cast<T*>(m_VoidIterator.m_ptr); }

	VoidIteratorType& operator++() { ++m_VoidIterator; return *this; }
	VoidIteratorType& operator--() { --m_VoidIterator; return *this; }
	VoidIteratorType operator++(int) { return m_VoidIterator++; }
	VoidIteratorType operator--(int) { return m_VoidIterator--; }

	VoidIteratorType& operator+=(size_t rhs) { return m_VoidIterator+=rhs; }
	VoidIteratorType& operator-=(size_t rhs) { return m_VoidIterator+=rhs; }

	VoidIteratorType operator+(size_t rhs) { return m_VoidIterator + rhs; }
	VoidIteratorType operator-(size_t rhs) { return m_VoidIterator - rhs; }
	//VoidIterator operator+(const VoidIterator& rhs) { return VoidIterator{ m_ptr + rhs.m_ptr }; }
	//VoidIterator operator-(const VoidIterator& rhs) { return VoidIterator{ m_ptr - rhs.m_ptr }; }

	T* operator[](size_t offset) const { return m_VoidIterator[offset]; }

private:
	VoidIterator m_VoidIterator;
};

class EntityRegistry;

class TypeViewBase
{
	friend class EntityRegistry;

public:

	TypeViewBase(EntityRegistry* registry) : m_pRegistry(registry) { }
	virtual ~TypeViewBase() = default;

	TypeViewBase(const TypeViewBase&) = delete;
	TypeViewBase(TypeViewBase&&) = delete;
	TypeViewBase& operator=(const TypeViewBase&) = delete;
	TypeViewBase& operator=(TypeViewBase&&) = delete;

	EntityRegistry* GetRegistry() const { return m_pRegistry; }

public:

	virtual void Update() = 0;

	const std::vector<entityId>& GetRegisteredEntities() const { return m_DataEntityMap; }

	virtual bool Contains(entityId id) = 0;

	virtual entityId GetEntityId(const void* elementAddress) = 0;

	/** Returns a void pointer to a reference pointer*/
	virtual VoidReference GetVoidReference(entityId id) const = 0;

	virtual VoidIterator GetVoidIterator() = 0;
	virtual VoidIterator GetVoidIteratorEnd() = 0;

	virtual uint32_t GetTypeId() const = 0;

	ViewDataFlag GetDataFlag() const { return m_DataFlag; }

	uint16_t GetDataFlagId() const { return m_DataFlagId; }

	virtual void SortData(volatile SortingProgress& sortingProgress, const volatile bool& quit) = 0;

	virtual void SerializeView(std::ofstream& stream) = 0;

	virtual void DeserializeView(std::ifstream& stream) = 0;

	virtual void Remove(entityId id) = 0;

	size_t GetSize() const
	{
		const size_t size = m_DataEntityMap.size();
		for (size_t i{}; i < size; ++i)
		{
			if (m_DataEntityMap[size - 1 - i] != Entity::InvalidId)
				return size - i;
		}
		return 0;
	}

public:

	std::vector<std::function<void(TypeViewBase*, entityId)>> OnElementAdd;
	
	std::vector<std::function<void(TypeViewBase*, entityId)>> OnElementRemove;

protected:

	std::vector<entityId> m_DataEntityMap{ 16,Entity::InvalidId };

	EntityRegistry* m_pRegistry{};

	ViewDataFlag m_DataFlag{ ViewDataFlag::valid };
	uint16_t m_DataFlagId{ 42 };

};