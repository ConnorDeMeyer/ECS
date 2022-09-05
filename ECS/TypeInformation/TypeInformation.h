#pragma once

#include <cassert>
#include <unordered_map>
#include <unordered_set>
#include <functional>

#include "reflection.h"

template <typename To, typename From>
static To* Cast(const From* ptr);

template <typename To, typename From>
static To& Cast(const From& ref);

class TypeInformation
{
private:
	struct Info
	{
		void* m_pVTable{};
		std::string_view m_TypeName;
		size_t m_Size{};
	};

public:

	template <typename T>
	static const Info& GetTypeInfo();

	static const Info& GetTypeInfo(uint32_t typeId);

	template <typename T>
	static void AddClass();

	template <typename Base, typename Child>
	static void AddBaseChildConnection();

	static std::string_view GetTypeName(uint32_t typeId);

	[[nodiscard]] static std::vector<uint32_t> GetSubClasses(uint32_t typeId);

	[[nodiscard]] static std::vector<uint32_t> GetSubClasses(uint32_t* typeIds, size_t size);

	[[nodiscard]] static std::vector<uint32_t> GetTypeCombinations(const uint32_t* typeIds, const size_t size, size_t combinationSize);

	[[nodiscard]] static std::vector<uint32_t> GetSubTypeCombinations(const uint32_t* typeIds, const size_t size);

	[[nodiscard]] static bool IsSubClass(uint32_t base, uint32_t subType);

	//template <typename T>
	//static bool IsSubClass(uint32_t baseClass, T* classInstance);

	//static void AddClass(uint32_t classId, std::string_view name, size_t size);

	//template <typename Class>
	//static void RegisterMemberField();

	static const std::unordered_map<void*, uint32_t>& GetVptrMap() { return GetInstance().m_VptrClassMap; }
	static const std::unordered_map<uint32_t, std::unordered_set<uint32_t>>& GetClassHierarchy() { return GetInstance().m_ClassHierarchy; }
	static std::unordered_map<uint32_t, std::vector<uint32_t>>& GetParentClasses() { return GetInstance().m_ParentClasses; }

private:

	static TypeInformation& GetInstance();

	std::unordered_map<uint32_t, Info> m_TypeInformation;
	std::unordered_map<uint32_t, std::unordered_set<uint32_t>> m_ClassHierarchy;
	std::unordered_map<uint32_t, std::vector<uint32_t>> m_ParentClasses;
	std::unordered_map<void*, uint32_t> m_VptrClassMap;

};

template <typename T>
const TypeInformation::Info& TypeInformation::GetTypeInfo()
{
	return GetTypeInfo(reflection::type_id<T>());
}

template <typename T>
void TypeInformation::AddClass()
{
	static_assert(std::is_class_v<T>);
	auto& instance = GetInstance();
	assert(!instance.m_TypeInformation.contains(reflection::type_id<T>()));

	if (instance.m_TypeInformation.contains(reflection::type_id<T>()))
		return;

	T TypeInstance{};

	Info info{};

	info.m_TypeName = reflection::type_name<T>();
	info.m_Size = sizeof(T);
	
	if constexpr (std::is_polymorphic_v<T>)
	{
		info.m_pVTable = static_cast<void*>(&TypeInstance);
		instance.m_VptrClassMap.emplace(info.m_pVTable, reflection::type_id<T>());
	}

	instance.m_TypeInformation.insert({ reflection::type_id<T>(), std::move(info) });
}

template <typename Base, typename Child>
void TypeInformation::AddBaseChildConnection()
{
	static_assert(std::is_polymorphic_v<Base> && std::is_polymorphic_v<Child> && std::is_base_of_v<Base, Child>);

	auto& instance = GetInstance();

	instance.m_ParentClasses[reflection::type_id<Child>()].emplace_back(reflection::type_id<Base>());
	instance.m_ClassHierarchy[reflection::type_id<Base>()].emplace(reflection::type_id<Child>());
}

template <typename To, typename From>
To* Cast(const From* ptr)
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
		auto it = TypeInformation::GetVptrMap().find(static_cast<void*>(ptr));
		if (it != TypeInformation::GetVptrMap().end())
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

template <typename To, typename From>
To& Cast(const From& ref)
{
	return *Cast<To, From>(&ref);
}