#pragma once
#include "TypeInformation/TypeInfoGenerator.h"

struct Transform
{
	float position[3]{};
};

RegisterClass<Transform> transformReg;