#pragma once

#include <vector>
#include <unordered_map>
#include <cassert>
#include <algorithm>
#include <functional>
#include <ranges>

#include "../Allocators/ObjectPoolAllocator.h"
#include "../Sorting/SmoothSort.h"
#include "TypeViewBase.h"

template <typename T>
class TypeView : public TypeViewBase
{
public:
	TypeView(EntityRegistry* registry) : TypeViewBase(registry) {};
	virtual ~TypeView() = default;

	TypeView(const TypeView&) = delete;
	TypeView(TypeView&&) = delete;
	TypeView& operator=(const TypeView&) = delete;
	TypeView& operator=(TypeView&&) = delete;

public:

	entityId GetId(const T* element) const;

	Reference<T> Get(entityId id) const;

	VoidReference GetVoidReference(entityId id) const override;

	/** Copies the data instance into the contiguous array that is kept by this class*/
	T* Add(entityId id, const T& data);

	/** Moves the data instance into the contiguous array that is kept by this class*/
	T* Add(entityId id, T&& data);

	/** Creates an instance of type T and emplaces it into the contiguous array that is kept by this class*/
	T* Add(entityId id);

	/**
	 * Removes the associated instance of type T from the array using a swap remove
	 * This will invalidate the order of the array if it was sorted
	 */
	void Remove(entityId id);

	bool Contains(entityId id) override;

	/** Sets the sorting algorithm for which the data will be sorted by.*/
	void SetSortingPredicate(const std::function<bool(const T&, const T&)>& predicate) { m_SortingAlgorithm = predicate; }
	void SetSortingPredicate(std::function<bool(const T&, const T&)>&& predicate) { m_SortingAlgorithm = predicate; };

	/** Returns the amount of elements inside of the underlying array.*/
	size_t GetSize() const { return m_Data.size(); }

	/** Returns the amount of active elements inside of the view*/
	size_t GetActiveAmount() const { return GetSize() - m_InactiveItems; }

	/** Returns the amount of inactive elements inside of the view.*/
	size_t GetInactiveAmount() const { return m_InactiveItems; }

	/** Returns the array of instances of Type T stored inside of the view*/
	const T* GetData() const { return m_Data.data(); }

	/** Returns the start iterator of the data*/
	auto begin() { return m_Data.begin(); }

	/** Returns the end of the iterator without inactive items*/
	auto end() { return m_Data.end() - m_InactiveItems; }

	/** Returns the end of the array, including the inactive items*/
	auto arrayEnd() { return m_Data.end(); }

	/** Sets the associated instance of type T inactive*/
	void SetInactive(entityId id);

	/** Sets the element inactive*/
	void SetInactive(const T* element);

	/** Sets the associated instance of type T active*/
	void SetActive(entityId id);

	/** Sets the element active*/
	void SetActive(const T* element);

private:

	VoidIterator GetVoidIterator() override;

	VoidIterator GetVoidIteratorEnd() override;

	/** Creates a map between the id and the data and vice-versa*/
	void AddMap(entityId id, T* data);

	/** Resizes the DataEntityMap and fills it with invalid ids*/
	void ResizeDataEntityMap(size_t size);

	/** Checks if the DataEntityMap has enough capacity and will resize the map whenever it is too small*/
	void CheckDataEntityMap(size_t size);

	void ResizeData();

	void CheckDataSize();

	/** Removes an element by swapping the last one with the element and popping the back*/
	void SwapRemove(size_t pos);

	/** Returns the position of the element inside of the Data array*/
	size_t GetPosition(T* element);

	void ChangeMapping(size_t oldPos, size_t newPos);

	void SetViewDataFlag(ViewDataFlag flag);

	void SortData(volatile SortingProgress& sortingProgress, const volatile bool& quit) override;

private:

	std::vector<T> m_Data;
	std::unordered_map<entityId, ReferencePointer<T>*> m_EntityDataReferences;

	ReferencePointer<T> m_InvalidReference{ nullptr };

	std::function<bool(const T&, const T&)> m_SortingAlgorithm;

	/** Amount of active items inside of the array*/
	size_t m_InactiveItems{};

	ObjectPoolAllocator<ReferencePointer<T>> m_ReferencePool;

};

template <typename T>
entityId TypeView<T>::GetId(const T* element) const
{
	assert(m_Data.data() - element < m_Data.size());
	return m_DataEntityMap[m_Data.data() - element];
}

template <typename T>
Reference<T> TypeView<T>::Get(entityId id) const
{
	auto it = m_EntityDataReferences.find(id);
	if (it != m_EntityDataReferences.end())
	{
		return *it->second;
	}
	return m_InvalidReference;
}

template <typename T>
VoidReference TypeView<T>::GetVoidReference(entityId id) const
{
	return VoidReference(static_cast<const void*>(&Get(id).GetReferencePointer()));
}

template <typename T>
T* TypeView<T>::Add(entityId id, const T& data)
{
	CheckDataSize();
	T* element = &m_Data.emplace_back(data);
	AddMap(id, element);
	for (auto& callback : OnElementAdd)
		callback(this, id);

	SetViewDataFlag(ViewDataFlag::dirty);

	return element;
}

template <typename T>
T* TypeView<T>::Add(entityId id, T&& data)
{
	CheckDataSize();
	T* element = &m_Data.emplace_back(data);
	AddMap(id, element);
	for (auto& callback : OnElementAdd)
		callback(this, id);

	SetViewDataFlag(ViewDataFlag::dirty);

	return element;
}

