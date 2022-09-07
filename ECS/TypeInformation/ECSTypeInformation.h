#pragma once
#include <functional>
#include <memory>
#include <cassert>
#include <vector>

#include "../Registry/TypeView.h"
#include "../System/System.h"
#include "../Registry/EntityRegistry.h"
#include "Concepts.h"

class ECSTypeInformation
{
public:
	template <typename T>
	static void AddClass();

	static TypeViewBase* AddTypeView(uint32_t typeId, EntityRegistry* registry);

	template <typename Type>
	static void AddSystem(const SystemParameters& parameters, const std::function<void(Type&)>& function);

	template <typename... Types>
	static void AddSystem(const SystemParameters& parameters, const std::function<void(Types&...)>& function) requires(sizeof...(Types) >= 2);

	template <typename Type>
	static void AddSystem(const SystemParameters& parameters, const std::function<void(float, Type&)>& function);

	template <typename... Types>
	static void AddSystem(const SystemParameters& parameters, const std::function<void(float, Types&...)>& function) requires(sizeof...(Types) >= 2);

	template <typename System>
	static void AddSystem(const SystemParameters& parameters) requires (std::is_base_of_v<SystemBase, System>);

	static void AddDefaultSystems(uint32_t typeId, EntityRegistry* registry);

	static void PrintSystemName(std::ostream& stream);

	//template <typename System>
	//static void AddSystem() requires (std::is_base_of_v<SystemBase, System> && SystemSimpleConstructor<System>);

	static const std::unordered_map<uint32_t, std::function<TypeViewBase* (EntityRegistry*)>>& GetTypeViewAdders() { return GetInstance().TypeViewAdder; }
	static const std::unordered_map<std::string, std::function<SystemBase*(EntityRegistry*)>>& GetSystemAdders() { return GetInstance().SystemAdder; }

private:

	template <typename T> void RegisterUpdateableClass		();
	template <typename T> void RegisterPreUpdateableClass	();
	template <typename T> void RegisterPostUpdateableClass	();
	template <typename T> void RegisterRenderableClass		();
	template <typename T> void RegisterLateRenderableClass	();

private:

	static ECSTypeInformation& GetInstance()
	{
		static ECSTypeInformation info{};
		return info;
	}

	std::unordered_map<uint32_t, std::function<TypeViewBase* (EntityRegistry*)>> TypeViewAdder;
	std::unordered_map<std::string, std::function<SystemBase* (EntityRegistry*)>> SystemAdder;
	std::unordered_map<uint32_t, std::vector<std::function<void(EntityRegistry*)>>> DefaultSystemAdders;
};


