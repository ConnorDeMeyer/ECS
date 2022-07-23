
#include <iostream>

#include <vld.h>

#include "Registry/EntityRegistry.h"
#include "Registry/TypeView.h"
#include "Entity/Entity.h"
#include "TestClasses/Transform.h"
#include "TestClasses/Render.h"
#include "TestClasses/gameComponentTest.h"
#include "Sorting/SmoothSort.h"
#include "Sorting/SorterThreadPool.h"

int main()
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
        renders.Add(registry.GetEntities()[i]);

    auto& binding = registry.AddBinding<Transform, Render>();

    for (size_t i{8}; i < 16; ++i)
        renders.Add(registry.GetEntities()[i]);

    for (size_t i{ }; i < 12; ++i)
        gameComps.Add(registry.GetEntities()[i]);

    for (size_t i{ }; i < 4; ++i)
        gameComps2.Add(registry.GetEntities()[i]);

    for (size_t i{}; i < transforms.GetSize(); ++i)
    {
        transforms.Get(i)->position[0] = float(i);
    }

    for (size_t i{}; i < renders.GetSize(); ++i)
    {
        renders.Get(i)->location[0] = float(i);
    }

    for (auto& transform : transforms)
    {
        std::cout << transform.position[0] << '\n';
    }

    for (auto& render : renders)
    {
        std::cout << render.location[0] << '\n';
    }

    for (size_t i{ }; i < 16; ++i)
    {
        auto refRender = renders.Get(0);
        std::cout << refRender->location[0] << '\n';
    }

    std::cout << "binding\n";

    for (auto& transformsRender : binding)
    {
        auto render = transformsRender.Get<Render>();
        auto transform = transformsRender.Get<Transform>();
        std::cout << render->location[0] << '\n';
        std::cout << transform->position[0] << '\n';
    }

    auto& SameBinding = registry.GetBinding<Render, Transform>();

    for (auto& transformsRender : SameBinding)
    {
        auto render = transformsRender.Get<Render>();
        auto transform = transformsRender.Get<Transform>();
        std::cout << render->location[0] << '\n';
        std::cout << transform->position[0] << '\n';
    }

    std::cout << "GameComps\n";

    
    registry.ForEachGameComponent([](GameComponentClass* comp)
        {
            comp->Initialize();
        });

    renders.SetSortingPredicate([](const Render& rhs, const Render& lhs) {return rhs.location[0] > lhs.location[0]; });

    renders.Add(registry.GetEntities()[20]);

    registry.Update();

    while (true)
    {
        registry.Update();
        std::cout << "Updated\n";
        for (auto& render : renders)
        {
            std::cout << render.location[0] << '\n';
        }
    }

}

