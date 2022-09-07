#pragma once

template <typename T> class Reference;

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