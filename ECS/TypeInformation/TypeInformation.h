#pragma once

#include <cassert>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <type_traits>

#include "Concepts.h"
#include "reflection.h"

/** Used instead of RegisterMemberVariable method of the ClassMemberAdder class*/
#define RegisterMemberVar(memVar) RegisterMemberVariable(#memVar, sizeof(decltype(memVar)), offsetof(std::remove_reference_t<decltype(*this)>,memVar), reflection::type_id<decltype(memVar)>())

template <typename To, typename From>
static To* Cast(const From* ptr);

template <typename To, typename From>
static To& Cast(const From& ref);

struct ClassFieldInfo
{
	std::string name;
	size_t size;
	size_t offset;
	uint32_t typeId;
};

using ClassFieldInfoMap = std::unordered_map<uint32_t, std::unordered_map<std::string, ClassFieldInfo>>;

class ClassMemberAdder
{
private:
	friend class TypeInformation;

	ClassMemberAdder(uint32_t classId, ClassFieldInfoMap& map) : m_ClassId(classId), m_Map(map) {}

public:

	void RegisterMemberVariable(const std::string& name, size_t size, size_t offset, uint32_t typeId);

	template <typename T>
	void RegisterMemberVariable(const std::string& name, size_t offset)
	{
		RegisterMemberVariable(name, sizeof(T), offset, reflection::type_id<T>());
	}

private:

	uint32_t m_ClassId{};
	ClassFieldInfoMap& m_Map;

};

class TypeInformation
{
private:
	struct Info
	{
		void* m_pVTable{};
		std::string m_TypeName;
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

	static const std::string& GetTypeName(uint32_t typeId);

	[[nodiscard]] static std::vector<uint32_t> GetSubClasses(uint32_t typeId);

	[[nodiscard]] static std::vector<uint32_t> GetSubClasses(uint32_t* typeIds, size_t size);

	[[nodiscard]] static std::vector<uint32_t> GetTypeCombinations(const uint32_t* typeIds, const size_t size, size_t combinationSize);

	[[nodiscard]] static std::vector<uint32_t> GetSubTypeCombinations(const uint32_t* typeIds, const size_t size);

	[[nodiscard]] static bool IsSubClass(uint32_t base, uint32_t subType);

	static std::unordered_map<std::string, ClassFieldInfo>& GetFieldInfo(uint32_t typeId);

	//template <typename T>
	//static bool IsSubClass(uint32_t baseClass, T* classInstance);

	//static void AddClass(uint32_t classId, std::string_view name, size_t size);

	//template <typename Class>
	//static void RegisterMemberField();

	static const std::unordered_map<void*, uint32_t>& GetVptrMap() { return GetInstance().m_VptrClassMap; }
	static const std::unordered_map<uint32_t, std::unordered_set<uint32_t>>& GetClassHierarchy() { return GetInstance().m_ClassHierarchy; }
	static std::unordered_map<uint32_t, std::vector<uint32_t>>& GetParentClasses() { return GetInstance().m_ParentClasses; }
	static const std::unordered_map<uint32_t, Info>& GetAllTypeInformation() { return GetInstance().m_TypeInformation; }

private:

	static TypeInformation& GetInstance();

	std::unordered_map<uint32_t, Info> m_TypeInformation;
	std::unordered_map<uint32_t, std::unordered_set<uint32_t>> m_ClassHierarchy;
	std::unordered_map<uint32_t, std::vector<uint32_t>> m_ParentClasses;
	std::unordered_map<void*, uint32_t> m_VptrClassMap;

	ClassFieldInfoMap m_ClassFieldInfo;

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

	constexpr uint32_t typeId{ reflection::type_id<T>() };

	T TypeInstance{};

	Info info{};

	info.m_TypeName = std::string(reflection::type_name<T>());
	info.m_Size = sizeof(T);
	
	if constexpr (std::is_polymorphic_v<T>)
	{
		info.m_pVTable = *reinterpret_cast<void**>(&TypeInstance);
		instance.m_VptrClassMap.emplace(info.m_pVTable, typeId);
	}

	if constexpr (HasMemberInfo<T>)
	{
		ClassMemberAdder adder(typeId, instance.m_ClassFieldInfo);
		TypeInstance.RegisterMemberInfo(adder);
	}

	instance.m_TypeInformation.insert({ typeId, std::move(info) });
}

template <typename Base, typename Child>
void TypeInformation::AddBaseChildConnection()
{
	static_assert(std::is_base_of_v<Base, Child>);

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