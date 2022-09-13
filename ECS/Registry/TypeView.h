#pragma once

#include <vector>
#include <unordered_map>
#include <cassert>
#include <algorithm>
#include <functional>
#include <ranges>
#include <string>

#include "../TypeInformation/reflection.h"
#include "../Allocators/ObjectPoolAllocator.h"
#include "../Sorting/SmoothSort.h"
#include "TypeViewBase.h"
#include "../Serialize/Serializer.h"
#include "../TypeInformation/Concepts.h"
#include "../TypeInformation/TypeInformation.h"

inline bool SortCompare(const int& i0, const int& i1) { return i0 < i1; }

template <typename Component>
class TypeView : public TypeViewBase
{
public:

	using ComponentType = Component;

	constexpr static float ReferenceRemovalInterval{ 1 };

public:

	TypeView(EntityRegistry* pRegistry) : TypeViewBase(pRegistry) {}
	~TypeView() override = default;

	TypeView(const TypeView&) = delete;
	TypeView(TypeView&&) = delete;
	TypeView& operator=(const TypeView&) = delete;
	TypeView& operator=(TypeView&&) = delete;

public:

	void Update(float deltaTime) override;

	entityId GetEntityId(const Component* element) const;

	entityId GetEntityId(const void* elementAddress) override;

	Reference<Component> Get(entityId id) const;

	VoidReference GetVoidReference(entityId id) const override;

	/** Copies the data instance into the contiguous array that is kept by this class*/
	Reference<Component> Add(entityId id, const Component& data);

	/** Moves the data instance into the contiguous array that is kept by this class*/
	Reference<Component> Add(entityId id, Component&& data);

	/** Creates an instance of type Component and emplaces it into the contiguous array that is kept by this class*/
	Reference<Component> Add(entityId id);

	Reference<Component> Add(Entity entity);

	Component* AddAfterUpdate(entityId id);

	Component* AddAfterUpdate(Entity entity);

	/**
	 * Removes the associated instance of type Component from the array using a swap remove
	 * This will invalidate the order of the array if it was sorted
	 */
	void Remove(entityId id) override;

	bool Contains(entityId id) override;

	/** Returns the amount of elements inside of the underlying array.*/
	size_t GetSize() const { return m_Data.size(); }

	/** Returns the amount of active elements inside of the view*/
	size_t GetActiveAmount() const { return GetSize() - m_InactiveItems; }

	/** Returns the amount of inactive elements inside of the view.*/
	size_t GetInactiveAmount() const { return m_InactiveItems; }

	/** Returns the array of instances of Type Component stored inside of the view*/
	const Component* GetData() const { return m_Data.data(); }

	/** Returns the start iterator of the data*/
	auto begin() { return VoidIteratorType<Component>(m_Data.data(), m_ElementSize); }

	/** Returns the end of the iterator without inactive items*/
	auto end() { return VoidIteratorType<Component>(m_Data.data() + m_Data.size(), m_ElementSize) - m_InactiveItems; }

	auto beginInactives() { return VoidIteratorType<Component>(m_Data, m_ElementSize) + GetActiveAmount(); }

	auto endInactive() { return VoidIteratorType<Component>(m_Data.back(), m_ElementSize) + 1; }

	/** Returns the end of the array, including the inactive items*/
	auto arrayEnd() { return m_Data.end(); }

	/** Sets the associated instance of type Component inactive*/
	void SetInactive(entityId id);

	/** Sets the element inactive*/
	void SetInactive(const Component* element);

	/** Sets the associated instance of type Component active*/
	void SetActive(entityId id);

	/** Sets the element active*/
	void SetActive(const Component* element);

	bool IsActive(entityId id) const;

	bool IsActive(const Component* element) const;

	uint32_t GetTypeId() const override { return typeId; }

	void SerializeView(std::ostream& stream) override;

	void DeserializeView(std::istream& stream) override;

	void PrintType(std::ostream& stream) override;

	TypeViewInfo GetInfo() override;

	void UpdateInfo(TypeViewInfo&) override;

