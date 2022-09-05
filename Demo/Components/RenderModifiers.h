#pragma once
#include <random>
#include <glm/glm.hpp>
#include "TypeInformation/TypeInfoGenerator.h"

struct RenderModifiers
{
	glm::vec4 deltaColor;
	glm::vec2 deltaPivot;
	float deltaDepth;

	void Randomize()
	{
		static std::default_random_engine generator;
		std::uniform_real<float> distribution(-0.01f, 0.01f);

		deltaColor.x = distribution(generator);
		deltaColor.y = distribution(generator);
		deltaColor.z = distribution(generator);
		deltaColor.w = distribution(generator);
		deltaPivot.x = distribution(generator);
		deltaPivot.y = distribution(generator);
		deltaDepth = distribution(generator);
	}
};

inline RegisterClass<RenderModifiers> RenderModifiersReg;