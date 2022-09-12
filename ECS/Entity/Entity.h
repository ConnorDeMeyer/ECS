#pragma once
#include <cstdint>
#include <limits>

using entityId = size_t;

class EntityRegistry;

/**
 * Class that contains an entityId and reference to the EntityRegistry it was created at.
 */
class Entity final
{

	friend class EntityRegistry;
	friend class GameObject;

private:

	Entity(EntityRegistry& registry, entityId id = Entity::InvalidId)
		: m_Registry(registry)
		, m_Id(id)
	{}

public:

	entityId GetId() const { return m_Id; }
	const EntityRegistry& GetRegistry() const { return m_Registry; }
	EntityRegistry& GetRegistry() { return m_Registry; }

private:

	EntityRegistry& m_Registry;
	entityId m_Id{ Entity::InvalidId };

public:

	constexpr static entityId InvalidId{ std::numeric_limits<entityId>::max() };
};

