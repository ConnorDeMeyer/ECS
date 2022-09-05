﻿
//#define SYSTEM_PROFILER

#include <Registry/EntityRegistry.h>

#include "RenderingInput/SDLOpenGl.h"
#include "Entity/GameObject.h"

#include <vld.h>

#include "Components/Render.h"
#include "Components/RenderModifiers.h"
#include "Components/Transform.h"
#include "Components/TransformModifiers.h"

#include "Systems/DynamicSystems.h"

#include <SDL.h>

void ProcessInput();


int main(int, char* [])
{
	int result = OpenGl::Initialize();
	if (result != 0)
		return result;

	std::cout << "Press P to print System Stats\n";
	std::cout << "Press O to serialize the registry\n";

	EntityRegistry registry;

	SDL::SetInputCallback(SDLK_p, [&registry]() {registry.PrintSystemInformation(); });
	SDL::SetInputCallback(SDLK_o, [&registry]()
		{
			std::ofstream stream("DemoSerialized");
			registry.Serialize(stream);
			std::cout << "Serialized to \"DemoSerialized\"";
		});

#ifdef REGISTRY_DESERIALIZE

	std::ifstream stream("DemoSerialized");
	registry.Deserialize(stream);

#else

	registry.AddSystem("RenderingSystem");
	registry.AddSystem("RenderTransformUpdate");
	registry.AddSystem("RenderModifier");
	registry.AddSystem("MoveScaleRotate");
	registry.AddSystem("PositionModulo");

	std::vector<GameObject> objects;
	constexpr size_t entitiesAmount{ 16'384 };
	objects.reserve(entitiesAmount);
	for (size_t i{}; i < entitiesAmount; ++i)
	{
		auto& object = objects.emplace_back(registry);
		auto render = object.AddComponent<Render>();
		auto transform = object.AddComponent<Transform>();
		auto transformMod = object.AddComponent<MoveScaleRotate>();
		auto renderMod = object.AddComponent<RenderModifiers>();

		transformMod->Randomize();
		renderMod->Randomize();
		transform->Randomize();
		render->Randomize();
	}

#endif

	SDL::SetUpdateCallback([&registry](float deltaTime)
		{
			registry.Update(deltaTime);
		});

	SDL::StartLoop();
	return 0;
}