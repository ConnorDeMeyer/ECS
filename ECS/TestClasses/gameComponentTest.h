#pragma once

class GameCompTest
{
public:
	virtual void Initialize()
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

RegisterClass<GameCompTest> gameCompReg;
RegisterClass<GameCompTest2> gameComp2Reg;
RegisterChildClass<GameCompTest, GameCompTest2> gamecomptest2;

namespace GameCompTest2Name
{
	static inline void staticTestFunction()
	{
		std::cout << "testing";
	}
}
