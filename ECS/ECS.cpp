
#include <iostream>

#include "Registry/EntityRegistry.h"
#include "Registry/TypeView.h"
#include "Entity/Entity.h"
#include "TestClasses/Transform.h"
#include "TestClasses/Render.h"
#include "TestClasses/gameComponentTest.h"
#include "Sorting/SmoothSort.h"
#include "Sorting/SorterThreadPool.h"
#include "TypeInformation/TypeInformation.h"
#include "Entity/GameObject.h"

#include <memory>
#include <tuple>

int testFunc(float, int, double, GameCompTest2*)
{
    return 5;
}

void test1();
void test2();
void test3();

int main()
{
    //test1();
    test2();
    //test3();
}

void test1()
{
    EntityRegistry registry;

    for (int i{}; i < 24; ++i)
        registry.CreateEntity();

    auto& transforms = registry.AddView<Transform>();
    auto& renders = registry.AddView<Render>();
    auto& gameComps = registry.AddView<GameCompTest>();
    auto& gameComps2 = registry.AddView<GameCompTest2>();

    for (auto entity : registry.GetEntities())
        transforms.Add(entity);

    for (size_t i{}; i < 8; ++i)
        renders.Add(i);

    auto& binding = registry.AddBinding<Transform, Render>();

    for (size_t i{ 8 }; i < 16; ++i)
        renders.Add(i);

    for (size_t i{ }; i < 12; ++i)
        gameComps.Add(i);

    for (size_t i{ }; i < 4; ++i)
        gameComps2.Add(i);

    for (size_t i{}; i < transforms.GetSize(); ++i)
    {
        transforms.Get(i)->position[0] = float(i);
    }
    
    for (size_t i{}; i < renders.GetSize(); ++i)
    {
        renders.Get(i)->TextureId = (i);
    }

    for (auto& transform : transforms)
    {
        std::cout << transform.position[0] << '\n';
    }

    for (auto& render : renders)
    {
        std::cout << render.TextureId << '\n';
    }

    for (size_t i{ }; i < 16; ++i)
    {
        auto refRender = renders.Get(0);
        std::cout << refRender->TextureId << '\n';
    }

    std::cout << "Inactive\n";

    for (size_t i{}; i < transforms.GetSize(); i += 3)
    {
        transforms.SetInactive(i);
    }

    for (auto& transform : transforms)
    {
        std::cout << transform.position[0] << '\n';
    }

    std::cout << "Inactive\n";

    for (size_t i{}; i < transforms.GetSize(); i += 3)
    {
        transforms.SetActive(i);
    }

    for (auto& transform : transforms)
    {
        std::cout << transform.position[0] << '\n';
    }

    std::cout << "binding\n";

    for (auto& transformsRender : binding)
    {
        auto render = transformsRender.Get<Render>();
        auto transform = transformsRender.Get<Transform>();
        std::cout << render->TextureId << '\n';
        std::cout << transform->position[0] << '\n';
    }

    auto& SameBinding = registry.GetBinding<Transform, Render>();

    for (auto& transformsRender : SameBinding)
    {
        auto render = transformsRender.Get<Render>();
        auto transform = transformsRender.Get<Transform>();
        std::cout << render->TextureId << '\n';
        std::cout << transform->position[0] << '\n';
    }

    std::cout << "GameComps\n";

    renders.SetSortingPredicate([](const Render& rhs, const Render& lhs) {return rhs.TextureId > lhs.TextureId; });

    renders.Add(20);

    registry.Update();

    //while (true)
    //{
    //    registry.Update();
    //    std::cout << "Updated\n";
    //    for (auto& render : renders)
    //    {
    //        std::cout << render.TextureId << '\n';
    //    }
    //}

    std::ofstream stream("test.txt", std::ios::binary);
    registry.Serialize(stream);
}

void test2()
{
    EntityRegistry registry;

    registry.AddView<Render>();
    registry.AddView<Transform>();
    registry.AddView<GameCompTest>();
    registry.AddView<GameCompTest2>();

    registry.AddBinding<Render, Transform>();

    registry.AddSystem<Render>("RenderPrinter", [](Render& render) {std::cout << render.TextureId; });
    registry.AddSystem<Render, Transform>("RenderPosUpdate", [](Render& render, Transform& transform)
    {
	    render.Transformation[0][0] = transform.position[0];
    }, 1000);
    registry.AddSystem<Transform>("MovingTransform", [](Transform& transform)
    {
	    transform.position[0] += 1.f;
    });
    registry.AddSystem<Render>("PrintRenderPos", [](Render& render) {std::cout << render.Transformation[0][0]; }, 2000);
    registry.AddSystem<GameCompTest>("GameCompInitializer", [](GameCompTest& gameComp)
    {
            gameComp.Initialize();
    });

    std::vector<GameObject> GameObjects;
    for (size_t i{}; i < 16; ++i)
    {
        auto& obj = GameObjects.emplace_back(registry);
        auto render = obj.AddComponent<Render>();
        obj.AddComponent<Transform>();
        obj.AddComponent<GameCompTest>();
        obj.AddComponent<GameCompTest2>();
        render->TextureId = i;
    }

    registry.Update();
    std::cout << '\n';
    for (size_t i{}; i < 4; ++i) GameObjects.pop_back();
    registry.Update();
    std::cout << '\n';
	for (size_t i{}; i < 4; ++i) GameObjects.pop_back();
    registry.Update();
    std::cout << '\n';
    for (size_t i{}; i < 4; ++i) GameObjects.pop_back();
    registry.Update();
    std::cout << '\n';



}

void test3()
{
    uint32_t types[] {1,2,3,4,5};
    auto typesCombinations = TypeInformation::GetTypeCombinations(types, 5, 3);
	for (size_t i{}; i < typesCombinations.size(); ++i)
	{
        if (i % 3 == 0)
            std::cout << '\n';
        std::cout << typesCombinations[i];
	}
}

