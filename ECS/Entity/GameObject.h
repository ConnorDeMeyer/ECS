#pragma once
#include "Entity.h"
#include "../Registry/EntityRegistry.h"

/**
 * Wrapper class for Entity that allows for easily adding, getting and removing Components.
 */
class GameObject final
{
public:

	GameObject(EntityRegistry& registry)
		: m_Entity(registry.CreateEntity())
	{}

	GameObject(EntityRegistry& registry, entityId id)
		: m_Entity(registry.CreateOrGetEntity(id))
	{}

	~GameObject()
	{
		m_Entity.GetRegistry().RemoveEntity(m_Entity);
	}

	GameObject(const GameObject&) = delete;
	GameObject& operator=(const GameObject&) = delete;

	GameObject(GameObject&& other) : m_Entity(other.m_Entity)
	{
		other.m_Entity.m_Id = Entity::InvalidId;
	}
	GameObject& operator=(GameObject&& other)
	{
		m_Entity.m_Id = other.m_Entity.m_Id;
		other.m_Entity.m_Id = Entity::InvalidId;
	}

	template <typename Component>
	Reference<Component> GetComponent()
	{
		constexpr uint32_t typeId{ reflection::type_id<Component>() };
		return m_Entity.GetRegistry().GetComponent(typeId, m_Entity);
	}

	template <typename Component>
	Reference<Component> AddComponent()
	{
		constexpr uint32_t typeId{ reflection::type_id<Component>() };
		return m_Entity.GetRegistry().AddComponentInstantly(typeId, m_Entity);
	}

	template <typename Component>
	void RemoveComponent()
	{
		constexpr uint32_t typeId{ reflection::type_id<Component>() };
		m_Entity.GetRegistry().RemoveComponent(typeId, m_Entity);
	}

	template <typename Component>
	void Enable()
	{
		constexpr uint32_t typeId{ reflection::type_id<Component>() };
		m_Entity.GetRegistry().Enable(typeId, m_Entity);
	}

	template <typename Component>
	void Disable()
	{
		constexpr uint32_t typeId{ reflection::type_id<Component>() };
		m_Entity.GetRegistry().Disable(typeId, m_Entity);
	}

	template <typename Component>
	bool IsEnabled()
	{
		constexpr uint32_t typeId{ reflection::type_id<Component>() };
		return m_Entity.GetRegistry().IsEnabled(typeId, m_Entity);
	}


private:

private:

	Entity m_Entity;

};