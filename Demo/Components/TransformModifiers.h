#pragma once
#include <glm/glm.hpp>
#include <random>
#include "TypeInformation/TypeInfoGenerator.h"

struct MoveScaleRotate
{
	glm::vec2 deltaPos;
	glm::vec2 deltaScale;
	float deltaRot;

	void Randomize()
	{
		static std::default_random_engine generator;
		std::uniform_real<float> distribution(-0.25f, 0.25f);
		std::uniform_real<float> distribution2(1.01f, 1.05f);

		deltaPos.x = distribution(generator);
		deltaPos.y = distribution(generator);
		deltaScale.y = deltaScale.x = distribution2(generator);
		deltaRot = distribution(generator) / 25.f;
	}
};

inline RegisterClass<MoveScaleRotate> MoveScaleRotateReg;
