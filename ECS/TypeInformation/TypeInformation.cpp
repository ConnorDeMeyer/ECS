#include "TypeInformation.h"

std::string_view TypeInformation::GetTypeName(uint32_t typeId)
{
	assert(GetInstance().m_TypeInformation.contains(typeId));
	return GetInstance().m_TypeInformation.find(typeId)->second.m_TypeName;
}

[[nodiscard]] std::vector<uint32_t> TypeInformation::GetSubClasses(uint32_t typeId)
{
	auto& instance = GetInstance();

	std::vector<uint32_t> subClasses;
	size_t processedCounter{};

	for (auto& childId : instance.m_ClassHierarchy[typeId])
		subClasses.emplace_back(childId);

	while (subClasses.size() != processedCounter)
	{
		for (auto& childId : instance.m_ClassHierarchy[subClasses[processedCounter]])
		{
			subClasses.emplace_back(childId);
		}
		++processedCounter;
	}

	return subClasses;
}

const TypeInformation::Info& TypeInformation::GetTypeInfo(uint32_t typeId)
{
	assert(GetInstance().m_TypeInformation.contains(typeId));
	return GetInstance().m_TypeInformation[typeId];
}

std::vector<uint32_t> TypeInformation::GetSubClasses(uint32_t* typeIds, size_t size)
{
	auto& instance = GetInstance();

	std::vector<uint32_t> subClasses;

	for (size_t i{}; i < size; ++i)
	{
		size_t processedCounter{};

		for (auto& childId : instance.m_ClassHierarchy[typeIds[i]])
			subClasses.emplace_back(childId);

		while (subClasses.size() != processedCounter)
		{
			for (auto& childId : instance.m_ClassHierarchy[subClasses[processedCounter]])
			{
				subClasses.emplace_back(childId);
			}
			++processedCounter;
		}
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
void CreateCombinationsRecursive(const uint32_t* arr, std::vector<uint32_t>& outData,
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
	for (size_t i{ start }; i <= end && end - i + 1 >= r - index; ++i)
	{
		data[index] = arr[i];
		CreateCombinationsRecursive(arr, outData, data, i + 1, end, index + 1, r);
	}
}


[[nodiscard]] std::vector<uint32_t> TypeInformation::GetTypeCombinations(const uint32_t* typeIds, const size_t size, size_t combinationSize)
{
	size_t combinationAmount = nChoosek(size, combinationSize);
	std::vector<uint32_t> types;
	types.reserve(combinationAmount * combinationSize);

	std::unique_ptr<uint32_t[]> data = std::make_unique<uint32_t[]>(combinationSize);

	CreateCombinationsRecursive(typeIds, types, data.get(), 0, size - 1, 0, combinationSize);

	return types;
}

void SubTypeCombinationsRecursive(const std::vector<uint32_t>* subTypes, size_t size, uint32_t* buffer,
	std::vector<uint32_t>& outCombinations, size_t depth)
{
	if (depth == size)
		return;

	for (uint32_t typeId : subTypes[depth])
	{
		buffer[depth] = typeId;
		SubTypeCombinationsRecursive(subTypes, size, buffer, outCombinations, depth + 1);

		if (depth == size - 1)
		{
			for (size_t i{}; i < size; ++i)
				outCombinations.emplace_back(buffer[i]);
		}
	}
}

std::vector<uint32_t> TypeInformation::GetSubTypeCombinations(const uint32_t* typeIds, const size_t size)
{
	std::unique_ptr<std::vector<uint32_t>[]> subTypes = std::make_unique<std::vector<uint32_t>[]>(size);
	for (size_t i{}; i < size; ++i)
	{
		subTypes[i] = GetSubClasses(typeIds[i]);
		subTypes[i].emplace_back(typeIds[i]);
	}

	std::vector<uint32_t> Combinations;
	std::unique_ptr<uint32_t[]> buffer = std::make_unique<uint32_t[]>(size);

	SubTypeCombinationsRecursive(subTypes.get(), size, buffer.get(), Combinations, 0);

	for (size_t i{}; i < size; ++i)
		Combinations.pop_back();

	return Combinations;
}

bool RecursiveParentSearch(uint32_t goal, uint32_t current)
{
	for (auto& parent : TypeInformation::GetParentClasses()[current])
	{
		if (parent == goal)
			return true;

		if (RecursiveParentSearch(goal, parent))
			return true;
	}
	return false;
}

bool TypeInformation::IsSubClass(uint32_t base, uint32_t subType)
{
	return RecursiveParentSearch(base, subType);
}

TypeInformation& TypeInformation::GetInstance()
{
	static TypeInformation info{};
	return info;
}
