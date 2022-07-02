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
	Reference<T> ToReference() const { return *static_cast<const Reference<T>*>(m_ReferencePointer); }

private:
	const void* m_ReferencePointer{};
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

public:

	std::vector<std::function<void(TypeViewBase*, entityId)>> OnElementAdd;
	
	std::vector<std::function<void(TypeViewBase*, entityId)>> OnElementRemove;

};