#pragma once

#include <cassert>
#include <unordered_map>
#include <functional>

#include "reflection.h"

template <typename To, typename From>
static To* Cast(const From* ptr);

template <typename To, typename From>
static To& Cast(const From& ref);

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

	[[nodiscard]] std::vector<uint32_t> GetSubClasses(uint32_t typeId);

	[[nodiscard]] std::vector<uint32_t> GetTypeCombinations(const uint32_t* typeIds, const size_t size, size_t combinationSize);

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

[[nodiscard]] inline std::vector<uint32_t> TypeInformation::GetSubClasses(uint32_t typeId)
{
	std::vector<uint32_t> subClasses;
	size_t processedCounter{};

	for (auto& childId : Data::ClassHierarchy[typeId])
		subClasses.emplace_back(childId);

	while (subClasses.size() != processedCounter)
	{
		for (auto& childId : Data::ClassHierarchy[subClasses[processedCounter]])
		{
			subClasses.emplace_back(childId);
		}
		++processedCounter;
	}

	return subClasses;
}

//constexpr size_t factorial(size_t num)
//{
//	return num ? 1 : factorial(num - 1) * num;
//}
//
//constexpr size_t factorial_max(size_t max, size_t min)
//{
//	return max == min ? min : factorial_max(max - 1, min);
//}

//https://stackoverflow.com/questions/9330915/number-of-combinations-n-choose-r-in-c
constexpr size_t nChoosek(size_t n, size_t k)
{
	if (k > n) return 0;
	if (k * 2 > n) k = n - k;
	if (k == 0) return 1;

	size_t result = n;
	for (size_t i = 2; i <= k; ++i) {
		result *= (n - i + 1);
		result /= i;
	}
	return result;
}

//https://www.geeksforgeeks.org/print-all-possible-combinations-of-r-elements-in-a-given-array-of-size-n/
/* arr[] ---> Input Array
data[] ---> Temporary array to
store current combination
start & end ---> Starting and
Ending indexes in arr[]
index ---> Current index in data[]
r ---> Size of a combination to be printed */
inline void CreateCombinationsRecursive(const uint32_t* arr, std::vector<uint32_t>& outData, 
	uint32_t* data, size_t start, size_t end, size_t index, size_t r)
{
	// Current combination is ready
	// to be printed, print it
	if (index == r)
	{
		for (size_t i{ 0 }; i < r; ++i)
			outData.push_back(data[i]);
		return;
	}

	// replace index with all possible
	// elements. The condition "end-i+1 >= r-index"
	// makes sure that including one element
	// at index will make a combination with
	// remaining elements at remaining positions
	for (size_t i{start}; i <= end && end - i + 1 >= r - index; ++i)
	{
		data[index] = arr[i];
		CreateCombinationsRecursive(arr, outData, data, i + 1, end, index + 1, r);
	}
}


[[nodiscard]] inline std::vector<uint32_t> TypeInformation::GetTypeCombinations(const uint32_t* typeIds, const size_t size, size_t combinationSize)
{
	size_t combinationAmount = nChoosek(size, combinationSize);
	std::vector<uint32_t> types;
	types.reserve(combinationAmount * combinationSize);

	std::unique_ptr<uint32_t[]> data = std::make_unique<uint32_t[]>(combinationSize);

	CreateCombinationsRecursive(typeIds, types, data.get(), 0, size - 1, 0, combinationSize);

	return types;
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

template <typename To, typename From>
To& Cast(const From& ref)
{
	return *Cast<To, From>(&ref);
}