template <typename T>
void ECSTypeInformation::AddClass()
{
	auto& instance = GetInstance();
	instance.TypeViewAdder.emplace(reflection::type_id<T>(), [](EntityRegistry* reg)
		{
			return &reg->AddView<T>();
		});

	if constexpr (Updateable<T>)		instance.RegisterUpdateableClass<T>();
	if constexpr (PreUpdateable<T>)		instance.RegisterPreUpdateableClass<T>();
	if constexpr (LateUpdateable<T>)	instance.RegisterPostUpdateableClass<T>();
	if constexpr (Renderable<T>)		instance.RegisterRenderableClass<T>();
	if constexpr (LateRenderable<T>)	instance.RegisterLateRenderableClass<T>();
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

template <typename Type>
void ECSTypeInformation::AddSystem(const SystemParameters& parameters,
	const std::function<void(float, Type&)>& function)
{
	GetInstance().SystemAdder.emplace(parameters.name, [parameters, function](EntityRegistry* reg)
		{
			return reg->AddSystem<Type>(parameters, function);
		});
}

template <typename ... Types>
void ECSTypeInformation::AddSystem(const SystemParameters& parameters,
	const std::function<void(float, Types&...)>& function) requires (sizeof...(Types) >= 2)
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

template <typename T>
void ECSTypeInformation::RegisterUpdateableClass()
{
	constexpr uint32_t typeId{ reflection::type_id<T>() };
	DefaultSystemAdders[typeId].emplace_back([](EntityRegistry* reg)
	{
		constexpr std::string_view typeName{ reflection::type_name<T>() };
		SystemBase* system = reg->AddSystem<T>(
			SystemParameters{
				std::string(typeName) + "_Update",
				int32_t(ExecutionTime::Update),
				(UpdateTimeInterval<T>) ? T::UpdateInterval : 0.f // If the class has a static member variable called UpdateInterval, use that one
			},
			[](float deltaTime, T& t)
			{
				t.Update(deltaTime);
			}, 
			false); // Dont add subsystems as they will automatically contain the function (unless base class is privated)
		system->SetFlag(SystemFlags::DefaultSystem, true);
	});
}

template <typename T>
void ECSTypeInformation::RegisterPreUpdateableClass()
{
	constexpr uint32_t typeId{ reflection::type_id<T>() };
	DefaultSystemAdders[typeId].emplace_back([](EntityRegistry* reg)
		{
			constexpr std::string_view typeName{ reflection::type_name<T>() };
			SystemBase* system = reg->AddSystem<T>(
				SystemParameters{
					std::string(typeName) + "_PreUpdate",
					int32_t(ExecutionTime::PreUpdate),
					(PreUpdateTimeInterval<T>) ? T::PreUpdateInterval : 0.f // If the class has a static member variable called UpdateInterval, use that one
				},
				[](float deltaTime, T& t)
				{
					t.PreUpdate(deltaTime);
				},
				false); // Dont add subsystems as they will automatically contain the function (unless base class is privated)
			system->SetFlag(SystemFlags::DefaultSystem, true);
		});
}

template <typename T>
void ECSTypeInformation::RegisterPostUpdateableClass()
{
	constexpr uint32_t typeId{ reflection::type_id<T>() };
	DefaultSystemAdders[typeId].emplace_back([](EntityRegistry* reg)
		{
			constexpr std::string_view typeName{ reflection::type_name<T>() };
			SystemBase* system = reg->AddSystem<T>(
				SystemParameters{
					std::string(typeName) + "_LateUpdate",
					int32_t(ExecutionTime::LateUpdate),
					(LateUpdateTimeInterval<T>) ? T::LateUpdateInterval : 0.f // If the class has a static member variable called UpdateInterval, use that one
				},
				[](float deltaTime, T& t)
				{
					t.LateUpdate(deltaTime);
				},
				false); // Dont add subsystems as they will automatically contain the function (unless base class is privated)
			system->SetFlag(SystemFlags::DefaultSystem, true);
		});
}

template <typename T>
void ECSTypeInformation::RegisterRenderableClass()
{
	constexpr uint32_t typeId{ reflection::type_id<T>() };
	DefaultSystemAdders[typeId].emplace_back([](EntityRegistry* reg)
		{
			constexpr std::string_view typeName{ reflection::type_name<T>() };
			SystemBase* system = reg->AddSystem<T>(
				SystemParameters{
					std::string(typeName) + "_Render",
					int32_t(ExecutionTime::Render),
					(RenderTimeInterval<T>) ? T::RenderInterval : 0.f // If the class has a static member variable called UpdateInterval, use that one
				},
				[](T& t)
				{
					t.Render();
				},
				false); // Dont add subsystems as they will automatically contain the function (unless base class is privated)
			system->SetFlag(SystemFlags::DefaultSystem, true);
		});
}

template <typename T>
void ECSTypeInformation::RegisterLateRenderableClass()
{
	constexpr uint32_t typeId{ reflection::type_id<T>() };
	DefaultSystemAdders[typeId].emplace_back([](EntityRegistry* reg)
		{
			constexpr std::string_view typeName{ reflection::type_name<T>() };
			SystemBase* system = reg->AddSystem<T>(
				SystemParameters{
					std::string(typeName) + "_LateRender",
					int32_t(ExecutionTime::LateRender),
					(LateRenderTimeInterval<T>) ? T::LateRenderInterval : 0.f // If the class has a static member variable called UpdateInterval, use that one
				},
				[](T& t)
				{
					t.LateRender();
				},
				false); // Dont add subsystems as they will automatically contain the function (unless base class is privated)
			system->SetFlag(SystemFlags::DefaultSystem, true);
		});
}

inline TypeViewBase* ECSTypeInformation::AddTypeView(uint32_t typeId, EntityRegistry* registry)
{
	assert(GetInstance().TypeViewAdder.contains(typeId));
	return GetInstance().TypeViewAdder.find(typeId)->second(registry);
}

inline void ECSTypeInformation::AddDefaultSystems(uint32_t typeId, EntityRegistry* registry)
{
	auto& instance = GetInstance();
	for (auto& adder : instance.DefaultSystemAdders[typeId])
	{
		adder(registry);
	}
}

inline void ECSTypeInformation::PrintSystemName(std::ostream& stream)
{
	for (auto& system : GetSystemAdders())
	{
		stream << system.first << '\n';
	}
}