	VoidReference AddEntity(entityId id) override;

	void* AddAfterUpdate_void(entityId id) override;

	void Enable(const VoidReference& ref) override;
	void Enable(entityId id) override;
	void Disable(const VoidReference& ref) override;
	void Disable(entityId id) override;
	bool IsEnabled(const VoidReference& ref) const override;
	bool IsEnabled(entityId id) const override;


private:

	VoidIterator GetVoidIterator() override;

	VoidIterator GetVoidIteratorEnd() override;

	/** Creates a map between the id and the data and vice-versa*/
	Reference<Component> AddMap(entityId id, Component* data);

	/** Resizes the DataEntityMap and fills it with invalid ids*/
	void ResizeDataEntityMap(size_t size);

	/** Checks if the DataEntityMap has enough capacity and will resize the map whenever it is too small*/
	void CheckDataEntityMap(size_t size);

	void ResizeData();

	void CheckDataSize();

	/** Removes an element by swapping the last one with the element and popping the back*/
	void SwapRemove(size_t pos);
	
	void ChangeMapping(size_t oldPos, size_t newPos);

	void SetViewDataFlag(ViewDataFlag flag);

	void SwapPositions(size_t pos0, size_t pos1);

	void SortData(volatile SortingProgress& sortingProgress, const volatile bool& quit) override;

	size_t GetPositionInArray(entityId id) const;

	/** Returns the position of the element inside of the Data array*/
	size_t GetPositionInArray(const Component* data) const;

private:

	std::vector<Component> m_Data;
	std::unordered_map<entityId, ReferencePointer<Component>*> m_EntityDataReferences;

	/** Amount of inactive items inside of the array*/
	size_t m_InactiveItems{};

	ObjectPoolAllocator<ReferencePointer<Component>> m_ReferencePool;

	std::vector<ReferencePointer<Component>*> m_PendingDeleteReferences;

	std::vector<std::pair<entityId, Component>> m_AddedEntitiesUpdate;

	const uint32_t typeId{ reflection::type_id<Component>() };
	const size_t m_ElementSize{ sizeof(Component) };

	float m_AccumulatedTime{};

};

template <typename T>
void TypeView<T>::Update(float deltaTime)
{
	m_AccumulatedTime += deltaTime;

	if (m_AccumulatedTime >= ReferenceRemovalInterval)
	{
		m_AccumulatedTime -= ReferenceRemovalInterval;

		const size_t size = m_PendingDeleteReferences.size();
		for (size_t i{}; i < size; ++i)
		{
			auto ref = m_PendingDeleteReferences[size - i - 1];
			if (ref->GetReferencesAmount() == 0)
			{
				m_ReferencePool.deallocate(ref);
				m_PendingDeleteReferences[size - i - 1] = m_PendingDeleteReferences.back();
				m_PendingDeleteReferences.pop_back();
			}
		}
	}

	for (auto& entity : m_AddedEntitiesUpdate)
	{
		Add(entity.first, std::move(entity.second));
	}
	m_AddedEntitiesUpdate.clear();
}

template <typename T>
entityId TypeView<T>::GetEntityId(const T* element) const
{
	assert(m_Data.data() - element < m_Data.size());
	return m_DataEntityMap[m_Data.data() - element];
}

template <typename T>
entityId TypeView<T>::GetEntityId(const void* elementAddress)
{
	const T* element = static_cast<const T*>(elementAddress);
	return m_DataEntityMap[GetPositionInArray(element)];
}

template <typename T>
Reference<T> TypeView<T>::Get(entityId id) const
{
	auto it = m_EntityDataReferences.find(id);
	if (it != m_EntityDataReferences.end())
	{
		return *it->second;
	}
	return Reference<T>::InvalidRef();
}

template <typename T>
VoidReference TypeView<T>::GetVoidReference(entityId id) const
{
	return VoidReference(static_cast<void*>(&Get(id).GetReferencePointer()));
}

