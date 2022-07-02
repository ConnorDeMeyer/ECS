#pragma once
#include <cstdint>
#include <limits>

using entityId = size_t;

class EntityRegistry;

class Entity
{
public:



private:

	entityId m_Id{ Entity::InvalidId };
	EntityRegistry* m_pRegistry{};

public:

	constexpr static entityId InvalidId{ std::numeric_limits<entityId>::max() };
};

