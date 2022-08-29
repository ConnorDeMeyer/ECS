#pragma once
#include <functional>
#include <memory>

#include "../Registry/TypeView.h"
#include "../System/System.h"


namespace TypeInformation
{
	template <typename T>
	static void AddTypeViewClass();

	namespace EcsData
	{
		inline static std::unordered_map<int32_t, std::function<std::unique_ptr<TypeViewBase>()>> TypeViewCreator;
		inline static std::unordered_map<int32_t, std::function<std::unique_ptr<SystemBase>()>> SystemCreator;
	}
}

template <typename T>
void TypeInformation::AddTypeViewClass()
{
	EcsData::TypeViewCreator.emplace(reflection::type_id<T>(), [] () -> TypeViewBase* { return new TypeView<T>; });
}