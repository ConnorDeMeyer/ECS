#pragma once

#include <unordered_map>

#include "TypeInformation.h"
#include "ECSTypeInformation.h"

//template <typename T>
//concept MemberFielsInfo = requires(T val) { T::TypeInfo_RegisterFields(); };

template <typename T>
class RegisterClass final
{
private:
	class ClassInformationGenerator final
	{
	public:
		ClassInformationGenerator()
		{
			TypeInformation::AddClass<T>();
			ECSTypeInformation::AddClass<T>();
			//if constexpr (MemberFielsInfo<T>)	T::TypeInfo_RegisterFields();
		}
	};
	inline static ClassInformationGenerator Generator{};
};

template <typename Base, typename Child>
class RegisterChildClass final
{
private:
	class ChildInformationGenerator final
	{
	private:
		RegisterClass<Base> base;
		RegisterClass<Child> child;

	public:
		ChildInformationGenerator()
		{
			TypeInformation::AddBaseChildConnection<Base, Child>();
		}
	};
	inline static ChildInformationGenerator Generator{};
};

template <typename System>
class RegisterSystem final
{
public:
	RegisterSystem(const SystemParameters& parameters)
	{
		std::cout << "Registering " << parameters.name << '\n';
		auto it = Generator.find(parameters.name);
		if (it == Generator.end())
		{
			Generator.emplace(parameters.name, SystemInformationGenerator{ parameters });
		}
	}

private:
	class SystemInformationGenerator final
	{
	public:
		SystemInformationGenerator(const SystemParameters& parameters)
		{
			ECSTypeInformation::AddSystem<System>(parameters);
		}
	};
	inline static std::unordered_map<std::string, SystemInformationGenerator> Generator{};
};

template <typename... Types>
class RegisterDynamicSystem final
{
public:
	RegisterDynamicSystem(const SystemParameters& parameters, const std::function<void(Types&...)>& function)
	{
		std::cout << "Registering " << parameters.name << '\n';
		auto it = Generator.find(parameters.name);
		if (it == Generator.end())
		{
			Generator.emplace(parameters.name, SystemInformationGenerator{ parameters, function });
		}
	}

	RegisterDynamicSystem(const SystemParameters& parameters, const std::function<void(float, Types&...)>& function)
	{
		std::cout << "Registering " << parameters.name << '\n';
		auto it = Generator.find(parameters.name);
		if (it == Generator.end())
		{
			Generator.emplace(parameters.name, SystemInformationGenerator{ parameters, function });
		}
	}

private:
	class SystemInformationGenerator final
	{
	public:
		SystemInformationGenerator(const SystemParameters& parameters, const std::function<void(Types&...)>& function)
		{
			ECSTypeInformation::AddSystem(parameters, function);
		}
	};
	inline static std::unordered_map<std::string, SystemInformationGenerator> Generator{};
};