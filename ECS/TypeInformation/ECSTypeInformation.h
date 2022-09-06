#pragma once
#include <functional>
#include <memory>
#include <cassert>

#include "../Registry/TypeView.h"
#include "../System/System.h"
#include "../Registry/EntityRegistry.h"

class ECSTypeInformation
{
public:
	template <typename T>
	static void AddTypeViewClass();

	static TypeViewBase* AddTypeView(uint32_t typeId, EntityRegistry* registry);

	template <typename Type>
	static void AddSystem(const SystemParameters& parameters, const std::function<void(Type&)>& function);

	template <typename... Types>
	static void AddSystem(const SystemParameters& parameters, const std::function<void(Types&...)>& function) requires(sizeof...(Types) >= 2);

	template <typename System>
	static void AddSystem(const SystemParameters& parameters) requires (std::is_base_of_v<SystemBase, System>);

	static void PrintSystemName(std::ostream& stream);

	//template <typename System>
	//static void AddSystem() requires (std::is_base_of_v<SystemBase, System> && SystemSimpleConstructor<System>);

	static const std::unordered_map<uint32_t, std::function<TypeViewBase* (EntityRegistry*)>>& GetTypeViewAdders() { return GetInstance().TypeViewAdder; }
	static const std::unordered_map<std::string, std::function<void(EntityRegistry*)>>& GetSystemAdders() { return GetInstance().SystemAdder; }

private:

	static ECSTypeInformation& GetInstance()
	{
		static ECSTypeInformation info{};
		return info;
	}

	std::unordered_map<uint32_t, std::function<TypeViewBase* (EntityRegistry*)>> TypeViewAdder;
	std::unordered_map<std::string, std::function<void(EntityRegistry*)>> SystemAdder;
};


template <typename T>
void ECSTypeInformation::AddTypeViewClass()
{
	GetInstance().TypeViewAdder.emplace(reflection::type_id<T>(), [](EntityRegistry* reg)
		{
			return &reg->AddView<T>();
		});
}

template <typename Type>
void ECSTypeInformation::AddSystem(const SystemParameters& parameters, const std::function<void(Type&)>& function)
{
	GetInstance().SystemAdder.emplace(parameters.name, [parameters, function](EntityRegistry* reg)
		{
			return reg->AddSystem<Type>(parameters, function);
		});
}

template <typename ... Types>
void ECSTypeInformation::AddSystem(const SystemParameters& parameters, const std::function<void(Types&...)>& function) requires (sizeof...(Types) >= 2)
{
	GetInstance().SystemAdder.emplace(parameters.name, [parameters, function](EntityRegistry* reg)
		{
			return reg->AddSystem<Types...>(parameters, function);
		});
}

template <typename System>
void ECSTypeInformation::AddSystem(const SystemParameters& parameters) requires (std::is_base_of_v<SystemBase,System>)
{
	GetInstance().SystemAdder.emplace(parameters.name, [parameters](EntityRegistry* reg)
		{
			return reg->AddSystem<System>(parameters);
		});
}

//template <typename System>
//void TypeInformation::AddSystem() requires (std::is_base_of_v<SystemBase, System> && SystemSimpleConstructor<System>)
//{
//	EcsData::SystemAdder.emplace(name, [name, function, executionOrder](EntityRegistry* reg)
//		{
//			return reg->AddSystem<System>(name, function, executionOrder);
//		});
//}

inline TypeViewBase* ECSTypeInformation::AddTypeView(uint32_t typeId, EntityRegistry* registry)
{
	assert(GetInstance().TypeViewAdder.contains(typeId));
	return GetInstance().TypeViewAdder.find(typeId)->second(registry);
}

inline void ECSTypeInformation::PrintSystemName(std::ostream& stream)
{
	for (auto& system : GetSystemAdders())
	{
		stream << system.first << '\n';
	}
}
