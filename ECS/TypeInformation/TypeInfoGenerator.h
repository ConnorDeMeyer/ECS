#pragma once

#include "TypeInformation.h"

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
