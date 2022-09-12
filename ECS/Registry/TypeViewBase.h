#pragma once
#include <cstdint>
#include <functional>
#include <vector>

#include "../Entity/Entity.h"
#include "../DataAccess/References.h"
#include "../DataAccess/Iterators.h"

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

	virtual void Update(float deltaTime) = 0;

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

	virtual void SerializeView(std::ostream& stream) = 0;

	virtual void DeserializeView(std::istream& stream) = 0;

	virtual void Remove(entityId id) = 0;

	virtual void PrintType(std::ostream& stream) = 0;

	virtual TypeViewInfo GetInfo() = 0;

	virtual void UpdateInfo(TypeViewInfo&) = 0;

	virtual VoidReference AddEntity(entityId id) = 0;

	virtual void* AddAfterUpdate_void(entityId id) = 0;

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
	uint16_t m_DataFlagId{ 1 };
	
};