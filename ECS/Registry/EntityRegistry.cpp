#include "EntityRegistry.h"

entityId EntityRegistry::CreateEntity()
{
	entityId newId = m_Entities.size() ? m_Entities.back() + 1 : 0;
	m_Entities.emplace_back(newId);
	return newId;
}
