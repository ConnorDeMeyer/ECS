#pragma once
#include <cstdint>
#include <iterator>

/**
 * Iterator that iterates over elements without knowing the type of the elements it is iterating over.
 * It only needs to know the size of the element so it can increase its internal void pointer by that amount of bytes.
 * Only works with data that is adjacent to each other like arrays and vectors.
 */
class VoidIterator final
{
	template <typename T>
	friend class VoidIteratorType;

public:
	using iterator_category = std::random_access_iterator_tag;
	using value_type = void*;
	using difference_type = size_t;
	using pointer = void*;
	using reference = void*&;
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
	bool operator<(const VoidIterator& rhs) const { return m_ptr < rhs.m_ptr; }
	bool operator>(const VoidIterator& rhs) const { return m_ptr > rhs.m_ptr; }
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

/**
 * Void iterator but returns a Type element instead of a void pointer.
 * It wont increase the underlying pointer by the size of the Type but will still use the size given in the voidIterator
 * This is handy for iterating over Inherited classes while still using systems for the base classes giving the ability to use a polymorphism.
 */
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
	bool operator<(const VoidIteratorType& rhs) const { return m_VoidIterator < rhs.m_VoidIterator; }
	bool operator>(const VoidIteratorType& rhs) const { return m_VoidIterator > rhs.m_VoidIterator; }
	bool operator<=(const VoidIteratorType& rhs) const { return m_VoidIterator <= rhs.m_VoidIterator; }
	bool operator>=(const VoidIteratorType& rhs) const { return m_VoidIterator >= rhs.m_VoidIterator; }

	T& operator*() { return *static_cast<T*>(m_VoidIterator.m_ptr); }
	const T& operator*() const { return *static_cast<T*>(m_VoidIterator.m_ptr); }
	T* operator->() { return static_cast<T*>(m_VoidIterator.m_ptr); }

	VoidIteratorType& operator++() { ++m_VoidIterator; return *this; }
	VoidIteratorType& operator--() { --m_VoidIterator; return *this; }
	VoidIteratorType operator++(int) { return m_VoidIterator++; }
	VoidIteratorType operator--(int) { return m_VoidIterator--; }

	VoidIteratorType& operator+=(size_t rhs) { return m_VoidIterator += rhs; }
	VoidIteratorType& operator-=(size_t rhs) { return m_VoidIterator += rhs; }

	VoidIteratorType operator+(size_t rhs) { return m_VoidIterator + rhs; }
	VoidIteratorType operator-(size_t rhs) { return m_VoidIterator - rhs; }
	//VoidIterator operator+(const VoidIterator& rhs) { return VoidIterator{ m_ptr + rhs.m_ptr }; }
	//VoidIterator operator-(const VoidIterator& rhs) { return VoidIterator{ m_ptr - rhs.m_ptr }; }

	T* operator[](size_t offset) const { return m_VoidIterator[offset]; }

private:
	VoidIterator m_VoidIterator;
};