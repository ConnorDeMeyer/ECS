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

template <typename T>
class ReferencePointer final
{
public:
	ReferencePointer(T* ptr) : m_ptr{ ptr } {}

	ReferencePointer(const ReferencePointer&)				= delete;
	ReferencePointer& operator=(const ReferencePointer&)	= delete;
	ReferencePointer(ReferencePointer&&)					= delete;
	ReferencePointer& operator=(ReferencePointer&&)			= delete;

	T* m_ptr{};
};

template <typename T>
class Reference final
{
public:
	Reference(const ReferencePointer<T>& ptr) : m_ReferencePointer{ ptr } {}
	T& operator*() { return *m_ReferencePointer.m_ptr; }
	const T& operator*() const { return *m_ReferencePointer.m_ptr; }
	const T* operator->() const { return m_ReferencePointer.m_ptr; }
	T* operator->() { return m_ReferencePointer.m_ptr; }
	const ReferencePointer<T>& GetReferencePointer() const { return m_ReferencePointer; }
private:
	const ReferencePointer<T>& m_ReferencePointer;
};

class VoidReference final
{
public:
	VoidReference(const void* ReferencePointer) : m_ReferencePointer{ ReferencePointer } {}
	VoidReference() = default;

	template <typename T>
	const ReferencePointer<T>& GetReferencePointer() const { return *static_cast<const ReferencePointer<T>*>(m_ReferencePointer); }

	template <typename T>
	Reference<T> ToReference() const
	{
		auto reference = static_cast<const ReferencePointer<T>*>(m_ReferencePointer);
		return *reference;
	}

private:
	const void* m_ReferencePointer{};
};

class VoidIterator final
{
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

class TypeViewBase
{
public:

	TypeViewBase() = default;
	virtual ~TypeViewBase() = default;

	TypeViewBase(const TypeViewBase&) = delete;
	TypeViewBase(TypeViewBase&&) = delete;
	TypeViewBase& operator=(const TypeViewBase&) = delete;
	TypeViewBase& operator=(TypeViewBase&&) = delete;

public:

	virtual const std::vector<entityId>& GetRegisteredEntities() const = 0;

	virtual bool Contains(entityId id) = 0;

	/** Returns a void pointer to a reference pointer*/
	virtual VoidReference GetVoidReference(entityId id) const = 0;

	virtual VoidIterator GetVoidIterator() = 0;
	virtual VoidIterator GetVoidIteratorEnd() = 0;

public:

	std::vector<std::function<void(TypeViewBase*, entityId)>> OnElementAdd;
	
	std::vector<std::function<void(TypeViewBase*, entityId)>> OnElementRemove;

};