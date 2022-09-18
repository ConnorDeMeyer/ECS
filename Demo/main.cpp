
//#define SYSTEM_PROFILER

#include <Registry/EntityRegistry.h>

#include "RenderingInput/SDLOpenGl.h"
#include "Entity/GameObject.h"

#include <vld.h>

#include <SDL.h>

#include "GUI_main.h"

//#define REGISTRY_DESERIALIZE
#ifndef REGISTRY_DESERIALIZE
#include "Components/Render.h"
#include "Components/RenderModifiers.h"
#include "Components/Transform.h"
#include "Components/TransformModifiers.h"
#include "Components/TestClasses.h"
#endif

void ProcessInput();


int main(int, char* [])
{
	int result = OpenGl::Initialize();
	if (result != 0)
		return result;

	GUI::SetWindow(OpenGl::GetWindow());
	GUI::InitializeImGui();

	std::cout << "Press P to print System Stats\n";
	std::cout << "Press O to serialize the registry\n";

	EntityRegistry registry;

	SDL::SetInputCallback(SDLK_p, [&registry]() {registry.PrintSystemInformation(); });
	SDL::SetInputCallback(SDLK_o, [&registry]()
		{
			std::ofstream stream("DemoSerialized", std::ios::binary | std::ios::out);
			registry.Serialize(stream);
			std::cout << "Serialized to \"DemoSerialized\"";
		});

#ifdef REGISTRY_DESERIALIZE

	std::ifstream stream("DemoSerialized", std::ios::binary | std::ios::in);
	stream.exceptions(std::ifstream::failbit);
	stream.exceptions(std::ifstream::end);
	try
	{
		registry.Deserialize(stream);
	}
	catch (const std::exception& fail)
	{
		std::cerr << "ERROR:" << fail.what() << '\n';
		return 1;
	}
#else

	registry.AddSystem("RenderingSystem");
	registry.AddSystem("RenderTransformUpdate");
	registry.AddSystem("RenderModifier");
	registry.AddSystem("MoveScaleRotate");
	registry.AddSystem("PositionModulo");
	registry.AddSystem("BaseClassNamePrinter");

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

	objects[0].AddComponent<BaseClass>();
	objects[1].AddComponent<DerivedClass>();
	objects[2].AddComponent<UpdateAbleClass>();

#endif

	GUI::Registry::SetEntityRegistry(&registry);

	SDL::SetUpdateCallback([&registry](float)
		{
			//registry.Update(deltaTime);
			GUI::UpdateGUI();
		});

	SDL::StartLoop();

	GUI::Destroy();

	return 0;
}