template <typename T>
Reference<T> TypeView<T>::Add(entityId id, const T& data)
{
	CheckDataSize();
	T* element = &m_Data.emplace_back(data);
	auto ref = AddMap(id, element);
	for (auto& callback : OnElementAdd)
		callback(this, id);

	SetViewDataFlag(ViewDataFlag::dirty);

	if constexpr (Initializable<T>)
	{
		*element.Initialize(GetRegistry());
	}

	return ref;
}

template <typename T>
Reference<T> TypeView<T>::Add(entityId id, T&& data)
{
	CheckDataSize();
	T* element = &m_Data.emplace_back(std::move(data));
	auto ref = AddMap(id, element);
	for (auto& callback : OnElementAdd)
		callback(this, id);

	SetViewDataFlag(ViewDataFlag::dirty);

	if constexpr (Initializable<T>)
	{
		*element.Initialize(GetRegistry());
	}

	return ref;
}

template <typename T>
Reference<T> TypeView<T>::Add(entityId id)
{
	CheckDataSize();
	T* element = &m_Data.emplace_back();
	auto ref = AddMap(id, element);
	for (auto& callback : OnElementAdd)
		callback(this, id);

	SetViewDataFlag(ViewDataFlag::dirty);

	if constexpr (Initializable<T>)
	{
		*element.Initialize(GetRegistry());
	}

	return ref;
}

template <typename T>
Reference<T> TypeView<T>::Add(Entity entity)
{
	return Add(entity.GetId());
}

template <typename T>
T* TypeView<T>::AddAfterUpdate(entityId id)
{
	return &m_AddedEntitiesUpdate.emplace_back(id, T()).second;
}

template <typename T>
T* TypeView<T>::AddAfterUpdate(Entity entity)
{
	return AddAfterUpdate(entity.GetId());
}

template <typename T>
void TypeView<T>::Remove(entityId id)
{
	auto it = m_EntityDataReferences.find(id);
	if (it != m_EntityDataReferences.end())
	{
		for (auto& callback : OnElementRemove)
			callback(this, id);

		size_t pos = it->second->m_ptr - m_Data.data();

		// swap remove
		m_Data[pos] = std::move(m_Data.back());
		m_Data.pop_back();

		// deallocate if no references to the element exists
		if (it->second->GetReferencesAmount() == 0)
			m_ReferencePool.deallocate(it->second);
		else
			m_PendingDeleteReferences.push_back(it->second);

		m_EntityDataReferences.erase(it);

		SetViewDataFlag(ViewDataFlag::dirty);
	}
}

template <typename T>
bool TypeView<T>::Contains(entityId id)
{
	return (m_EntityDataReferences.contains(id));
}

template <typename T>
void TypeView<T>::SetInactive(entityId id)
{
	assert(m_EntityDataReferences.contains(id));
	SwapPositions( GetActiveAmount() - 1, GetPositionInArray(id));
	++m_InactiveItems;
}

template <typename T>
void TypeView<T>::SetInactive(const T* element)
{
	SwapPositions(GetActiveAmount() - 1, GetPositionInArray(element));
	++m_InactiveItems;
}

template <typename T>
void TypeView<T>::SetActive(entityId id)
{
	assert(m_EntityDataReferences.contains(id));
	SwapPositions(GetActiveAmount(), GetPositionInArray(id));
	--m_InactiveItems;
}

template <typename T>
void TypeView<T>::SetActive(const T* element)
{
	SwapPositions(GetActiveAmount(), GetPositionInArray(element));
	--m_InactiveItems;
}

template <typename Component>
bool TypeView<Component>::IsActive(entityId id) const
{
	return GetPositionInArray(id) < GetActiveAmount();
}

template <typename Component>
bool TypeView<Component>::IsActive(const Component* element) const
{
	return GetPositionInArray(element) < GetActiveAmount();
}

