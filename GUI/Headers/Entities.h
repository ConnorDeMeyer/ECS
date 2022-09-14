#pragma once
#include <cstdint>

class Entity;
class EntityRegistry;
using entityId = size_t;

namespace GUI::Entities
{
	void UpdateAll();

	void DrawEntity();
	void DrawComponentInfos();
	void DrawComponentInfo(uint32_t component, bool* window = nullptr);

	void SetEntity(Entity* pEntity);
	void SetEntity(entityId id, EntityRegistry& registry);
	entityId GetEntityId();
	EntityRegistry* GetEntityRegistry();

	void RefreshEntityInfo();
	void RefreshAvailableComponents();
}