template <typename T>
T* TypeView<T>::Add(entityId id)
{
	CheckDataSize();
	T* element = &m_Data.emplace_back();
	AddMap(id, element);
	for (auto& callback : OnElementAdd)
		callback(this, id);

	SetViewDataFlag(ViewDataFlag::dirty);

	return element;
}

template <typename T>
void TypeView<T>::Remove(entityId id)
{
	auto it = m_EntityDataReferences.find(id);
	if (it != m_EntityDataReferences.end())
	{
		size_t pos = it->second->m_ptr - m_Data.data();

		// swap remove
		m_Data[pos] = m_Data.back();
		m_Data.pop_back();

		for (auto& callback : OnElementRemove)
			callback(this, id);

		m_ReferencePool.deallocate(it->second);

		m_EntityDataReferences.erase(it);

		m_DataFlag = ViewDataFlag::dirty;
	}
}

template <typename T>
bool TypeView<T>::Contains(entityId id)
{
	return (m_EntityDataReferences.contains(id));
}

template <typename T>
VoidIterator TypeView<T>::GetVoidIterator()
{
	return VoidIterator(static_cast<void*>(&*begin()), sizeof(T));
}

template <typename T>
VoidIterator TypeView<T>::GetVoidIteratorEnd()
{
	return ++VoidIterator(static_cast<void*>(&*(--end())), sizeof(T));
}

template <typename T>
void TypeView<T>::AddMap(entityId id, T* data)
{
	size_t pos = GetPosition(data);

	auto reference = m_ReferencePool.allocate();
	reference->m_ptr = data;

	// insert into entity data map
	m_EntityDataReferences.emplace(id, reference);

	// insert into data entity map
	CheckDataEntityMap(pos);
	m_DataEntityMap[pos] = id;
}

template <typename T>
void TypeView<T>::ResizeDataEntityMap(size_t size)
{
	m_DataEntityMap.resize(size, Entity::InvalidId);
}

template <typename T>
void TypeView<T>::CheckDataEntityMap(size_t size)
{
	if (size >= m_DataEntityMap.size())
	{
		ResizeDataEntityMap(size * 2);
	}
}

template <typename T>
void TypeView<T>::ResizeData()
{
	T* originalLoc = m_Data.data();
	m_Data.reserve(m_Data.size() * 2);
	T* newLoc = m_Data.data();

	auto difference = newLoc - originalLoc;

	for (auto& reference : m_EntityDataReferences)
	{
		reference.second->m_ptr += difference;
	}
}

template <typename T>
void TypeView<T>::CheckDataSize()
{
	if (m_Data.size() == m_Data.capacity())
	{
		ResizeData();
	}
}

template <typename T>
void TypeView<T>::SwapRemove(size_t pos)
{
	m_Data[pos] = m_Data.back();
	m_Data.pop_back();
	
	ChangeMapping(m_Data.size(), pos);
}

template <typename T>
size_t TypeView<T>::GetPosition(T* element)
{
	size_t pos = element - &*m_Data.begin();
	assert(pos < m_Data.size());
	return pos;
}

template <typename T>
void TypeView<T>::ChangeMapping(size_t oldPosArray, size_t newPosArray)
{
	entityId id = m_DataEntityMap[oldPosArray];
	m_DataEntityMap[newPosArray] = id;
	m_DataEntityMap[oldPosArray] = Entity::InvalidId;

	m_EntityDataReferences[id]->m_ptr = m_Data.data() + newPosArray;
}

template <typename T>
void TypeView<T>::SetViewDataFlag(ViewDataFlag flag)
{
	switch (flag)
	{
	case ViewDataFlag::dirty:
		if (m_SortingAlgorithm)
		{
			m_DataFlag = ViewDataFlag::dirty;
		}
		break;/*
	case ViewDataFlag::invalid:
		if (m_SortingAlgorithm)
		{
			
			m_DataFlag = ViewDataFlag::valid;
		}
		break;*/
	}
}

template <typename T>
void TypeView<T>::SortData(volatile SortingProgress& sortingProgress, const volatile bool& quit)
{
	try
	{
		sortingProgress = SortingProgress::sorting;

		size_t size = m_Data.size();
		auto DataCopy = std::unique_ptr<T[]>(new T[size]);
		auto newEntityMapping = std::unique_ptr<entityId[]>(new entityId[size]);

		std::memcpy(DataCopy.get(), m_Data.data(), size * sizeof(T));
		std::memcpy(newEntityMapping.get(), m_DataEntityMap.data(), size * sizeof(entityId));

		m_DataFlag = ViewDataFlag::sorting;

		SmoothSort(DataCopy.get(), newEntityMapping.get(), m_DataFlag, size, m_SortingAlgorithm);

		if (m_DataFlag == ViewDataFlag::sorting) // in case the data flag changed to dirty while sorting
		{
			sortingProgress = SortingProgress::done;

			auto entityCopyBuffer = std::unique_ptr<T*[]>(new T*[size]);

			while (sortingProgress != SortingProgress::copying)
			{
				if (quit)
					return;
			}

			std::memcpy(m_Data.data(), DataCopy.get(), sizeof(T) * size);

			for (size_t i{}; i < size; ++i)
			{
				auto newId{ newEntityMapping[i] };
				entityCopyBuffer[i] = m_EntityDataReferences[newId]->m_ptr;
			}

			for (size_t i{}; i < size; ++i)
			{
				auto oldId{ m_DataEntityMap[i] };
				m_EntityDataReferences[oldId]->m_ptr = entityCopyBuffer[i];
			}

			sortingProgress = SortingProgress::none;
			m_DataFlag = ViewDataFlag::valid;
		}
		else
		{
			sortingProgress = SortingProgress::canceled;
		}
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what();
		sortingProgress = SortingProgress::canceled;
	}
}