template <typename Component>
void TypeView<Component>::SerializeView(std::ostream& stream)
{
	WriteStream(stream, GetSize());

	// Serialize the Data Entities map
	stream.write(reinterpret_cast<const char*>(m_DataEntityMap.data()), GetSize() * sizeof(entityId));

	//static_assert(std::is_trivially_copyable_v<Component> || Streamable<Component>, 
	//	"Component has to be trivially copyable (POD) for a Serialize and Deserialize method not to exist for the Component.\n Please define both Serialize(std::ostream&) and Deserialize(std::istream&) methods for the Component");

	// add the size of the serialized data
	const auto sizePos = stream.tellp();
	WriteStream(stream, size_t{5}); // write empty size that will later be overriden
	const auto startPos = stream.tellp();

	// Serialize all the Components
	if constexpr ( Streamable<Component> )
		for (auto& element : m_Data)
			// If the component has a Serialize function, use that one
			element.Serialize(stream);
	else if constexpr (std::is_trivially_copyable_v<Component>)
		// else just copy all the data
		stream.write(reinterpret_cast<const char*>(m_Data.data()), m_Data.size() * sizeof(Component));
	else
	{
		constexpr uint32_t typeId{ reflection::type_id<Component>() };
		std::cout << "Couldn't Serialize [" << TypeInformation::GetTypeName(typeId) << "] because its not trivially copyable (POD). You may provide your own Serialize(std::ostream&) and Deserialize(std::istream&) methods\n";
	}

	// end position
	const auto endPos = stream.tellp();
	const auto amountWritten{ endPos - startPos };
	stream.seekp(sizePos);
	WriteStream(stream, size_t(amountWritten)); // overwrite the previous empty size

	// go back to end
	stream.seekp(endPos);
}

template <typename Component>
void TypeView<Component>::DeserializeView(std::istream& stream)
{
	assert(GetSize() == 0); // Check if view is empty

	size_t size{};
	ReadStream(stream, size);

	// Resize the vectors
	ResizeDataEntityMap(size);
	m_Data.resize(size);

	// Get all entities
	stream.read(reinterpret_cast<char*>(m_DataEntityMap.data()), size * sizeof(entityId));

	// Get the data size
	size_t dataSize{};
	ReadStream(stream, dataSize);

	// get begin position to verify size
	auto beginPos = stream.tellg();

	// Deserialize the elements or copy the data if they dont have a Deserialize function
	if constexpr ( Streamable<Component> )
		for (auto& element : m_Data)
			element.Deserialize(stream);
	else if constexpr (std::is_trivially_copyable_v<Component>)
		stream.read(reinterpret_cast<char*>(m_Data.data()), size * sizeof(Component));
	else
	{
		constexpr uint32_t typeId{ reflection::type_id<Component>() };
		std::cout << "Couldn't Deserialize [" << TypeInformation::GetTypeName(typeId) << "] because its not trivially copyable (POD). You may provide your own Serialize(std::ostream&) and Deserialize(std::istream&) methods\n";
	}

	// Verify size;
	auto endPos = stream.tellg();
	if (size_t(endPos - beginPos) != dataSize)
	{
		auto typeName = TypeInformation::GetTypeName(reflection::type_id<Component>());
		throw std::runtime_error("Deserializing failed for " + std::string(typeName) + ". Amount of data written ("
			+ std::to_string(dataSize) + ") was different from amount read (" + std::to_string(endPos - beginPos) + ")");
	}

	// Fill containers, create references, and create maps for each entity
	for (size_t i{}; i < size; ++i)
	{
		// create the references
		auto reference = m_ReferencePool.allocate();
		reference->m_ptr = &m_Data[i];

		// insert into entity data map
		m_EntityDataReferences.emplace(m_DataEntityMap[i], reference);

		for (auto& onAdd : OnElementAdd)
			onAdd(this, m_DataEntityMap[i]);
	}
}

template <typename T>
void TypeView<T>::PrintType(std::ostream& stream)
{
	stream << '[' << reflection::type_name<T>() << ']';
}

template <typename Component>
TypeViewInfo TypeView<Component>::GetInfo()
{
	return TypeViewInfo
	{
		GetTypeId(),
		TypeInformation::GetTypeName(GetTypeId()),
		m_ElementSize,
		GetSize(),
		GetActiveAmount(),
		GetInactiveAmount()
	};
}

