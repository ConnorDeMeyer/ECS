#pragma once
#include <iostream>
#include "TypeInformation/TypeInfoGenerator.h"

class UpdateAbleClass final
{
public:

	constexpr static float UpdateInterval{ 0.5f };
	void Update(float deltaTime)
	{
		std::cout << "deltaTime: " << deltaTime << '\n';
	}


};
inline RegisterClass<UpdateAbleClass> UpdateAbleClassReg;


class BaseClass
{
public:
	std::string name{ "Base Class" };

	void Serialize(std::ostream& stream)
	{
		WriteStream(stream, name.size());
		stream.write(name.data(), name.size());
	}

	void Deserialize(std::istream& stream)
	{
		size_t size{ };
		ReadStream(stream, size);
		name.resize(size);
		stream.read(name.data(), size);
	}
};

class DerivedClass final : public BaseClass
{
public:
	DerivedClass()
	{
		name = "Derived Class";
	}
};
inline RegisterChildClass<BaseClass, DerivedClass> derivedBaseReg;