#pragma once
#include <random>
#include <glm/glm.hpp>

#include "TypeInformation/TypeInfoGenerator.h"

struct Transform
{
	glm::mat3 transform;

	Transform()
		: transform(
			1,0,0,
			0,1,0,
			0,0,1)
	{}

	/*Transform(const glm::vec2& scale)
		: transform(
			scale.x, 0, 0,
			0, scale.y, 0,
			0,0,0
		)
	{}

	Transform(const glm::vec2& pos)
		: transform(
			1,0,pos.x,
			0,1,pos.y,
			0,0,1
		)
	{}*/

	Transform(const glm::vec2& pos, const glm::vec2& scale)
		: transform(
			scale.x, 0, 0,
			0, scale.y, 0,
			pos.x, pos.y,1
		)
	{}

	void Scale(const glm::vec2& scaling)
	{
		transform[0][0] *= scaling.x;
		transform[1][1] *= scaling.y;
	}
	/*void Rotate(float rotation)
	{
		glm::vec2 pos = { transform[0][2], transform[1][2] };
		transform[0][2] = transform[1][2] = 0;
		glm::mat3 rotation
	}*/
	void Move(const glm::vec2& movement)
	{
		transform[2][0] += movement.x;
		transform[2][1] += movement.y;
	}

	void Randomize()
	{
		static std::default_random_engine generator;
		std::uniform_real<float> distribution(1, 1000);
		std::uniform_real<float> distribution2(1, 400);

		transform[2][0] = distribution(generator);
		transform[2][1] = distribution(generator);
		transform[1][1] = transform[0][0] = distribution2(generator);
	}
};

inline RegisterClass<Transform> TransformReg;