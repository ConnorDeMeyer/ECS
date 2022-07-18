#pragma once
#include "../GameComponent/GameComponent.h"

class GameCompTest : public GameComponent
{
	void Initialize() override
	{
		std::cout << "Initialized\n";
	}
};

class GameCompTest2 : public GameComponent
{
	void Initialize() override
	{
		std::cout << "Initialized test2\n";
	}
};