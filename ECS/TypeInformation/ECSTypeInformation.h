#pragma once
#include <functional>
#include <memory>

#include "../Registry/TypeView.h"
#include "../System/System.h"
#include "../Registry/EntityRegistry.h"

namespace TypeInformation
{
	template <typename T>
	static void AddTypeViewClass();

	static TypeViewBase* AddTypeView(uint32_t typeId, EntityRegistry* registry);

	namespace EcsData
	{
		inline static std::unordered_map<int32_t, std::function<TypeViewBase*(EntityRegistry*)>> TypeViewAdder;
		inline static std::unordered_map<int32_t, std::function<std::unique_ptr<SystemBase>()>> SystemCreator;
	}
}

template <typename T>
void TypeInformation::AddTypeViewClass()
{
	EcsData::TypeViewAdder.emplace(reflection::type_id<T>(), [](EntityRegistry* reg)
		{
			return &reg->AddView<T>();
		});
}

inline TypeViewBase* TypeInformation::AddTypeView(uint32_t typeId, EntityRegistry* registry)
{
	assert(EcsData::TypeViewAdder.contains(typeId));
	return EcsData::TypeViewAdder.find(typeId)->second(registry);
}