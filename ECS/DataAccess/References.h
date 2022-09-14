#pragma once

template <typename T> class Reference;

/**
 * Reference pointer is a class that always points to the element it is assigned to.
 * It is static in memory and gets updated by the Type View in case the data has to change locations.
 * In case the element it points to is deleted it will contain a nullptr and only be deallocated from memory once no references point to it.
 */
template <typename T>
class ReferencePointer final
{
	friend class Reference<T>;

public:
	ReferencePointer(T* ptr) : m_ptr{ ptr } {}

	ReferencePointer(const ReferencePointer&) = delete;
	ReferencePointer& operator=(const ReferencePointer&) = delete;
	ReferencePointer(ReferencePointer&&) = delete;
	ReferencePointer& operator=(ReferencePointer&&) = delete;

	static ReferencePointer InvalidRef() { return nullptr; }

	bool IsValid() const { return m_ptr != nullptr; }
	size_t GetReferencesAmount() const { return m_Counter; }

	T* m_ptr{};
private:
	size_t m_Counter{};
};

class VoidReferencePointer final
{
public:
	VoidReferencePointer(void* ptr);

	VoidReferencePointer(const VoidReferencePointer&) = delete;
	VoidReferencePointer& operator=(const VoidReferencePointer&) = delete;
	VoidReferencePointer(VoidReferencePointer&&) = delete;
	VoidReferencePointer& operator=(VoidReferencePointer&&) = delete;

	static VoidReferencePointer InvalidRef() { return nullptr; }

	bool IsValid() const { return m_ptr != nullptr; }
	size_t GetReferencesAmount() const { return m_Counter; }

	template <typename T>
	ReferencePointer<T>& ToReferencePointer() { return reinterpret_cast<ReferencePointer<T>>(*this); }

	void* m_ptr{};
private:
	size_t m_Counter{};
};

/**
 * Reference is a class that points to a ReferencePointer which points at an element.
 * It can be moved, deleted, copied, etc without losing the pointer to the original element.
 * In case the element changes memory locations the Type View will update the ReferencePointer.
 * Because the ReferencePointer is static in memory, a Reference will always be pointing to either a valid element or a nullptr.
 */
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

	/** Check if the element has not been deleted yet*/
	bool IsValid() const
	{
		return (m_ReferencePointer && m_ReferencePointer->m_ptr);
	}

	static Reference InvalidRef() { return nullptr; }

private:
	ReferencePointer<T>* m_ReferencePointer;
};

/**
 * Same as a Reference but without a Type.
 * It can be converted to any Reference<Type> but will result in undefined behaviour if not casted to the right type.
 */
class VoidReference final
{
public:
	VoidReference(void* ReferencePointer) : m_ReferencePointer{ ReferencePointer } {}
	VoidReference() = default;

	//template <typename T>
	//VoidReference(Reference<T>& ref) : m_ReferencePointer(&ref.GetReferencePointer()) {}

	template <typename T>
	const ReferencePointer<T>& GetReferencePointer() const { return *static_cast<const ReferencePointer<T>*>(m_ReferencePointer); }

	template <typename T>
	Reference<T> ToReference() const
	{
		auto reference = static_cast<ReferencePointer<T>*>(m_ReferencePointer);
		return *reference;
	}

	void* Data() { return GetReferencePointer<void>().m_ptr; }
	const void* Data() const { return GetReferencePointer<void>().m_ptr; }

private:
	void* m_ReferencePointer{};
};