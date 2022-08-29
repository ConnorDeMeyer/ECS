#pragma once
#include "TypeBinding.h"

class TypeBindingBase
{

	friend class EntityRegistry;

public:

	TypeBindingBase()										= default;

	TypeBindingBase(const TypeBindingBase&)					= default;
	TypeBindingBase(TypeBindingBase&&) noexcept				= default;
	TypeBindingBase& operator=(const TypeBindingBase&)		= default;
	TypeBindingBase& operator=(TypeBindingBase&&) noexcept	= default;
	virtual ~TypeBindingBase()								= default;

protected:

	virtual void AddEmptyData(size_t& outPos) = 0;
	virtual void RegisterEntity(entityId entity, size_t offset) = 0;
	virtual void SetVoidReference(const VoidReference& ref, size_t bindingView, size_t viewPos) = 0;
	virtual bool Contains(entityId id) const = 0;
	virtual bool Contains(entityId id, size_t& offset) const = 0;
	virtual void RemoveId(entityId id) = 0;
	virtual void SwapRemove(size_t offset, std::unordered_map<uint32_t, std::unique_ptr<TypeViewBase>>& typeViews) = 0;
};