template <typename Component>
void TypeView<Component>::UpdateInfo(TypeViewInfo& info)
{
	info.totalSize = GetSize();
	info.activeAmount = GetActiveAmount();
	info.inactiveAmount = GetInactiveAmount();
}

template <typename Component>
VoidReference TypeView<Component>::AddEntity(entityId id)
{
	return VoidReference(static_cast<void*>(&Add(id).GetReferencePointer()));
}

template <typename Component>
void* TypeView<Component>::AddAfterUpdate_void(entityId id)
{
	return AddAfterUpdate(id);
}

template <typename Component>
void TypeView<Component>::Enable(const VoidReference& ref)
{
	assert(ref.ToReference<Component>().IsValid());
	SetActive(ref.ToReference<Component>().get());
}

template <typename Component>
void TypeView<Component>::Enable(entityId id)
{
	SetActive(id);
}

template <typename Component>
void TypeView<Component>::Disable(const VoidReference& ref)
{
	assert(ref.ToReference<Component>().IsValid());
	SetInactive(ref.ToReference<Component>().get());
}

template <typename Component>
void TypeView<Component>::Disable(entityId id)
{
	SetInactive(id);
}

template <typename Component>
bool TypeView<Component>::IsEnabled(const VoidReference& ref) const
{
	return IsActive(ref.ToReference<Component>().get());
}

template <typename Component>
bool TypeView<Component>::IsEnabled(entityId id) const
{
	return IsActive(id);
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
Reference<T> TypeView<T>::AddMap(entityId id, T* data)
{
	size_t pos = GetPositionInArray(data);

	auto reference = m_ReferencePool.allocate();
	reference->m_ptr = data;

	// insert into entity data map
	m_EntityDataReferences.emplace(id, reference);

	// insert into data entity map
	CheckDataEntityMap(pos);
	m_DataEntityMap[pos] = id;

	return Reference<T>(reference);
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
	m_Data.reserve(m_Data.empty() ? 4 : (m_Data.size() * 2));
	T* newLoc = m_Data.data();

	const int64_t difference = reinterpret_cast<int8_t*>(newLoc) - reinterpret_cast<int8_t*>(originalLoc);

	for (auto& reference : m_EntityDataReferences)
	{
		reference.second->m_ptr = reinterpret_cast<T*>(reinterpret_cast<uint8_t*>(reference.second->m_ptr) + difference);
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
		if constexpr (Sortable<T>)
		{
			m_DataFlag = ViewDataFlag::dirty;
			++m_DataFlagId;
		}
		break;
	case ViewDataFlag::invalid:
		if constexpr (Sortable<T>)
		{
			volatile SortingProgress progress{};
			const volatile bool quit{};
			SortData(progress, quit);
			m_DataFlag = ViewDataFlag::valid;
		}
		else
		{
			m_DataFlag = ViewDataFlag::valid;
		}
		break;
	}
}

template <typename T>
void TypeView<T>::SwapPositions(size_t pos0, size_t pos1)
{
	assert(pos0 < GetSize());
	assert(pos1 < GetSize());
	if (pos0 == pos1) return;

	std::swap(m_Data[pos0], m_Data[pos1]);
	std::swap(m_EntityDataReferences[pos0]->m_ptr, m_EntityDataReferences[pos1]->m_ptr);
	std::swap(m_DataEntityMap[pos0], m_DataEntityMap[pos1]);

	SetViewDataFlag(ViewDataFlag::dirty);
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

		SmoothSort(DataCopy.get(), newEntityMapping.get(), m_DataFlag, size);

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

template <typename T>
size_t TypeView<T>::GetPositionInArray(entityId id) const
{
	assert(m_EntityDataReferences.contains(id));
	auto ref = m_EntityDataReferences.find(id);
	return GetPositionInArray(ref->second->m_ptr);
}

template <typename T>
size_t TypeView<T>::GetPositionInArray(const T* data) const
{
	assert(data <= &m_Data.back() && data >= &m_Data.front());
	return data - &m_Data.front();
}
