#pragma once
#include "../GameComponent/GameComponent.h"

class GameCompTest : public GameComponent
{
public:
	void Initialize() override
	{
		std::cout << "Initialized\n";
	}
};

class GameCompTest2 : public GameCompTest
{
public:
	void Initialize() override
	{
		std::cout << "Initialized test2\n";
	}

	static void staticTestFunction()
	{
		std::cout << "testing";
	}

	int test{};
};

namespace GameCompTest2Name
{
	static inline void staticTestFunction()
	{
		std::cout << "testing";
	}
}
