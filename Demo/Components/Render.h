#pragma once
#include <random>
#include <glm/glm.hpp>
#include "TypeInformation/TypeInfoGenerator.h"

struct Render
{
	constexpr static float UpdateInterval{ 1.f };

	glm::mat3 Transform{ 100,0,0,0,100,0,200,200,1 };
	glm::vec4 Color{ 1,1,1,1 };
	glm::vec4 Uvs{ 0,1,0,1 }; // {x_start, x_end, y_start, y_end}
	glm::vec2 Pivot{ 0,0 };
	float Depth{0.5f};
	uint32_t textureId{};

	void Randomize()
	{
		static std::default_random_engine generator;
		std::uniform_real<float> distribution(0, 1);
		std::uniform_real<float> distribution2(-2, 2);

		Color.x = distribution(generator);
		Color.y = distribution(generator);
		Color.z = distribution(generator);
		Color.w = distribution(generator);

		Pivot.x = distribution2(generator);
		Pivot.y = distribution2(generator);
		
		Depth = distribution(generator);
	}
};

inline RegisterClass<Render> RenderReg;