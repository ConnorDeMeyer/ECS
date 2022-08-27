#pragma once

#include <cassert>
#include <unordered_map>
#include <functional>

#include "reflection.h"

template <typename To, typename From>
static To* Cast(From* ptr);

namespace TypeInformation
{

	struct Info
	{
		void* m_pVTable{};
		std::string_view m_TypeName;
		size_t m_Size{};
		std::function<void(EntityRegistry*)> m_ViewAdder;
	};

	template <typename T>
	static const Info& GetTypeInfo();

	static const Info& GetTypeInfo(uint32_t typeId);

	template <typename T>
	static void AddClass();

	template <typename Base, typename Child>
	static void AddBaseChildConnection();

	//template <typename T>
	//static bool IsSubClass(uint32_t baseClass, T* classInstance);

	//static void AddClass(uint32_t classId, std::string_view name, size_t size);

	//template <typename Class>
	//static void RegisterMemberField();

	

	namespace Data
	{
		inline static std::unordered_map<uint32_t, Info> TypeInformation;
		inline static std::unordered_map<uint32_t, std::unordered_set<uint32_t>> ClassHierarchy;
		inline static std::unordered_map<uint32_t, std::vector<uint32_t>> ParentClasses;
		inline static std::unordered_map<void*, uint32_t> VptrClassMap;
	}


};

template <typename T>
const TypeInformation::Info& TypeInformation::GetTypeInfo()
{
	return GetTypeInfo(reflection::type_id<T>());
}

inline const TypeInformation::Info& TypeInformation::GetTypeInfo(uint32_t typeId)
{
	assert(Data::TypeInformation.contains(typeId));
	return Data::TypeInformation[typeId];
}

template <typename T>
void TypeInformation::AddClass()
{
	static_assert(std::is_class_v<T>);
	assert(!Data::TypeInformation.contains(reflection::type_id<T>()));

	if (Data::TypeInformation.contains(reflection::type_id<T>()))
		return;

	T instance{};

	Info info{};

	info.m_TypeName = reflection::type_name<T>();
	info.m_Size = sizeof(T);
	
	if constexpr (std::is_polymorphic_v<T>)
	{
		info.m_pVTable = static_cast<void*>(&instance);
		Data::VptrClassMap.emplace(info.m_pVTable, reflection::type_id<T>());
	}

	Data::TypeInformation.insert({ reflection::type_id<T>(), std::move(info) });
}

template <typename Base, typename Child>
void TypeInformation::AddBaseChildConnection()
{
	static_assert(std::is_polymorphic_v<Base> && std::is_polymorphic_v<Child> && std::is_base_of_v<Base, Child>);

	Data::ParentClasses[reflection::type_id<Child>()].emplace_back(reflection::type_id<Base>());
	Data::ClassHierarchy[reflection::type_id<Base>()].emplace(reflection::type_id<Child>());
}

//template <typename T>
//bool TypeInformation::IsSubClass(uint32_t baseClass, T* classInstance)
//{
//	static_assert(std::is_polymorphic_v<T>);
//
//	T instance{};
//
//	auto it = Data::TypeInformation.find(baseClass);
//	if (it != Data::TypeInformation.end())
//	{
//		assert(it->second.m_pVTable);
//	}
//
//	return false;
//}

inline bool RecursiveParentSearch(uint32_t goal, uint32_t current)
{
	for (auto& parent : TypeInformation::Data::ParentClasses[current])
	{
		if (parent == goal)
			return true;
		else
			return RecursiveParentSearch(goal, parent);
	}
	return false;
}

template <typename To, typename From>
To* Cast(From* ptr)
{
	if (!ptr)
		return static_cast<To*>(nullptr);

	if constexpr (std::is_same_v<std::remove_cv<To>, From>)
	{
		return static_cast<To*>(ptr);
	}

	else if constexpr (std::is_base_of_v<To, From>)
	{
		return static_cast<To*>(ptr);
	}

	else if constexpr (std::is_same_v<void,To>)
	{
		return static_cast<void*>(ptr);
	}

	else if constexpr (std::is_base_of_v<From, To>)
	{
		auto it = TypeInformation::Data::VptrClassMap.find(static_cast<void*>(ptr));
		if (it != TypeInformation::Data::VptrClassMap.end())
		{
			uint32_t classId = it->second;
			if (RecursiveParentSearch(reflection::type_id<To>(), classId))
			{
				return reinterpret_cast<To*>(ptr);
			}
		}
	}

	return nullptr;
}
