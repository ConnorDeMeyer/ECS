#pragma once
#include "SystemBase.h"
#include "../Registry/TypeView.h"
#include "../Registry/TypeBinding.h"
#include "../TypeInformation/Concepts.h"


/**
 * ViewSystem is a system that acts on a single component.
 * It has a pointer to the TypeView that contains the Component and can be accessed using the GetTypeView() method.
 */
template <typename Components>
class ViewSystem : public SystemBase
{
public:
	using ComponentType = Components;

public:

	ViewSystem(const SystemParameters& parameters) : SystemBase(parameters) {}

	TypeView<Components>* GetTypeView() const { return m_TypeView; }
	void SetTypeView(TypeView<Components>* view) { m_TypeView = view; }

	constexpr uint32_t GetTypeId() const { return reflection::type_id<Components>(); }
	constexpr std::string_view GetTypeName() const { return reflection::type_name<Components>(); }

	size_t GetEntityAmount() override { return m_TypeView->GetActiveAmount(); }
	void PrintTypes(std::ostream& stream) override { m_TypeView->PrintType(stream); }

protected:

	TypeView<Components>* m_TypeView{};

};

/**
 * BindingSystem is a system that acts on multiple components.
 * It has a pointer to the TypeBinding that contains the given Components...
 * @warning: Must use 2 or more components in template
 */
template <typename... Components>
class BindingSystem : public SystemBase
{
	static_assert(sizeof...(Components) >= 2);

public:
	BindingSystem(const SystemParameters& parameters) : SystemBase(parameters) {}

	TypeBinding* GetTypeBinding() const { return m_Binding; }
	void SetTypeBinding(TypeBinding* binding) { m_Binding = binding; }

	static constexpr std::array<uint32_t, sizeof...(Components)> GetTypes()	{ return reflection::Type_ids<Components...>();}

	size_t GetEntityAmount() override { return m_Binding->GetSize(); }
	void PrintTypes(std::ostream& stream) override { m_Binding->PrintTypes(stream); }

protected:

	TypeBinding* m_Binding{};

};

/**
 * View System that can be initialized using a function taking the reference of the component.
 * This will call the function on every component when the Execute() method is called.
 */
template <typename Component>
class ViewSystemDynamic final : public ViewSystem<Component>
{
public:
	ViewSystemDynamic(const SystemParameters& parameters, std::function<void(Component&)> function) : ViewSystem<Component>(parameters), m_ExecutingFunction(function) {}

	void Execute() override
	{
		for (auto& element : *ViewSystem<Component>::m_TypeView)
			m_ExecutingFunction(element);
	}

private:

	std::function<void(Component&)> m_ExecutingFunction;
};

/**
 * Same as ViewSystemDynamic but the first parameter is deltaTime
 */
template <typename Component>
class ViewSystemDynamicDT final : public ViewSystem<Component>
{
public:
	ViewSystemDynamicDT(const SystemParameters& parameters, std::function<void(float, Component&)> function) : ViewSystem<Component>(parameters), m_ExecutingFunction(function) {}

	void Execute() override
	{
		for (auto& element : *ViewSystem<Component>::m_TypeView)
			m_ExecutingFunction(SystemBase::GetDeltaTime(), element);
	}

private:

	std::function<void(float, Component&)> m_ExecutingFunction;
};

/**
 * Binding system that can be initialized using a function taking the references of the components.
 * This will call the function on every element of the TypeBinding when the Execute() method is called.
 */
template <typename... Components>
class BindingSystemDynamic final : public BindingSystem<Components...>
{
public:
	BindingSystemDynamic(const SystemParameters& parameters, const std::function<void(Components&...)>& function) : BindingSystem<Components...>(parameters), m_ExecutingFunction(function) {}

	void Execute() override
	{
		BindingSystem<Components...>::m_Binding->ApplyFunctionOnAll(m_ExecutingFunction);
	}

private:

	std::function<void(Components&...)> m_ExecutingFunction;

};

/**
 * Same as BindingSystemDynamic but the first parameter is deltaTime
 */
template <typename... Components>
class BindingSystemDynamicDT final : public BindingSystem<Components...>
{
public:
	BindingSystemDynamicDT(const SystemParameters& parameters, const std::function<void(float, Components&...)>& function) : BindingSystem<Components...>(parameters), m_ExecutingFunction(function) {}

	void Execute() override
	{
		BindingSystem<Components...>::m_Binding->ApplyFunctionOnAllDT(m_ExecutingFunction, SystemBase::GetDeltaTime());
	}

private:

	std::function<void(float, Components&...)> m_ExecutingFunction;

};