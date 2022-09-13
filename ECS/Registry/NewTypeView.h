#pragma once
#include <cassert>
#include <functional>
#include <vector>
#include <memory>
#include <unordered_map>

#include "../TypeInformation/reflection.h"
#include "../DataAccess/References.h"
#include "../DataAccess/Iterators.h"
#include "../Entity/Entity.h"
#include "../Allocators/ObjectPoolAllocator.h"

class EntityRegistry;

class BaseTypeViewData
{
public:
	BaseTypeViewData() = default;
	virtual ~BaseTypeViewData() = default;

	BaseTypeViewData(const BaseTypeViewData&) = delete;
	BaseTypeViewData(BaseTypeViewData&&) = delete;
	BaseTypeViewData& operator=(const BaseTypeViewData&) = delete;
	BaseTypeViewData& operator=(BaseTypeViewData&&) = delete;

	virtual VoidReference Add(entityId id) = 0;
	virtual VoidReference Add(const Entity& entity) = 0;
	virtual void* AddAfterUpdate(entityId id) = 0;
	virtual void* AddAfterUpdate(const Entity& entity) = 0;

	void Update(float deltaTime);
	virtual void UpdateData() = 0;

protected:

	ObjectPoolAllocator<VoidReferencePointer> m_ReferencePool;
	std::vector<VoidReferencePointer*> m_PendingDeleteReferences;
	std::unordered_map<entityId, VoidReferencePointer*> m_EntityDataReferences;
	float m_AccumulatedTime{};
};

template <typename Data>
class TypeViewData : public BaseTypeViewData
{
public:
	void UpdateData() override;

	VoidReference Add(const Entity& entity) override;
	VoidReference Add(entityId id) override;
	void* AddAfterUpdate(const Entity& entity) override;
	void* AddAfterUpdate(entityId id) override;

	Reference<Data> Add(entityId id, const Data& data);
	Reference<Data> Add(entityId id, Data&& data);

private:
	std::vector<Data> m_Data;
	std::vector<std::pair<entityId, Data>> m_PendingAddedData;
};

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

struct TypeViewInfo
{
	uint32_t typeId;
	std::string typeName;
	size_t ElementSize;
	size_t totalSize;
	size_t activeAmount;
	size_t inactiveAmount;
};

class NewTypeView final
{
public:
	NewTypeView(EntityRegistry& registry)
	: m_Registry(registry)
	{}

	~NewTypeView() = default;

	NewTypeView(const NewTypeView&) = delete;
	NewTypeView(NewTypeView&&) = delete;
	NewTypeView& operator=(const NewTypeView&) = delete;
	NewTypeView& operator=(NewTypeView&&) = delete;

	template <typename Component>
	void Initialize()
	{
		assert(!m_TypeId && !m_DataSize);
		m_DataSize = sizeof(Component);
		m_TypeId = reflection::type_id<Component>();
		m_Data = std::make_unique<TypeViewData<Component>>();
	}

public:

	void Update(float deltaTime);

	size_t GetSize() const;

	void SortData(volatile SortingProgress& sortingProgress, const volatile bool& quit);

	void SerializeView(std::ostream& stream);
	void DeserializeView(std::istream& stream);
	void PrintType(std::ostream& stream);
	TypeViewInfo GetInfo();
	void UpdateInfo(TypeViewInfo&);

	bool Contains(entityId id);
	entityId GetEntityId(const void* elementAddress);
	VoidReference AddEntity(entityId id);

	VoidReference GetVoidReference(entityId id) const;
	VoidIterator GetVoidIterator();
	VoidIterator GetVoidIteratorEnd();

	void Remove(entityId id);

	void* AddAfterUpdate(entityId id);

	template <typename Component>
	entityId GetEntityId(const Component* element) const;
	template <typename Component>
	Reference<Component> Get(entityId id) const;
	template <typename Component>
	Reference<Component> Add(entityId id, const Component& data);
	template <typename Component>
	Reference<Component> Add(entityId id, Component&& data);
	template <typename Component>
	Reference<Component> Add(entityId id);
	template <typename Component>
	Reference<Component> Add(Entity entity);
	template <typename Component>
	Component* AddAfterUpdate(entityId id);
	template <typename Component>
	Component* AddAfterUpdate(Entity entity);
	template <typename Component>
	const Component* GetData() const { return m_Data.data(); }
	template <typename Component>
	auto begin() { return VoidIteratorType<Component>(m_Data.data(), m_ElementSize); }
	template <typename Component>
	auto end() { return VoidIteratorType<Component>(m_Data.data() + m_Data.size(), m_ElementSize) - m_InactiveItems; }
	template <typename Component>
	auto arrayEnd() { return m_Data.end(); }
	template <typename Component>
	void SetInactive(const Component* element);
	template <typename Component>
	void SetActive(const Component* element);
	

	size_t GetActiveAmount() const { return GetSize() - m_InactiveItems; }
	size_t GetInactiveAmount() const { return m_InactiveItems; }
	void SetInactive(entityId id);
	void SetActive(entityId id);

private:
	/** Creates a map between the id and the data and vice-versa*/
	template <typename Component>
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
	size_t GetPositionInArray(entityId id) const;

	/** Returns the position of the element inside of the Data array*/
	template <typename Component>
	size_t GetPositionInArray(const Component* data) const;

public:

	EntityRegistry& GetEntityRegistry() { return m_Registry; }
	const EntityRegistry& GetEntityRegistry() const { return m_Registry; }
	const std::unique_ptr<BaseTypeViewData>& GetTypeViewData() const { return m_Data; }
	size_t GetComponentSize() const { return m_DataSize; }
	uint32_t GetTypeId() const { return m_TypeId; }
	const std::vector<entityId>& GetRegisteredEntities() const { return m_DataEntityMap; }
	ViewDataFlag GetDataFlag() const { return m_DataFlag; }
	uint16_t GetDataFlagId() const { return m_DataFlagId; }

private:

	EntityRegistry& m_Registry;

	std::unique_ptr<BaseTypeViewData> m_Data;

	size_t m_DataSize{};
	uint32_t m_TypeId{};


	std::vector<entityId> m_DataEntityMap{ 16,Entity::InvalidId };

	

	

	size_t m_InactiveItems{};

	

	std::vector<std::function<void(NewTypeView*, entityId)>> OnElementAdd;

	std::vector<std::function<void(NewTypeView*, entityId)>> OnElementRemove;

	ViewDataFlag m_DataFlag{ ViewDataFlag::valid };
	uint16_t m_DataFlagId{ 1 };
};

template <typename Data>
void TypeViewData<Data>::UpdateData()
{
	for (auto& entity : m_PendingAddedData)
	{
		Add(entity.first, std::move(entity.second));
	}
	m_PendingAddedData.clear();
}

template <typename Data>
VoidReference TypeViewData<Data>::Add(const Entity& entity)
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

template <typename Data>
VoidReference TypeViewData<Data>::Add(entityId id)
{
}

template <typename Data>
void* TypeViewData<Data>::AddAfterUpdate(const Entity& entity)
{
}

template <typename Data>
void* TypeViewData<Data>::AddAfterUpdate(entityId id)
{
}

template <typename Data>
Reference<Data> TypeViewData<Data>::Add(entityId id, const Data& data)
{
}

template <typename Data>
Reference<Data> TypeViewData<Data>::Add(entityId id, Data&& data)
{
}
