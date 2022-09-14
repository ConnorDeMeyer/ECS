#pragma once
#include <string>

class EntityRegistry;

namespace GUI::Registry
{
	void UpdateAll();

	void DrawEntityRegistryInfo();
	void DrawTypeViews();
	void DrawEntities();
	void DrawTypeBindings();
	void DrawSystems();
	void DrawProfiler();

	void SetEntityRegistry(EntityRegistry* pRegistry);
	EntityRegistry* GetEntityRegistry();

	void RegisterEntityRegistry(EntityRegistry* pRegistry);
	void RegisterEntityRegistry(EntityRegistry* pRegistry, const char* name);
	void RemoveRegisteredRegistry(EntityRegistry* pRegistry);

	void RefreshRegistryInfo();
	void RefreshTypeViews();

	void AddSystem(const std::string& name);
	void RemoveSystem(const std::string& name);

}