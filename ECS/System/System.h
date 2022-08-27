#pragma once
#include "SystemBase.h"
#include "../Registry/TypeView.h"
#include "../Registry/TypeBinding.h"

template <typename T>
class ViewSystem : public SystemBase
{
public:
	ViewSystem(const std::string& name, TypeView<T>* typeView, int32_t executionOrder = int32_t(ExecutionTime::Update)) : SystemBase(name, executionOrder), m_TypeView(typeView) {}

	TypeView<T>* GetTypeView() const { return m_TypeView; }

	using ElementType = T;

protected:

	TypeView<T>* m_TypeView{};

};
template <typename Class, typename U>
concept isViewSystem = std::is_base_of_v<ViewSystem<U>, Class>;


template <typename... Types>
class BindingSystem : public SystemBase
{
	static_assert(sizeof...(Types) >= 2);
public:
	BindingSystem(const std::string& name, TypeBinding<Types...>* typeBinding, int32_t executionOrder = int32_t(ExecutionTime::Update)) : SystemBase(name, executionOrder), m_Binding(typeBinding) {}

	TypeBinding<Types...>* GetTypeBinding() const { return m_Binding; }

	static constexpr std::array<uint32_t, sizeof...(Types)> GetTypes()
	{
		return reflection::Type_ids<Types...>();
	}

	static constexpr TypeBinding<Types...>* ReinterpretCast(TypeBindingBase* binding)
	{
		return reinterpret_cast<TypeBinding<Types...>*>(binding);
	}

protected:

	TypeBinding<Types...>* m_Binding{};

};
template <typename Class, typename... U>
concept isBindingSystem = std::is_base_of_v<BindingSystem<U...>, Class>;


template <typename T>
class ViewSystemDynamic final : public ViewSystem<T>
{
public:
	ViewSystemDynamic(const std::string& name, TypeView<T>* typeView, std::function<void(T&)> function, int32_t executionOrder = int32_t(ExecutionTime::Update)) : ViewSystem<T>(name, typeView, executionOrder), m_ExecutingFunction(function) {}

	void Execute() override
	{
		for (auto& element : *ViewSystem<T>::m_TypeView)
		{
			m_ExecutingFunction(element);
		}
	}

private:

	std::function<void(T&)> m_ExecutingFunction;
};

template <typename... Types>
class BindingSystemDynamic final : public BindingSystem<Types...>
{
public:
	BindingSystemDynamic(const std::string& name, TypeBinding<Types...>* typeBinding, std::function<void(Types&...)> function, int32_t executionOrder = int32_t(ExecutionTime::Update)) : BindingSystem<Types...>(name, typeBinding, executionOrder), m_ExecutingFunction(function) {}

	void Execute() override
	{
		for (auto& bind : *BindingSystem<Types...>::m_Binding)
		{
			bind.ApplyFunction(m_ExecutingFunction);
		}
	}

private:

	std::function<void(Types&...)> m_ExecutingFunction;

};