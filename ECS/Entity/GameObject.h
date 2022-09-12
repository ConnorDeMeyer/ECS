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

	template <typename T>
	Reference<T> GetComponent()
	{
		return m_Entity.GetRegistry().GetComponent<T>(m_Entity);
	}

	template <typename T>
	Reference<T> AddComponent()
	{
		return m_Entity.GetRegistry().AddComponentInstantly<T>(m_Entity);
	}

	template <typename T>
	void RemoveComponent()
	{
		m_Entity.GetRegistry().RemoveComponent<T>(m_Entity);
	}

private:

private:

	Entity m_Entity;

};